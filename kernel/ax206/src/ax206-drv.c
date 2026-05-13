// SPDX-License-Identifier: GPL-2.0-only
/*
 * ax206-drv.c - AX206 (DPF) digital photo frame MFD parent driver
 *
 * Handles USB probe/disconnect, workqueue lifetime and MFD child
 * registration.  All USB I/O after probe runs through the shared
 * workqueue so that children (DRM, backlight) never block each other
 * and never call USB from atomic context.
 *
 * Children are:
 *   ax206-drm   - DRM KMS display driver
 *   ax206-bl    - backlight driver
 *
 * Copyright (C) 2025 Oskari Rauta <oskari.rauta@gmail.com>
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/usb.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/mfd/core.h>
#include <linux/platform_device.h>

#include "ax206.h"
#include "ax206-priv.h"

#define AX206_DRIVER_NAME		"ax206"
#define AX206_DRM_CELL_NAME		"ax206-drm"
#define AX206_BL_CELL_NAME		"ax206-bl"

/* MFD cell definitions - one DRM child, one backlight child */
static const struct mfd_cell ax206_cells[] = {
	{
		.name		= AX206_DRM_CELL_NAME,
	},
	{
		.name		= AX206_BL_CELL_NAME,
	},
};

/* ----- USB disconnect (software version) ----- */

/*
 * ax206_sw_disconnect() - perform a software disconnect
 *
 * Blanks the display and kills the backlight before the USB
 * interface is released.  Called from ax206_disconnect() and
 * also from the DRM driver's lastclose path.
 *
 * May sleep; must NOT be called from atomic context.
 */
static void ax206_sw_disconnect(struct ax206_dev *dpf)
{
	if (!dpf->connected)
		return;

	/*
	 * When rmmod is called, the USB core sets intf->condition to
	 * USB_INTERFACE_UNBINDING before invoking disconnect().  Any
	 * usb_submit_urb() call in that window returns -ENOENT.
	 * Skip the blank/backlight sequence in that case; it's a clean
	 * module removal, not a physical unplug.
	 */
	if (dpf->intf->condition != USB_INTERFACE_BOUND)
		return;

	/* Turn backlight off first so the black fill is not seen lit */
	ax206_usb_set_backlight(dpf, 0);
	ax206_usb_fill_black(dpf);
}

/* ----- USB probe ----- */

static int ax206_probe(struct usb_interface *intf,
		     const struct usb_device_id *id)
{
	struct ax206_dev *dpf;
	int ret;

	dpf = devm_kzalloc(&intf->dev, sizeof(*dpf), GFP_KERNEL);
	if (!dpf)
		return -ENOMEM;

	dpf->udev      = interface_to_usbdev(intf);
	dpf->intf      = intf;
	dpf->dev       = &intf->dev;
	dpf->connected = true;
	dpf->blanked   = false;
	dpf->backlight = AX206_BL_DEFAULT;

	mutex_init(&dpf->usb_mutex);

	usb_set_intfdata(intf, dpf);

	/*
	 * Discover bulk endpoints from the interface descriptor.
	 * This must happen before any USB transfer attempt.
	 */
	ret = ax206_usb_init(dpf);
	if (ret)
		return ret;

	/*
	 * Query physical display dimensions synchronously.
	 * The workqueue does not exist yet so there is no concurrency.
	 * The USB core guarantees the device is active during probe when
	 * supports_autosuspend = 0 is set in the usb_driver struct.
	 */
	mutex_lock(&dpf->usb_mutex);
	ret = ax206_usb_query_dims(dpf);
	mutex_unlock(&dpf->usb_mutex);
	if (ret) {
		dev_err(dpf->dev, "ax206: cannot determine display size\n");
		return ret;
	}

	/* Create single-threaded workqueue - all post-probe USB I/O goes here */
	dpf->wq = alloc_ordered_workqueue("ax206_usb", WQ_MEM_RECLAIM);
	if (!dpf->wq) {
		dev_err(dpf->dev, "ax206: failed to create workqueue\n");
		return -ENOMEM;
	}

	/*
	 * Set initial backlight.  ax206_usb_set_backlight() acquires
	 * usb_mutex internally, so we must NOT hold it here.
	 */
	ax206_usb_set_backlight(dpf, dpf->backlight);

	/* Register MFD children */
	ret = mfd_add_devices(dpf->dev, -1,
			      ax206_cells, ARRAY_SIZE(ax206_cells),
			      NULL, 0, NULL);
	if (ret) {
		dev_err(dpf->dev, "ax206: failed to add MFD children: %d\n", ret);
		destroy_workqueue(dpf->wq);
		return ret;
	}

	dev_info(dpf->dev,
		 "ax206: AX206 frame at %s (%ux%u)\n",
		 dev_name(dpf->dev), dpf->width, dpf->height);

	return 0;
}

/* ----- USB disconnect ----- */

static void ax206_disconnect(struct usb_interface *intf)
{
	struct ax206_dev *dpf = usb_get_intfdata(intf);

	if (!dpf)
		return;

	/* Blank display before tearing down */
	ax206_sw_disconnect(dpf);

	dpf->connected = false;

	/* Remove children first so they stop queuing work */
	mfd_remove_devices(dpf->dev);

	/* Drain and destroy workqueue */
	if (dpf->wq) {
		drain_workqueue(dpf->wq);
		destroy_workqueue(dpf->wq);
		dpf->wq = NULL;
	}

	usb_set_intfdata(intf, NULL);

	dev_info(dpf->dev, "ax206: disconnected\n");
}

/* ----- USB device table ----- */

static const struct usb_device_id ax206_usb_ids[] = {
	{ USB_DEVICE(AX206_USB_VID, AX206_USB_PID) },
	{ }
};
MODULE_DEVICE_TABLE(usb, ax206_usb_ids);

static struct usb_driver ax206_usb_driver = {
	.name               = AX206_DRIVER_NAME,
	.probe              = ax206_probe,
	.disconnect         = ax206_disconnect,
	.id_table           = ax206_usb_ids,
	/*
	 * soft_unbind = 1: do not set intf->condition = UNBINDING before
	 * calling disconnect().  This allows USB transfers (backlight off,
	 * screen blank) to succeed during rmmod/module removal.
	 */
	.soft_unbind        = 1,
	/*
	 * supports_autosuspend = 0: USB core keeps the device active while
	 * the driver is bound, calling usb_autopm_get_interface() before
	 * probe and the matching put after disconnect.
	 */
	.supports_autosuspend = 0,
};

module_usb_driver(ax206_usb_driver);

MODULE_AUTHOR("Oskari Rauta <oskari.rauta@gmail.com>");
MODULE_DESCRIPTION("AX206 (DPF) digital photo frame MFD parent driver");
MODULE_LICENSE("GPL");
