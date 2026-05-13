/* SPDX-License-Identifier: GPL-2.0 */
/*
 * MPro USB display driver — internal protocol definitions.
 *
 * Wire-level constants and command bytes. Not exposed to child drivers.
 */
#ifndef _MPRO_INTERNAL_H_
#define _MPRO_INTERNAL_H_ 1

#include <linux/usb.h>

#define MPRO_REQ_CMD		0xb5
#define MPRO_REQ_STATUS		0xb6
#define MPRO_REQ_READ		0xb7
#define MPRO_CMD		0xb0

#define MPRO_USB_OUT		(USB_DIR_OUT | USB_TYPE_VENDOR | USB_RECIP_DEVICE)
#define MPRO_USB_IN		(USB_DIR_IN  | USB_TYPE_VENDOR | USB_RECIP_DEVICE)

#define MPRO_LZ4_MIN_MAJOR	 0
#define MPRO_LZ4_MIN_MINOR	22

static const u8 cmd_get_screen[5] = {
	0x51, 0x02, 0x04, 0x1f, 0xfc
};

static const u8 cmd_get_version[5] = {
	0x51, 0x02, 0x04, 0x1f, 0xf8
};

static const u8 cmd_get_id[5] = {
	0x51, 0x02, 0x08, 0x1f, 0xf0
};

static const u8 cmd_quit_sleepmode[6] = {
	0x00, 0x29, 0x00, 0x00, 0x00, 0x00
};

#endif /* _MPRO_INTERNAL_H_ */
