// SPDX-License-Identifier: GPL-2.0-only
/*
 * beada-main.c – BeadaPanel MFD parent: USB driver entry point
 *
 * USB probe/disconnect, MFD child registration, and module boilerplate.
 *
 * Probe sequence:
 *   1. Validate USB interface (must be interface 0, vendor-class)
 *   2. Allocate beada_mfd_dev and initialise its locks and lists
 *   3. Discover USB bulk endpoints (beada-usb.c)
 *   4. Allocate DMA-safe Status-Link and Panel-Link buffers
 *   5. Reset the device's Panel-Link receiver (PL_RESET)
 *   6. Query the device for model/resolution via Status-Link GET_INFO
 *   7. Allocate the async transfer pipeline (workqueue)
 *   8. Register sysfs attribute group (beada-sysfs.c)
 *   9. Publish parent handle via usb_set_intfdata()
 *  10. Synchronise device clock to kernel UTC time
 *  11. Register MFD child devices (DRM + backlight) via devm
 */

#include <linux/delay.h>
#include <linux/mfd/core.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/usb.h>

#include "beada-priv.h"

/* ------------------------------------------------------------------ */
/* USB device table                                                     */
/* ------------------------------------------------------------------ */

#define BEADA_VENDOR_ID		0x4e58		/* NXElec */

static const struct usb_device_id beada_usb_ids[] = {
	{ USB_DEVICE(BEADA_VENDOR_ID, 0x1001) },
	{ USB_DEVICE(BEADA_VENDOR_ID, 0x1002) },
	{ }
};
MODULE_DEVICE_TABLE(usb, beada_usb_ids);

/* ------------------------------------------------------------------ */
/* MFD child cells                                                      */
/* ------------------------------------------------------------------ */

/*
 * Children retrieve the parent handle with:
 *
 *   struct beada_mfd_dev *beada = dev_get_drvdata(pdev->dev.parent);
 *
 * No platform_data is passed; beada.h is the sole coupling surface.
 */
static const struct mfd_cell beada_cells[] = {
	{
		.name          = BEADA_CELL_DRM,
		.of_compatible = "nxelec,beada-drm",
	},
	{
		.name          = BEADA_CELL_BACKLIGHT,
		.of_compatible = "nxelec,beada-backlight",
	},
};

/* ------------------------------------------------------------------ */
/* Coherent DMA buffer cleanup (devm action)                            */
/* ------------------------------------------------------------------ */

static void beada_free_frame_buf(void *data)
{
	struct beada_mfd_dev *beada = data;

	usb_free_coherent(beada->udev, PANELLINK_MAX_FRAME_BYTES,
			  beada->pl_frame_buf, beada->pl_frame_buf_dma);
}

/* ------------------------------------------------------------------ */
/* USB probe                                                            */
/* ------------------------------------------------------------------ */

static int beada_probe(struct usb_interface *intf,
		       const struct usb_device_id *id)
{
	struct beada_mfd_dev *beada;
	int ret;

	/*
	 * Interface 0: Panel-Link + Status-Link (bInterfaceClass = 0xFF).
	 * Interface 1, when present: USB mass storage for BeadaTools.
	 */
	if (intf->cur_altsetting->desc.bInterfaceNumber != 0)
		return -ENODEV;
	if (intf->cur_altsetting->desc.bInterfaceClass != USB_CLASS_VENDOR_SPEC)
		return -ENODEV;

	beada = devm_kzalloc(&intf->dev, sizeof(*beada), GFP_KERNEL);
	if (!beada)
		return -ENOMEM;

	beada->udev = interface_to_usbdev(intf);
	beada->intf = intf;
	beada->dev  = &intf->dev;

	mutex_init(&beada->xfer_lock);
	mutex_init(&beada->resume_handlers_lock);
	INIT_LIST_HEAD(&beada->resume_handlers);

	/* ── Endpoint discovery ─────────────────────────────── */
	ret = beada_find_endpoints(beada);
	if (ret)
		return ret;

	/* ── DMA-safe I/O buffers ───────────────────────────── */
	beada->sl_buf     = devm_kmalloc(&intf->dev, 256, GFP_KERNEL);
	beada->pl_tag_buf = devm_kmalloc(&intf->dev,
					 sizeof(struct panellink_tag),
					 GFP_KERNEL);
	if (!beada->sl_buf || !beada->pl_tag_buf)
		return -ENOMEM;

	/*
	 * Frame buffer: must be DMA-coherent and below the 4 GB boundary so
	 * 32-bit USB controllers (EHCI) can DMA directly to it without going
	 * through swiotlb bouncing.  Without this, on hosts with > 4 GB RAM
	 * the kernel falls back to swiotlb on every frame transfer; under a
	 * high-frame-rate client (e.g. kmscube hitting hundreds of fps) the
	 * 64 MB swiotlb pool fragments and 300 KB allocations start failing
	 * with EAGAIN.  JT365 uses usb_alloc_coherent for the same reason.
	 */
	beada->pl_frame_buf = usb_alloc_coherent(beada->udev,
						 PANELLINK_MAX_FRAME_BYTES,
						 GFP_KERNEL,
						 &beada->pl_frame_buf_dma);
	if (!beada->pl_frame_buf)
		return -ENOMEM;
	ret = devm_add_action_or_reset(&intf->dev,
				       beada_free_frame_buf, beada);
	if (ret)
		return ret;

	/* ── Panel-Link reset ───────────────────────────────── */
	/*
	 * Reset the device's Panel-Link receiver before anything else.
	 * Required to recover from a "mid-frame" state the firmware can
	 * be left in by a previous driver session; without this the first
	 * frame's START tag gets consumed as stale pixel data and the
	 * whole display is shifted by 270 bytes.
	 *
	 * Cosmetic side effect: the device's standby clock resets to its
	 * power-on default.  The beada_sync_time() call further down
	 * restores it.
	 */
	ret = beada_panellink_reset(beada);
	if (ret)
		return ret;

	/*
	 * Settling delay.  The reset triggers a firmware-internal re-init;
	 * if GET_INFO is sent too soon the device does not yet ACK and the
	 * transfer times out.  50 ms is empirically the smallest reliable
	 * value on tested firmware (7.19, T113 platform).
	 */
	msleep(50);

	/* ── GET_INFO ───────────────────────────────────────── */
	ret = beada_query_device_info(beada);
	if (ret)
		return ret;

	/* ── Async transfer pipeline ────────────────────────── */
	ret = beada_pipeline_init(beada);
	if (ret)
		return ret;

	/* ── sysfs ──────────────────────────────────────────── */
	ret = beada_sysfs_init(beada);
	if (ret) {
		dev_warn(beada->dev,
			 "sysfs init failed (%d); continuing\n", ret);
		/* Non-fatal */
	}

	/* ── Publish parent handle ──────────────────────────── */
	/*
	 * Must be set before mfd_add_devices() because children's probe()
	 * runs synchronously and calls dev_get_drvdata(pdev->dev.parent).
	 */
	usb_set_intfdata(intf, beada);

	/* ── Clock sync ─────────────────────────────────────── */
	if (beada->has_statuslink) {
		ret = beada_sync_time(beada);
		if (ret && ret != -EOPNOTSUPP)
			dev_warn(beada->dev,
				 "initial clock sync failed (%d)\n", ret);
	}

	/* ── MFD children (DRM + backlight) ─────────────────── */
	/*
	 * devm_mfd_add_devices registers a devm cleanup action that
	 * removes the children automatically when the parent device
	 * is torn down — no explicit mfd_remove_devices() needed in
	 * disconnect().
	 */
	ret = devm_mfd_add_devices(&intf->dev, -1,
				   beada_cells, ARRAY_SIZE(beada_cells),
				   NULL, 0, NULL);
	if (ret) {
		dev_err(beada->dev, "mfd_add_devices failed: %d\n", ret);
		return ret;
	}

	dev_info(beada->dev, "BeadaPanel %s ready\n", beada->panel.model);
	return 0;
}

/* ------------------------------------------------------------------ */
/* USB disconnect                                                       */
/* ------------------------------------------------------------------ */

static void beada_disconnect(struct usb_interface *intf)
{
	struct beada_mfd_dev *beada = usb_get_intfdata(intf);

	if (!beada)
		return;

	/*
	 * Mark the device gone before doing anything else.  Any concurrent
	 * send_frame / set_brightness / sync_time will see disconnected=true
	 * via beada_check_alive() and return -ENODEV without touching USB.
	 *
	 * Children's remove() functions run after this returns (the devm
	 * action registered by devm_mfd_add_devices() takes care of it).
	 * By that point @disconnected is already true, so any USB calls
	 * they might make short-circuit cleanly.
	 */
	mutex_lock(&beada->xfer_lock);
	beada->disconnected = true;
	mutex_unlock(&beada->xfer_lock);

	cancel_delayed_work_sync(&beada->resume_work);
	flush_work(&beada->xfer_work);

	dev_info(&intf->dev, "BeadaPanel %s disconnected\n",
		 beada->panel.model);
}

/* ------------------------------------------------------------------ */
/* Module registration                                                  */
/* ------------------------------------------------------------------ */

static struct usb_driver beada_usb_driver = {
	.name			= "beadapanel",
	.probe			= beada_probe,
	.disconnect		= beada_disconnect,
	.suspend		= beada_usb_suspend,
	.resume			= beada_usb_resume,
	.reset_resume		= beada_usb_resume,	/* same handling */
	.id_table		= beada_usb_ids,
	.supports_autosuspend	= 1,
};

module_usb_driver(beada_usb_driver);

MODULE_DESCRIPTION("BeadaPanel USB display – MFD parent (Panel-Link + Status-Link v1.3)");
MODULE_AUTHOR("Oskari Rauta <oskari.rauta@gmail.com>");
MODULE_LICENSE("GPL");
