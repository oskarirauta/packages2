// SPDX-License-Identifier: GPL-2.0-only
/*
 * beada-usb.c – BeadaPanel USB transport layer
 *
 * Raw USB I/O helpers and endpoint discovery.  This file knows only about
 * bytes and endpoints – it has no knowledge of Panel-Link or Status-Link
 * message formats.  Protocol-aware logic lives in beada-pipeline.c.
 *
 * BeadaPanel USB layout (interface 0, vendor-class 0xFF):
 *
 *   EP 0x01  bulk-OUT  Panel-Link: pixel data from host to display
 *   EP 0x02  bulk-OUT  Status-Link: control commands from host
 *   EP 0x82  bulk-IN   Status-Link: control responses from device
 *
 * The Status-Link endpoints are absent on some firmware versions.
 * beada_find_endpoints() detects this and sets has_statuslink accordingly.
 */

#include <linux/usb.h>

#include "beada-priv.h"

/* ------------------------------------------------------------------ */
/* Raw transfer helpers                                                 */
/* ------------------------------------------------------------------ */

/**
 * beada_bulk_out() - Synchronous bulk-OUT transfer
 * @beada:      Device handle
 * @ep:         Endpoint number (address without direction bit)
 * @data:       DMA-safe source buffer (caller is responsible)
 * @len:        Bytes to send
 * @timeout_ms: Transfer timeout in milliseconds
 *
 * Wrapper around usb_bulk_msg() that adds structured error reporting and
 * treats short writes as errors (bulk-OUT short writes are always a bug on
 * this device).
 *
 * Caller must hold @beada->xfer_lock.
 * Returns 0 on success, negative errno on failure.
 */
int beada_bulk_out(struct beada_mfd_dev *beada, u8 ep,
		   const void *data, size_t len, unsigned int timeout_ms)
{
	int transferred;
	int ret;

	/*
	 * usb_bulk_msg() takes a void * even for OUT transfers because the
	 * USB DMA infrastructure needs to map it.  The const-cast is safe:
	 * bulk-OUT never modifies the source buffer.
	 */
	ret = usb_bulk_msg(beada->udev,
			   usb_sndbulkpipe(beada->udev, ep),
			   (void *)data, (int)len,
			   &transferred, timeout_ms);
	if (ret) {
		dev_err(beada->dev,
			"bulk-OUT ep=0x%02x %zu B: %d\n", ep, len, ret);
		return ret;
	}

	if (transferred != (int)len) {
		dev_err(beada->dev,
			"bulk-OUT ep=0x%02x short write: %d/%zu B\n",
			ep, transferred, len);
		return -EIO;
	}

	return 0;
}

/**
 * beada_bulk_in() - Synchronous bulk-IN transfer
 * @beada:      Device handle
 * @ep:         Endpoint number (address without direction bit; the pipe
 *              macro adds the IN direction bit automatically)
 * @data:       DMA-safe destination buffer
 * @len:        Exact number of bytes expected
 * @timeout_ms: Transfer timeout in milliseconds
 *
 * Caller must hold @beada->xfer_lock.
 * Returns 0 on success, negative errno on failure.
 */
int beada_bulk_in(struct beada_mfd_dev *beada, u8 ep,
		  void *data, size_t len, unsigned int timeout_ms)
{
	int transferred;
	int ret;

	ret = usb_bulk_msg(beada->udev,
			   usb_rcvbulkpipe(beada->udev, ep),
			   data, (int)len,
			   &transferred, timeout_ms);
	if (ret) {
		dev_err(beada->dev,
			"bulk-IN ep=0x%02x %zu B: %d\n", ep, len, ret);
		return ret;
	}

	if (transferred != (int)len) {
		dev_err(beada->dev,
			"bulk-IN ep=0x%02x short read: %d/%zu B\n",
			ep, transferred, len);
		return -EIO;
	}

	return 0;
}

/* ------------------------------------------------------------------ */
/* Endpoint discovery                                                   */
/* ------------------------------------------------------------------ */

/**
 * beada_find_endpoints() - Discover and record USB bulk endpoints
 * @beada: Device handle (@intf must already point to interface 0)
 *
 * Logs every endpoint found on interface 0 to dmesg.
 *
 * BeadaPanel interface 0 has four bulk endpoints forming two pairs:
 *
 *   Endpoint pair 1 (ep_num=1):  0x01 OUT  +  0x81 IN
 *   Endpoint pair 2 (ep_num=2):  0x02 OUT  +  0x82 IN
 *
 * Assignment:
 *   Lowest-numbered OUT pair → Panel-Link  (ep_pl_out / ep_pl_in)
 *   Next OUT pair            → Status-Link (ep_sl_out / ep_sl_in)
 *
 * Pairing is done by endpoint NUMBER (lower nibble of address), not by
 * the order in which the descriptor lists them.  This avoids the previous
 * bug where the first IN endpoint found (0x81) was incorrectly assigned
 * as ep_sl_in instead of the correct 0x82.
 *
 * ep_pl_in (0x81) is preserved separately because newer firmware (≥ 7.x)
 * proactively pushes Status-Link INFO packets on this endpoint rather than
 * waiting for a GET_INFO request on ep_sl_out / ep_sl_in.
 *
 * Returns 0 on success, -ENODEV if no bulk-OUT endpoint is found.
 */
int beada_find_endpoints(struct beada_mfd_dev *beada)
{
	struct usb_host_interface *alt = beada->intf->cur_altsetting;
	u8 out_addr[8] = {};	/* bulk-OUT addresses in discovery order */
	int n_out = 0;
	unsigned int i;

	dev_dbg(beada->dev, "interface 0 has %u endpoint(s):\n",
		alt->desc.bNumEndpoints);

	for (i = 0; i < alt->desc.bNumEndpoints; i++) {
		struct usb_endpoint_descriptor *ep =
				&alt->endpoint[i].desc;
		const char *xfer =
			usb_endpoint_xfer_bulk(ep) ? "bulk" :
			usb_endpoint_xfer_int(ep)  ? "int"  :
			usb_endpoint_xfer_isoc(ep) ? "isoc" : "ctrl";

		dev_dbg(beada->dev, "  EP 0x%02x  %-4s  %-3s  maxpkt=%u\n",
			ep->bEndpointAddress, xfer,
			usb_endpoint_dir_in(ep) ? "IN" : "OUT",
			le16_to_cpu(ep->wMaxPacketSize));

		if (usb_endpoint_xfer_bulk(ep) && usb_endpoint_dir_out(ep) &&
		    n_out < ARRAY_SIZE(out_addr))
			out_addr[n_out++] = ep->bEndpointAddress;
	}

	if (!n_out) {
		dev_err(beada->dev, "no bulk-OUT endpoint found\n");
		return -ENODEV;
	}

	/*
	 * Assign Panel-Link to the first (lowest) OUT endpoint, and pair it
	 * with the IN endpoint that has the same endpoint number.
	 *
	 * USB convention: for an OUT endpoint with address N (bit 7 = 0),
	 * the paired IN endpoint has address (N | 0x80).
	 */
	beada->ep_pl_out = out_addr[0];
	beada->ep_pl_in  = out_addr[0] | 0x80;

	if (n_out >= 2) {
		beada->ep_sl_out = out_addr[1];
		beada->ep_sl_in  = out_addr[1] | 0x80;
	}

	beada->has_statuslink = (beada->ep_sl_out != 0);

	dev_dbg(beada->dev,
		"ep_pl_out=0x%02x ep_pl_in=0x%02x  ep_sl_out=0x%02x ep_sl_in=0x%02x  statuslink=%s\n",
		beada->ep_pl_out, beada->ep_pl_in,
		beada->ep_sl_out, beada->ep_sl_in,
		beada->has_statuslink ? "yes" : "no");

	return 0;
}
