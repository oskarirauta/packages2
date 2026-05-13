// SPDX-License-Identifier: GPL-2.0-only
/*
 * ax206-drm-pm.c - AX206 DRM virtual power management
 *
 * The AX206 frame has no real suspend/resume hardware path.  Power
 * management is implemented with a software idle timer:
 *
 *   - After idle_timeout_s seconds without a frame update the
 *     display is "blanked": backlight off, screen filled black.
 *     New framebuffer data continues to be accepted into the shadow
 *     buffer but is not sent to the device.
 *
 *   - Any frame update, or a sysfs write to the power attribute,
 *     wakes the display: restores the backlight level stored in the
 *     backlight child driver, and blits the current shadow buffer.
 *
 * The idle_timeout_s value 0 disables the timer entirely.
 *
 * Copyright (C) 2025 Oskari Rauta <oskari.rauta@gmail.com>
 */

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/timer.h>
#include <linux/workqueue.h>
#include <linux/backlight.h>
#include <linux/device.h>

#include "ax206.h"
#include "ax206-drm-priv.h"

/*
 * ax206_idle_timer_reset() - restart the idle countdown
 *
 * Called on every frame update.  Does nothing if the timeout is 0.
 * If the display is currently blanked it schedules a resume instead.
 */
void ax206_idle_timer_reset(struct ax206_drm_dev *d)
{
	if (d->idle_active) {
		/* A frame arrived while blanked - wake up */
		queue_work(d->dpf->wq, &d->resume_work);
		return;
	}

	if (!d->idle_timeout_s)
		return;

	mod_timer(&d->idle_timer,
		  jiffies + (unsigned long)d->idle_timeout_s * HZ);
}

/*
 * ax206_idle_timer_fn() - timer callback, fires in softirq context
 *
 * Cannot do USB from here; schedules the idle work item.
 */
void ax206_idle_timer_fn(struct timer_list *t)
{
	struct ax206_drm_dev *d = timer_container_of(d, t, idle_timer);

	if (!d->idle_active)
		queue_work(d->dpf->wq, &d->idle_work);
}

/*
 * ax206_idle_enter() - blank the display (called from workqueue)
 */
void ax206_idle_enter(struct ax206_drm_dev *d)
{
	if (d->idle_active)
		return;

	d->idle_active    = true;
	d->dpf->blanked   = true;

	/* Turn off backlight first */
	ax206_usb_set_backlight(d->dpf, 0);

	/* Fill screen black */
	ax206_usb_fill_black(d->dpf);
}

/*
 * ax206_idle_leave() - restore display from blank (called from workqueue)
 */
void ax206_idle_leave(struct ax206_drm_dev *d)
{
	struct device *parent = d->dpf->dev;
	struct backlight_device *bd;
	int brightness = AX206_BL_DEFAULT;

	if (!d->idle_active)
		return;

	/* Find the backlight child to retrieve the stored brightness */
	bd = backlight_device_get_by_name("ax206-bl");
	if (bd) {
		brightness = backlight_get_brightness(bd);
		put_device(&bd->dev);
	} else {
		brightness = d->dpf->backlight;
	}

	d->idle_active  = false;
	d->dpf->blanked = false;

	/* Restore shadow buffer to display */
	ax206_usb_blit(d->dpf, d->shadow_buf, d->shadow_size,
		     0, 0, d->dpf->width, d->dpf->height);

	/* Restore backlight */
	ax206_usb_set_backlight(d->dpf, brightness);

	/* Restart idle timer */
	ax206_idle_timer_reset(d);

	dev_dbg(parent, "ax206-drm: resumed from virtual idle\n");
}

/* Work item: enter idle */
void ax206_idle_work_fn(struct work_struct *work)
{
	struct ax206_drm_dev *d =
		container_of(work, struct ax206_drm_dev, idle_work);

	ax206_idle_enter(d);
}

/* Work item: leave idle */
void ax206_resume_work_fn(struct work_struct *work)
{
	struct ax206_drm_dev *d =
		container_of(work, struct ax206_drm_dev, resume_work);

	ax206_idle_leave(d);
}
