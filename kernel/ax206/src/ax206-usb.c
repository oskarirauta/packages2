// SPDX-License-Identifier: GPL-2.0-only
/*
 * ax206-usb.c - AX206 MFD parent USB communication layer
 *
 * Implements the SCSI-over-USB (Bulk-Only Transport) protocol used by
 * AX206-based digital photo frames.  The endpoint addresses are
 * discovered from the interface descriptor at probe time rather than
 * hardcoded, which is necessary for reliable operation across different
 * host controllers.
 *
 * All functions here are called from probe/remove or from the parent
 * workqueue; none may be called from atomic context.
 *
 * Command codes confirmed against the original dpf-ax / lcd4linux
 * reference implementation:
 *   http://dpf-ax.sourceforge.net/
 *   https://github.com/dreamlayers/dpf-ax
 *
 * Copyright (C) 2025 Oskari Rauta <oskari.rauta@gmail.com>
 */

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/usb.h>

#include "ax206.h"
#include "ax206-priv.h"

/* USB Bulk-Only Transport: Command Block Wrapper */
struct ax206_cbw {
	__le32	dCBWSignature;          /* "USBC" = 0x43425355 */
	__le32	dCBWTag;
	__le32	dCBWDataTransferLength;
	__u8	bmCBWFlags;             /* 0x80 = IN, 0x00 = OUT */
	__u8	bCBWLUN;
	__u8	bCBWCBLength;           /* always 16 for AX206 */
	__u8	CBWCB[16];
} __packed;

/* USB Bulk-Only Transport: Command Status Wrapper */
struct ax206_csw {
	__le32	dCSWSignature;          /* "USBS" = 0x53425355 */
	__le32	dCSWTag;
	__le32	dCSWDataResidue;
	__u8	bCSWStatus;
} __packed;

#define AX206_CBW_SIG	0x43425355U
#define AX206_CSW_SIG	0x53425355U
#define AX206_CBW_TAG	0xdeadbeefU
#define AX206_USB_TIMEOUT	5000		/* ms */

/*
 * ax206_usb_cmd() - execute one SCSI-over-USB command
 *
 * @dpf:  parent device (bulk_in/bulk_out must be set)
 * @cmd:  16-byte SCSI command block
 * @dir:  USB_DIR_IN or USB_DIR_OUT
 * @data: payload buffer (may be NULL)
 * @len:  payload length in bytes
 *
 * Caller must hold dpf->usb_mutex.
 * Returns 0 on success, negative errno or positive CSW status on failure.
 */
static int ax206_usb_cmd(struct ax206_dev *dpf, const u8 *cmd, u8 dir,
		       u8 *data, size_t len)
{
	struct ax206_cbw *cbw;
	struct ax206_csw *csw;
	int actual, ret;

	cbw = kmalloc(sizeof(*cbw), GFP_KERNEL);
	csw = kmalloc(sizeof(*csw), GFP_KERNEL);
	if (!cbw || !csw) {
		ret = -ENOMEM;
		goto free;
	}

	/* Build CBW */
	memset(cbw, 0, sizeof(*cbw));
	cbw->dCBWSignature          = cpu_to_le32(AX206_CBW_SIG);
	cbw->dCBWTag                = cpu_to_le32(AX206_CBW_TAG);
	cbw->dCBWDataTransferLength = cpu_to_le32((u32)len);
	cbw->bmCBWFlags             = (dir == USB_DIR_IN) ? 0x80 : 0x00;
	cbw->bCBWLUN                = 0;
	cbw->bCBWCBLength           = 16;
	memcpy(cbw->CBWCB, cmd, 16);

	/* Send CBW */
	ret = usb_bulk_msg(dpf->udev,
			   usb_sndbulkpipe(dpf->udev, dpf->bulk_out),
			   cbw, sizeof(*cbw), &actual, AX206_USB_TIMEOUT);
	if (ret) {
		dev_err(dpf->dev, "ax206: CBW send failed: %d\n", ret);
		goto free;
	}

	/* Optional data phase */
	if (data && len) {
		if (dir == USB_DIR_OUT) {
			ret = usb_bulk_msg(dpf->udev,
					   usb_sndbulkpipe(dpf->udev,
							   dpf->bulk_out),
					   data, len, &actual, AX206_USB_TIMEOUT);
		} else {
			ret = usb_bulk_msg(dpf->udev,
					   usb_rcvbulkpipe(dpf->udev,
							   dpf->bulk_in &
							   USB_ENDPOINT_NUMBER_MASK),
					   data, len, &actual, AX206_USB_TIMEOUT);
		}
		if (ret) {
			dev_err(dpf->dev, "ax206: data phase failed: %d\n", ret);
			goto free;
		}
	}

	/* Receive CSW */
	ret = usb_bulk_msg(dpf->udev,
			   usb_rcvbulkpipe(dpf->udev,
					   dpf->bulk_in &
					   USB_ENDPOINT_NUMBER_MASK),
			   csw, sizeof(*csw), &actual, AX206_USB_TIMEOUT);
	if (ret) {
		dev_err(dpf->dev, "ax206: CSW receive failed: %d\n", ret);
		goto free;
	}

	if (le32_to_cpu(csw->dCSWSignature) != AX206_CSW_SIG) {
		dev_err(dpf->dev, "ax206: invalid CSW signature\n");
		ret = -EIO;
		goto free;
	}

	ret = csw->bCSWStatus;  /* 0 = success */

free:
	kfree(cbw);
	kfree(csw);
	return ret;
}

/*
 * ax206_usb_init() - discover bulk endpoints from the interface descriptor
 *
 * Must be called during probe before any USB transfer.
 * Returns 0 on success, -ENODEV if endpoints are missing.
 */
int ax206_usb_init(struct ax206_dev *dpf)
{
	struct usb_interface *intf = dpf->intf;
	struct usb_endpoint_descriptor *ep;
	int i;

	for (i = 0; i < intf->cur_altsetting->desc.bNumEndpoints; i++) {
		ep = &intf->cur_altsetting->endpoint[i].desc;
		if (!dpf->bulk_out && usb_endpoint_is_bulk_out(ep))
			dpf->bulk_out = ep->bEndpointAddress;
		else if (!dpf->bulk_in && usb_endpoint_is_bulk_in(ep))
			dpf->bulk_in = ep->bEndpointAddress;
	}

	if (!dpf->bulk_out || !dpf->bulk_in) {
		dev_err(dpf->dev, "ax206: missing bulk endpoints\n");
		return -ENODEV;
	}

	dev_dbg(dpf->dev, "ax206: bulk_out=0x%02x bulk_in=0x%02x\n",
		dpf->bulk_out, dpf->bulk_in);
	return 0;
}

/*
 * ax206_usb_query_dims() - read physical LCD dimensions
 *
 * Called only during probe (synchronously, no workqueue).
 * Caller must hold dpf->usb_mutex.
 */
int ax206_usb_query_dims(struct ax206_dev *dpf)
{
	u8 cmd[16] = {
		0xcd, 0x00, 0x00, 0x00,
		0x00, AX206_CMD_GET_LCD_PARAMS,
		0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00,
		0x00, 0x00
	};
	u8 *buf;
	int ret;

	buf = kmalloc(5, GFP_KERNEL);
	if (!buf)
		return -ENOMEM;

	ret = ax206_usb_cmd(dpf, cmd, USB_DIR_IN, buf, 5);
	if (ret)
		goto out;

	dpf->width  = buf[0] | ((u16)buf[1] << 8);
	dpf->height = buf[2] | ((u16)buf[3] << 8);

	if (!dpf->width || !dpf->height) {
		dev_err(dpf->dev, "ax206: device returned invalid dimensions %ux%u\n",
			dpf->width, dpf->height);
		ret = -ENODEV;
		goto out;
	}

	dev_info(dpf->dev, "ax206: LCD dimensions: %ux%u\n",
		 dpf->width, dpf->height);
out:
	kfree(buf);
	return ret;
}

/*
 * ax206_usb_set_backlight() - set backlight brightness (0-7)
 *
 * Acquires usb_mutex; must NOT be called with it already held.
 * Safe to call from workqueue.
 */
int ax206_usb_set_backlight(struct ax206_dev *dpf, int value)
{
	u8 cmd[16] = {
		0xcd, 0x00, 0x00, 0x00,
		0x00, 0x06, AX206_CMD_SETPROPERTY,
		AX206_PROP_BRIGHTNESS, 0x00,
		0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00
	};
	int ret;

	if (value < AX206_BL_MIN) value = AX206_BL_MIN;
	if (value > AX206_BL_MAX) value = AX206_BL_MAX;

	cmd[9]  = (u8)value;
	cmd[10] = (u8)(value >> 8);

	mutex_lock(&dpf->usb_mutex);
	ret = ax206_usb_cmd(dpf, cmd, USB_DIR_OUT, NULL, 0);
	mutex_unlock(&dpf->usb_mutex);
	return ret;
}
EXPORT_SYMBOL_GPL(ax206_usb_set_backlight);

/*
 * ax206_usb_blit() - send an RGB565 rectangle to the display
 *
 * Acquires usb_mutex; must NOT be called with it already held.
 * Safe to call from workqueue.
 */
int ax206_usb_blit(struct ax206_dev *dpf, const u8 *buf, size_t len,
		 u16 x0, u16 y0, u16 x1, u16 y1)
{
	u8 cmd[16] = {
		0xcd, 0x00, 0x00, 0x00,
		0x00, 0x06, AX206_CMD_BLIT,
		0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00,
		0x00
	};
	int ret;

	cmd[7]  = (u8)x0;
	cmd[8]  = (u8)(x0 >> 8);
	cmd[9]  = (u8)y0;
	cmd[10] = (u8)(y0 >> 8);
	cmd[11] = (u8)(x1 - 1);
	cmd[12] = (u8)((x1 - 1) >> 8);
	cmd[13] = (u8)(y1 - 1);
	cmd[14] = (u8)((y1 - 1) >> 8);

	mutex_lock(&dpf->usb_mutex);
	ret = ax206_usb_cmd(dpf, cmd, USB_DIR_OUT, (u8 *)buf, len);
	mutex_unlock(&dpf->usb_mutex);
	return ret;
}
EXPORT_SYMBOL_GPL(ax206_usb_blit);

/*
 * ax206_usb_fill_black() - fill entire display with black (RGB565 zero)
 *
 * Allocates a zero-filled buffer and blits it.
 * Used during blank and disconnect.
 */
int ax206_usb_fill_black(struct ax206_dev *dpf)
{
	size_t size = (size_t)dpf->width * dpf->height * AX206_BPP;
	u8 *black;
	int ret;

	black = kzalloc(size, GFP_KERNEL);
	if (!black)
		return -ENOMEM;

	ret = ax206_usb_blit(dpf, black, size, 0, 0, dpf->width, dpf->height);
	kfree(black);
	return ret;
}
EXPORT_SYMBOL_GPL(ax206_usb_fill_black);
