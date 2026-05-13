/* SPDX-License-Identifier: GPL-2.0 */
/*
 * MPro USB display driver — public API for child drivers.
 *
 * Copyright (C) Oskari Rauta <oskari.rauta@gmail.com>
 */
#ifndef _MPRO_H_
#define _MPRO_H_ 1

#include <linux/usb.h>
#include <linux/workqueue.h>
#include <linux/mfd/core.h>
#include <linux/mutex.h>
#include <linux/spinlock.h>
#include <linux/list.h>
#include <linux/string.h>

#ifndef MPRO_BPP
#define MPRO_BPP			16
#endif

#ifndef MPRO_USB_TIMEOUT_MS
#define MPRO_USB_TIMEOUT_MS		200
#endif

struct mpro_device;
struct mpro_drm;
struct mpro_xfer;		/* opaque, defined in mpro_pipeline.c */

/* ------------------------------------------------------------------ */
/* Model database                                                     */
/* ------------------------------------------------------------------ */

struct mpro_model_info {
	u32 screen_id;
	const char *name;
	const char *description;
	u32 width;
	u32 height;
	u32 width_mm;
	u32 height_mm;
	u32 margin;
	/* DRM_MODE_ROTATE_* | DRM_MODE_REFLECT_* */
	u16 native_rotation;
	/* Touch panel orientation compensation, applied to raw coords */
	bool touch_invert_x;
	bool touch_invert_y;
	bool touch_swap_xy;
};

/* ------------------------------------------------------------------ */
/* Screen state notifications                                         */
/* ------------------------------------------------------------------ */

/*
 * Child drivers register a listener to be notified when the DRM pipe
 * transitions between enabled and disabled. Used by the backlight to
 * follow the display state, and by the touchscreen to stop its URB
 * pipeline when the screen is blank.
 */
struct mpro_screen_listener {
	void (*screen_off)(void *priv);
	void (*screen_on)(void *priv);
	/*
	 * Called after a touch-wake or force-active transition to request
	 * an immediate repaint. The DRM child uses this to re-push the
	 * last rendered frame so the display shows valid content as soon
	 * as the backlight comes on. Optional — set to NULL if not needed.
	 */
	void (*screen_wake)(void *priv);
	void *priv;
	struct list_head node;	/* mpro->screen_listeners */
};

struct mpro_touch_listener {
	void (*touch_packet)(void *priv, const void *data, size_t len);
	void *priv;
	struct list_head node; /* entry in mpro_device.touch_listeners */
};

/* ------------------------------------------------------------------ */
/* Device state                                                       */
/* ------------------------------------------------------------------ */

struct mpro_device {
	struct usb_device *udev;
	struct usb_interface *intf;
	struct device *dma_dev;
	struct usb_anchor anchor;

	/* Synchronous control-message protection */
	struct mutex lock;
	u8 xfer_buf[128];
	u32 request_delay;

	/* Device identity */
	u32 screen;
	u32 version;
	u8 id[8];
	const struct mpro_model_info *model;
	char fw_string[64];
	s16 fw_major;
	s16 fw_minor;

	/* fbdev console mode disables USB autosuspend */
	bool fbdev_enabled;

	/* Async frame pipeline (mpro_pipeline.c) */
	spinlock_t state_lock;
	bool running;
	bool in_flight;
	struct mpro_xfer *current_xfer;
	struct mpro_xfer *pending_xfer;
	struct workqueue_struct *wq;
	struct work_struct complete_work;

	/* DRM child handle for back-reference on disconnect */
	struct mpro_drm *drm;

	/* LZ4 compression */
	int lz4_level;		/* 0=off, 1=fast, 2..12=HC */
	int lz4_threshold;	/* min frame size to compress */
	void *lz4_workmem;
	size_t lz4_workmem_size;
	struct mutex lz4_lock;	/* protects workmem */

	/* Screen state notification */
	struct mutex listeners_lock;
	struct list_head screen_listeners;

	/* Touch input pipeline (mpro_touch.c) */
	struct urb *touch_urb;
	void *touch_buf;
	dma_addr_t touch_buf_dma;
	atomic_t touch_submitted;
	spinlock_t touch_listeners_lock; /* held in IRQ context */
	struct list_head touch_listeners;

	/* Pipeline statistics */
	atomic_t stats_submitted;
	atomic_t stats_displayed;
	atomic_t stats_dropped;

	/* Virtual idle management (mpro_pm.c) */
	struct delayed_work	idle_work;
	atomic_t		active_refs;	/* references from children */
	bool			is_idle;	/* protected by listeners_lock */
	u32			idle_delay_ms;	/* 0 = disabled */
	bool			touch_wake_enabled;
	struct work_struct	wake_work;

	/* FPS tracking — EWMA of inter-frame deltas */
	atomic64_t last_frame_ns; /* timestamp of last frame */
	u32 ewma_period_ns;	  /* EWMA from jiffies-delta */
	spinlock_t fps_lock;	  /* protects ewma */

	/* Virtual idle statistics — protected by listeners_lock */
	u64			pm_active_total_ns;
	u64			pm_idle_total_ns;
	u64			pm_state_changed_ns;
	u32			pm_idle_count;
	u32			pm_wake_count;
	u32			pm_touch_wake_count;
};

/* ------------------------------------------------------------------ */
/* Public API for child drivers                                       */
/* ------------------------------------------------------------------ */

/* Synchronous request/response (mpro_protocol.c) */
int mpro_make_request(struct mpro_device *mpro,
		      const void *tx, size_t txlen, void *rx, size_t rxlen);
int mpro_request_u32(struct mpro_device *mpro,
		     const void *tx, size_t txlen, u32 * val);
int mpro_send_command(struct mpro_device *mpro,
		      const void *cmd, size_t cmd_len);
int mpro_get_firmware(struct mpro_device *mpro);

/* LZ4 helpers */
bool mpro_firmware_supports_lz4(const struct mpro_device *mpro);
int  mpro_lz4_workmem_alloc(struct mpro_device *mpro);

/* Color helpers (mpro_color.c) */
u8 mpro_pow_lut(u32 x, u32 g_x100);

/*
 * Async frame submission (mpro_pipeline.c) — non-blocking, latest-wins.
 * Caller's data is copied internally so it may be freed/reused
 * immediately on return.
 *
 * Returns: 0 on accept, -ESHUTDOWN if disconnected, -ENOMEM, -EINVAL.
 */
int mpro_send_full_frame(struct mpro_device *mpro,
			 const void *data, size_t len);
int mpro_send_partial_frame(struct mpro_device *mpro,
			    const void *data, u16 x, u16 y, u16 w, u16 h);

/* Internal pipeline init/shutdown (called from mpro_main.c) */
void mpro_io_init(struct mpro_device *mpro);
void mpro_io_shutdown(struct mpro_device *mpro);

/* Model + sysfs */
int mpro_get_model(struct mpro_device *mpro);
int mpro_sysfs_add(struct mpro_device *mpro);

/* Power management — virtual idle state (mpro_pm.c) */
void mpro_pm_init(struct mpro_device *mpro);
void mpro_pm_shutdown(struct mpro_device *mpro);
void mpro_active_get(struct mpro_device *mpro, bool *held);
void mpro_active_put(struct mpro_device *mpro, bool *held);
void mpro_pm_force_idle(struct mpro_device *mpro);
void mpro_pm_force_active(struct mpro_device *mpro);
void mpro_pm_wake_async(struct mpro_device *mpro);
void mpro_pm_touch_activity(struct mpro_device *mpro);

/* Screen state listeners (mpro_screen.c) */
int  mpro_screen_listener_register(struct mpro_device *mpro,
				   struct mpro_screen_listener *l);
void mpro_screen_listener_unregister(struct mpro_device *mpro,
				     struct mpro_screen_listener *l);
void mpro_screen_notify_off(struct mpro_device *mpro);
void mpro_screen_notify_on(struct mpro_device *mpro);
void mpro_screen_notify_wake(struct mpro_device *mpro);

/* Touch input pipeline (mpro_touch.c) */
int  mpro_touch_init(struct mpro_device *mpro);
void mpro_touch_shutdown(struct mpro_device *mpro);
int  mpro_touch_start(struct mpro_device *mpro);
void mpro_touch_stop(struct mpro_device *mpro);
int  mpro_touch_listener_register(struct mpro_device *mpro,
				  struct mpro_touch_listener *l);
void mpro_touch_listener_unregister(struct mpro_device *mpro,
				    struct mpro_touch_listener *l);

/*
 * Parse a gamma value formatted as "N", "N.M", or "N.MM" into an
 * integer gamma * 100. Returns 0 on success, -EINVAL on parse error.
 *
 *   "1"     -> 100
 *   "1.5"   -> 150
 *   "1.50"  -> 150
 *   "0.75"  -> 75
 *   "1.234" -> -EINVAL (too many fractional digits)
 *
 * Range validation is left to the caller — different child drivers
 * may impose different bounds.
 */
static inline int mpro_parse_gamma_x100(const char *buf, u32 *out)
{
	unsigned int whole, frac = 0;
	const char *dot;
	int frac_digits = 0;
	int n;

	dot = strchr(buf, '.');
	n = sscanf(buf, "%u.%u", &whole, &frac);
	if (n < 1)
		return -EINVAL;

	if (n == 2 && dot) {
		const char *p = dot + 1;

		while (*p >= '0' && *p <= '9') {
			frac_digits++;
			p++;
		}
		if (frac_digits == 1)
			frac *= 10;		/* "2.5" → 50 (i.e. 0.50) */
		else if (frac_digits > 2)
			return -EINVAL;
	}

	*out = whole * 100 + frac;
	return 0;
}

#endif /* _MPRO_H_ */
