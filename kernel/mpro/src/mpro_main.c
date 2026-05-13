// SPDX-License-Identifier: GPL-2.0
/*
 * mpro_main.c — MPro USB display driver, MFD parent.
 *
 * Top-level USB driver (probe, disconnect, suspend, resume) that
 * registers the MFD framework and three child platform devices:
 * mpro_drm, mpro_touchscreen and mpro_backlight.
 *
 * All USB I/O goes through this module. Children use the public API
 * defined in mpro.h (mpro_send_full_frame, mpro_send_command,
 * mpro_make_request, etc.) and never touch USB directly.
 */

#include <linux/module.h>
#include <linux/lz4.h>

#include "mpro_internal.h"
#include "drm/mpro_drm.h"
#include "mpro.h"

/* ------------------------------------------------------------------ */
/* Module parameters                                                  */
/* ------------------------------------------------------------------ */

static bool mpro_fbdev_enabled;
module_param_named(fbdev, mpro_fbdev_enabled, bool, 0444);
MODULE_PARM_DESC(fbdev,
	"Enable fbdev console emulation on attached displays (default: 0). "
	"When enabled, USB autosuspend is disabled because fbdev keeps "
	"the display pipe permanently active. Enable this only if you "
	"want the MPro to act as a kernel console.");

static int mpro_lz4_level;
module_param_named(lz4_level, mpro_lz4_level, int, 0644);
MODULE_PARM_DESC(lz4_level,
	"LZ4 compression: 0=off (default), 1=fast, 2-12=HC levels");

static int mpro_lz4_threshold = 1024;
module_param_named(lz4_threshold, mpro_lz4_threshold, int, 0644);
MODULE_PARM_DESC(lz4_threshold,
	"Minimum frame size in bytes to apply LZ4 compression "
	"(default: 1024). Frames smaller than this are sent uncompressed "
	"because the compression overhead outweighs the bandwidth saved.");

static unsigned int mpro_idle_delay_ms = 30000;
module_param_named(idle_delay_ms, mpro_idle_delay_ms, uint, 0644);
MODULE_PARM_DESC(idle_delay_ms,
	"Time in milliseconds of inactivity before the display goes "
	"idle (backlight off, touch URB stopped). 0 = disabled. "
	"Default: 30000.");

static bool mpro_touch_wake = true;
module_param_named(touch_wake, mpro_touch_wake, bool, 0644);
MODULE_PARM_DESC(touch_wake,
	"Wake the display from virtual idle when the touchscreen is "
	"touched. Keeps the touchscreen URB pipeline running even "
	"while the display is idle. Default: 1.");

/* ------------------------------------------------------------------ */
/* MFD cells                                                          */
/* ------------------------------------------------------------------ */

static struct mfd_cell mpro_cells[] = {
	{ .name = "mpro_drm" },
	{ .name = "mpro_touchscreen" },
	{ .name = "mpro_backlight" },
};

/* ------------------------------------------------------------------ */
/* devm release callbacks                                             */
/* ------------------------------------------------------------------ */

static void mpro_dma_dev_release(void *data)
{
	struct mpro_device *mpro = data;

	if (mpro->dma_dev)
		put_device(mpro->dma_dev);
}

static void mpro_lz4_workmem_release(void *data)
{
	struct mpro_device *mpro = data;

	if (mpro->lz4_workmem) {
		kvfree(mpro->lz4_workmem);
		mpro->lz4_workmem = NULL;
	}
}

static void mpro_wq_release(void *data)
{
	struct mpro_device *mpro = data;

	mpro_io_shutdown(mpro);
	if (mpro->wq) {
		destroy_workqueue(mpro->wq);
		mpro->wq = NULL;
	}
}

static void mpro_touch_release(void *data)
{
	mpro_touch_shutdown(data);
}

/* ------------------------------------------------------------------ */
/* LZ4 helpers                                                        */
/* ------------------------------------------------------------------ */

int mpro_lz4_workmem_alloc(struct mpro_device *mpro)
{
	int ret;

	if (mpro->lz4_workmem)
		return 0;

	mpro->lz4_workmem_size = max((size_t)LZ4_MEM_COMPRESS,
				     (size_t)LZ4HC_MEM_COMPRESS);
	mpro->lz4_workmem = kvmalloc(mpro->lz4_workmem_size, GFP_KERNEL);
	if (!mpro->lz4_workmem)
		return -ENOMEM;

	ret = devm_add_action_or_reset(&mpro->intf->dev,
				       mpro_lz4_workmem_release, mpro);
	if (ret)
		return ret;
	return 0;
}
EXPORT_SYMBOL_GPL(mpro_lz4_workmem_alloc);

static int mpro_setup_lz4(struct mpro_device *mpro, struct usb_interface *intf)
{
	int requested;
	int ret;

	if (mpro_lz4_level <= 0)
		return 0;

	requested = clamp(mpro_lz4_level, 0, 12);

	if (mpro->model->margin > 0) {
		dev_info(&intf->dev,
			 "LZ4 requested but disabled: margin device "
			 "does not support compression\n");
		return 0;
	}

	if (!mpro_firmware_supports_lz4(mpro)) {
		dev_warn(&intf->dev,
			 "LZ4 requested but disabled: firmware %d.%d "
			 "does not support compression (need >= %d.%d)\n",
			 mpro->fw_major, mpro->fw_minor,
			 MPRO_LZ4_MIN_MAJOR, MPRO_LZ4_MIN_MINOR);
		return 0;
	}

	ret = mpro_lz4_workmem_alloc(mpro);
	if (ret)
		return ret;

	mpro->lz4_level = requested;
	dev_info(&intf->dev, "LZ4 compression enabled: level %d\n",
		 mpro->lz4_level);
	return 0;
}

/* ------------------------------------------------------------------ */
/* Probe / disconnect                                                 */
/* ------------------------------------------------------------------ */

static int mpro_usb_probe(struct usb_interface *intf,
			  const struct usb_device_id *id)
{
	struct device *dev = &intf->dev;
	struct mpro_device *mpro;
	int ret;

	if (usb_get_intfdata(intf))
		return -EBUSY;

	mpro = devm_kzalloc(dev, sizeof(*mpro), GFP_KERNEL);
	if (!mpro)
		return -ENOMEM;

	mpro->intf		= intf;
	mpro->udev		= interface_to_usbdev(intf);
	mpro->request_delay	= 5;
	mpro->running		= false;
	mpro->fbdev_enabled	= mpro_fbdev_enabled;

	mutex_init(&mpro->lock);
	mutex_init(&mpro->lz4_lock);
	mutex_init(&mpro->listeners_lock);
	INIT_LIST_HEAD(&mpro->screen_listeners);

	/* DMA device — used for buffer sharing by child drivers */
	mpro->dma_dev = usb_intf_get_dma_device(intf);
	if (mpro->dma_dev) {
		ret = devm_add_action_or_reset(dev, mpro_dma_dev_release, mpro);
		if (ret)
			return ret;
	} else {
		dev_warn(dev, "buffer sharing not supported\n");
	}

	/* --- Synchronous probe queries --- */

	ret = mpro_request_u32(mpro, cmd_get_screen, sizeof(cmd_get_screen),
			       &mpro->screen);
	if (ret)
		return dev_err_probe(dev, ret, "get_screen failed\n");

	ret = mpro_request_u32(mpro, cmd_get_version, sizeof(cmd_get_version),
			       &mpro->version);
	if (ret)
		return dev_err_probe(dev, ret, "get_version failed\n");

	ret = mpro_make_request(mpro, cmd_get_id, sizeof(cmd_get_id),
				mpro->id, sizeof(mpro->id));
	if (ret)
		return dev_err_probe(dev, ret, "get_id failed\n");

	ret = mpro_make_request(mpro, cmd_quit_sleepmode,
				sizeof(cmd_quit_sleepmode), NULL, 0);
	if (ret)
		dev_warn(dev, "quit_sleepmode failed: %d\n", ret);

	ret = mpro_get_model(mpro);
	if (ret)
		return dev_err_probe(dev, ret,
				     "unsupported model 0x%x\n", mpro->screen);

	dev_info(dev, "Detected %s\n", mpro->model->description);

	ret = mpro_get_firmware(mpro);
	if (ret)
		dev_warn(dev, "firmware probe failed: %d\n", ret);

	if (mpro->fw_major >= 0)
		dev_info(dev, "Firmware: %s\n", mpro->fw_string);
	else
		dev_warn(dev,
			 "Firmware: %s (version parsing failed)\n",
			 mpro->fw_string);

	/* --- LZ4 (depends on the firmware version, so set up after probe) --- */

	mpro->lz4_threshold = clamp(mpro_lz4_threshold, 0, INT_MAX);
	ret = mpro_setup_lz4(mpro, intf);
	if (ret)
		return ret;

	/* --- Pipeline --- */

	mpro->wq = alloc_ordered_workqueue("mpro_wq", WQ_MEM_RECLAIM);
	if (!mpro->wq)
		return -ENOMEM;

	mpro_io_init(mpro);

	/*
	 * mpro_io_shutdown() and destroy_workqueue() run automatically on
	 * probe failure or disconnect.
	 */
	ret = devm_add_action_or_reset(dev, mpro_wq_release, mpro);
	if (ret)
		return ret;

	/* Touch URB pipeline — must come after io_init, before MFD children */
	ret = mpro_touch_init(mpro);
	if (ret)
		return dev_err_probe(dev, ret, "touch init failed\n");

	ret = devm_add_action_or_reset(dev, mpro_touch_release, mpro);
	if (ret)
		return ret;

	/* sysfs attributes (devm-managed) */
	ret = mpro_sysfs_add(mpro);
	if (ret)
		return dev_err_probe(dev, ret, "sysfs add failed\n");

	usb_set_intfdata(intf, mpro);
	mpro->running = true;

	/* MFD children */
	ret = devm_mfd_add_devices(dev, -1, mpro_cells,
				   ARRAY_SIZE(mpro_cells), NULL, 0, NULL);
	if (ret) {
		mpro->running = false;
		usb_set_intfdata(intf, NULL);
		return dev_err_probe(dev, ret, "MFD register failed\n");
	}

	/*
	 * Initialise virtual idle state. The device firmware does not
	 * support a working USB-bus resume, so real autosuspend cannot be
	 * used. Instead we track child activity and turn the backlight off
	 * after a configurable idle period — see mpro_pm.c for details.
	 */
	mpro->idle_delay_ms = mpro_idle_delay_ms;
	mpro->touch_wake_enabled = mpro_touch_wake;
	mpro_pm_init(mpro);

	if (mpro->idle_delay_ms > 0)
		dev_info(dev, "Virtual idle enabled (delay %u ms)\n",
			 mpro->idle_delay_ms);
	else
		dev_info(dev, "Virtual idle disabled\n");

	dev_info(dev, "MPRO MFD driver registered\n");
	return 0;
}

static void mpro_usb_disconnect(struct usb_interface *intf)
{
	struct mpro_device *mpro = usb_get_intfdata(intf);
	struct mpro_drm *mdrm;
	unsigned long flags;

	if (!mpro)
		return;

	/*
	 * Stop accepting new submissions to the pipeline. Children may
	 * still call into us during their own teardown — those calls will
	 * get -ESHUTDOWN.
	 */
	spin_lock_irqsave(&mpro->state_lock, flags);
	mpro->running = false;
	spin_unlock_irqrestore(&mpro->state_lock, flags);

	/* Cancel any pending idle work before children start tearing down */
	mpro_pm_shutdown(mpro);

	/*
	 * Unplug the DRM device before the MFD children are torn down.
	 * This blocks any new DRM operations but leaves mdrm->mpro intact
	 * — the DRM remove callback will clear it once all in-flight
	 * callbacks have drained.
	 */
	mdrm = READ_ONCE(mpro->drm);
	if (mdrm) {
		drm_dev_unplug(&mdrm->drm);
		WRITE_ONCE(mpro->drm, NULL);
	}

	usb_set_intfdata(intf, NULL);

	dev_info(&intf->dev, "MPRO disconnected\n");

	/*
	 * Remaining resources (touch URB pipeline, workqueue + frame pipeline,
	 * lz4_workmem, dma_dev, MFD children, sysfs) are released
	 * automatically through devres in reverse registration order.
	 * MFD children are torn down first, so mpro_ts__remove() stops the
	 * touch URB before mpro_touch_release() frees the hardware resources.
	 */
}

/* ------------------------------------------------------------------ */
/* USB driver                                                         */
/* ------------------------------------------------------------------ */

static const struct usb_device_id id_table[] = {
	{ USB_DEVICE(0xc872, 0x1004) },
	{ }
};

MODULE_DEVICE_TABLE(usb, id_table);

static struct usb_driver mpro_usb_driver = {
	.name			= "mpro",
	.probe			= mpro_usb_probe,
	.disconnect		= mpro_usb_disconnect,
	.supports_autosuspend	= 0,
	.id_table		= id_table,
};

module_usb_driver(mpro_usb_driver);

MODULE_DESCRIPTION("MPRO USB driver (MFD infrastructure)");
MODULE_AUTHOR("Oskari Rauta <oskari.rauta@gmail.com>");
MODULE_LICENSE("GPL");
