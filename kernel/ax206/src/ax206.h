/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * ax206.h - AX206 digital photo frame MFD parent shared header
 *
 * Public API visible to MFD child drivers (DRM and backlight).
 *
 * Copyright (C) 2025 Oskari Rauta <oskari.rauta@gmail.com>
 */

#ifndef _AX206_H
#define _AX206_H

#include <linux/types.h>
#include <linux/usb.h>

/* USB IDs */
#define AX206_USB_VID		0x1908
#define AX206_USB_PID		0x0102

/* Pixel format: RGB565, 2 bytes per pixel */
#define AX206_BPP			2

/* Backlight range */
#define AX206_BL_MIN		0
#define AX206_BL_MAX		7
#define AX206_BL_DEFAULT		7

/* AX206 proprietary SCSI-over-USB command subcodes (byte [6] of CBWCB) */
#define AX206_CMD_SETPROPERTY	0x01
#define AX206_CMD_BLIT		0x12
#define AX206_CMD_GET_LCD_PARAMS	0x02

/* Property identifiers */
#define AX206_PROP_BRIGHTNESS	0x01

/**
 * struct ax206_dev - MFD parent device context
 *
 * All fields are managed by the parent (mfd/dpf-drv.c).
 * Children access this via platform_get_drvdata() on their MFD parent.
 *
 * USB operations must be submitted through the workqueue; the children
 * call dpf_submit_work() or the provided helper functions below.
 */
struct ax206_dev {
	struct usb_device	*udev;
	struct usb_interface	*intf;
	struct device		*dev;

	/* Physical display dimensions (read from device at probe) */
	u16			width;
	u16			height;

	/* Bulk endpoint addresses discovered from interface descriptor */
	u8			bulk_out;   /* e.g. 0x01 */
	u8			bulk_in;    /* e.g. 0x81 */

	/* Current backlight level (0-7); used by resume to restore */
	int			backlight;

	/* Workqueue for all USB I/O after probe */
	struct workqueue_struct	*wq;

	/* Protects USB access */
	struct mutex		usb_mutex;

	/* True while USB device is reachable */
	bool			connected;

	/* Power state - true when display is blanked (virtual idle) */
	bool			blanked;
};

/* ----- API exported by parent to children ----- */

/**
 * ax206_usb_blit() - blit an RGB565 region to the display
 * @dpf:   parent device
 * @buf:   RGB565 pixel data for the rectangle
 * @len:   byte length of @buf
 * @x0:    left edge (inclusive)
 * @y0:    top edge (inclusive)
 * @x1:    right edge (exclusive)
 * @y1:    bottom edge (exclusive)
 *
 * Must be called from workqueue context or probe/remove.
 * Returns 0 on success, negative on error.
 */
int ax206_usb_blit(struct ax206_dev *dpf, const u8 *buf, size_t len,
		 u16 x0, u16 y0, u16 x1, u16 y1);

/**
 * ax206_usb_set_backlight() - set display backlight (0-7)
 * @dpf:   parent device
 * @value: brightness level
 *
 * Must be called from workqueue context or probe/remove.
 * Returns 0 on success, negative on error.
 */
int ax206_usb_set_backlight(struct ax206_dev *dpf, int value);

/**
 * ax206_usb_fill_black() - fill entire display with black
 * @dpf:   parent device
 *
 * Convenience helper used during blank/shutdown.
 * Must be called from workqueue context or probe/remove.
 */
int ax206_usb_fill_black(struct ax206_dev *dpf);

#endif /* _AX206_H */
