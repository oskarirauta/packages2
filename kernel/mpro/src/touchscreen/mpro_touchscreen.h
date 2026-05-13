/* SPDX-License-Identifier: GPL-2.0 */
/*
 * MPro touchscreen driver — internal header.
 *
 * Wire-protocol struct definitions and per-instance state.
 */
#ifndef _MPRO_TOUCHSCREEN_H_
#define _MPRO_TOUCHSCREEN_H_ 1

#include <linux/input.h>
#include <linux/mutex.h>
#include <linux/timer.h>

#include "../mpro.h"

#define MPRO_TOUCH_PKT_SIZE	14
#define MPRO_TOUCH_MAX_SLOTS	2

/*
 * Release watchdog timeout: how long to wait without packets before
 * assuming all fingers have been lifted. Normally the firmware sends
 * a state=1 packet on finger release, so the release happens
 * immediately — this watchdog is only a safety net for firmware bugs
 * and dropped USB packets.
 */
#define MPRO_TOUCH_RELEASE_WATCHDOG_MS	850

/*
 * Touch packet layout — corresponds to struct touch + struct point in
 * the libusb reference code.
 *
 * Bitfield interpretation:
 *   xh: high 4 bits of x | 2 reserved | 2 bits state
 *   yh: high 4 bits of y | 4 bits slot id (0..1; > 1 = unused)
 *
 * 12-bit coordinates are reassembled as: x = (xh.h << 8) | xl
 *
 * State code semantics, as observed on firmware v0.25 by USB-protocol
 * analysis (note: differs from the original userspace assumption):
 *   0 = touch start marker
 *   1 = release (finger lifted)
 *   2 = active touch (finger on screen, coordinates valid)
 *   3 = not observed in use
 */
union mpro_axis {
	struct {
		u8 h:4;
		u8 u:2;
		u8 f:2;		/* state */
	} x;
	struct {
		u8 h:4;
		u8 id:4;	/* slot id */
	} y;
	u8 c;
} __packed;

struct mpro_touch_point {
	union mpro_axis xh;
	u8 xl;
	union mpro_axis yh;
	u8 yl;
	u8 weight;	/* TODO: report as ABS_MT_PRESSURE if validated */
	u8 misc;
} __packed;

struct mpro_touch_packet {
	u8 unused[2];
	u8 count;	/* TODO: investigate whether usable */
	struct mpro_touch_point p[MPRO_TOUCH_MAX_SLOTS];
} __packed;

struct mpro_touch {
	struct mpro_device		*mpro;
	struct input_dev		*input;
	struct mpro_screen_listener	screen_listener;
	struct mpro_touch_listener	touch_listener;

	struct mutex			lock;
	bool				opened;		/* userspace open() */
	bool				screen_on;	/* DRM pipe state */

	/* Release watchdog — safety net for a missing state=1 packet */
	struct timer_list		release_timer;
	bool				any_active;
};

#endif /* _MPRO_TOUCHSCREEN_H_ */
