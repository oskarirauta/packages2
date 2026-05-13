// SPDX-License-Identifier: GPL-2.0-only
/*
 * beada-drm-vblank.c – Software vblank emulation and flip-event handling
 *
 * BeadaPanel has no hardware vblank signal.  An hrtimer fires at 60 fps
 * and calls drm_crtc_handle_vblank(), giving DRM core a steady stream of
 * vblank timestamps that compositors use to pace rendering.
 *
 * Flip events are NEVER sent from pipe_update directly.  They are armed
 * for the NEXT vblank tick via drm_crtc_arm_vblank_event(), so the event
 * timestamp comes from a real hrtimer fire and is monotonically
 * non-decreasing.  This is what keeps Weston's
 *
 *   assert(timespec_sub_to_nsec(stamp, &output->frame_time) >= 0)
 *
 * from firing on first commit after pipe_enable — a problem we hit with
 * the previous "send event immediately" approach.
 *
 * State machine
 * ─────────────
 *   pipe_enable  → drm_crtc_vblank_on()         (timestamps now seeded)
 *                → beada_finish_pageflip()      (immediate, no prior tick)
 *   pipe_update  → beada_arm_pageflip()         (next vblank tick)
 *   pipe_disable → beada_finish_pageflip()      (drain before vblank_off)
 *                → drm_crtc_vblank_off()        (cancels hrtimer)
 *
 * @vblank_enabled is flipped only from beada_vblank_enable/disable, which
 * are called by DRM core from drm_crtc_vblank_on/off respectively.
 */

#include <linux/hrtimer.h>
#include <linux/spinlock.h>
#include <linux/version.h>

#include <drm/drm_crtc.h>
#include <drm/drm_simple_kms_helper.h>
#include <drm/drm_vblank.h>

#include "beada-drm.h"

/* ------------------------------------------------------------------ */
/* hrtimer callback                                                     */
/* ------------------------------------------------------------------ */

static enum hrtimer_restart beada_vblank_timer_cb(struct hrtimer *timer)
{
	struct beada_drm_dev *bdev =
		container_of(timer, struct beada_drm_dev, vblank_timer);
	unsigned long flags;
	bool enabled;

	spin_lock_irqsave(&bdev->vblank_lock, flags);
	enabled = bdev->vblank_enabled;
	spin_unlock_irqrestore(&bdev->vblank_lock, flags);

	if (!enabled)
		return HRTIMER_NORESTART;

	/*
	 * Advances the DRM vblank sequence counter and consumes any
	 * armed flip event with this tick's timestamp.  This is the only
	 * place where armed events are released; pipe_update never sends
	 * them directly.
	 */
	drm_crtc_handle_vblank(&bdev->pipe.crtc);

	hrtimer_forward_now(timer, bdev->frame_time);
	return HRTIMER_RESTART;
}

/* ------------------------------------------------------------------ */
/* Init + enable/disable                                                */
/* ------------------------------------------------------------------ */

void beada_vblank_init(struct beada_drm_dev *bdev)
{
	spin_lock_init(&bdev->vblank_lock);
	bdev->vblank_enabled = false;
	bdev->frame_time     = ns_to_ktime(NSEC_PER_SEC /
					   BEADA_TIMING_REFRESH);

	/*
	 * hrtimer_setup() (kernel ≥ 6.15) combines init and callback
	 * assignment.  On older kernels we do it in two steps.
	 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 15, 0)
	hrtimer_setup(&bdev->vblank_timer, beada_vblank_timer_cb,
		      CLOCK_MONOTONIC, HRTIMER_MODE_REL_SOFT);
#else
	hrtimer_init(&bdev->vblank_timer, CLOCK_MONOTONIC,
		     HRTIMER_MODE_REL_SOFT);
	bdev->vblank_timer.function = beada_vblank_timer_cb;
#endif
}

int beada_vblank_enable(struct drm_simple_display_pipe *pipe)
{
	struct beada_drm_dev *bdev = pipe_to_beada_drm(pipe);
	unsigned long flags;

	spin_lock_irqsave(&bdev->vblank_lock, flags);
	bdev->vblank_enabled = true;
	spin_unlock_irqrestore(&bdev->vblank_lock, flags);

	hrtimer_start(&bdev->vblank_timer, bdev->frame_time,
		      HRTIMER_MODE_REL_SOFT);
	return 0;
}

void beada_vblank_disable(struct drm_simple_display_pipe *pipe)
{
	struct beada_drm_dev *bdev = pipe_to_beada_drm(pipe);
	unsigned long flags;

	spin_lock_irqsave(&bdev->vblank_lock, flags);
	bdev->vblank_enabled = false;
	spin_unlock_irqrestore(&bdev->vblank_lock, flags);

	hrtimer_cancel(&bdev->vblank_timer);
}

/* ------------------------------------------------------------------ */
/* Flip-event helpers                                                   */
/* ------------------------------------------------------------------ */

void beada_arm_pageflip(struct drm_simple_display_pipe *pipe)
{
	struct drm_crtc *crtc = &pipe->crtc;
	struct drm_pending_vblank_event *event;
	bool got_vblank;

	/*
	 * drm_crtc_arm_vblank_event() requires the caller to hold a vblank
	 * reference; it does NOT take one itself.  The reference is released
	 * automatically when the event fires via drm_crtc_handle_vblank().
	 * If we can't get a reference (vblank disabled / racing teardown)
	 * fall back to sending the event immediately.
	 */
	got_vblank = (drm_crtc_vblank_get(crtc) == 0);

	spin_lock_irq(&crtc->dev->event_lock);
	event = crtc->state->event;
	if (event) {
		crtc->state->event = NULL;
		if (got_vblank) {
			drm_crtc_arm_vblank_event(crtc, event);
			got_vblank = false;	/* ref ownership transferred */
		} else {
			drm_crtc_send_vblank_event(crtc, event);
		}
	}
	spin_unlock_irq(&crtc->dev->event_lock);

	/* Drop the reference if no event consumed it */
	if (got_vblank)
		drm_crtc_vblank_put(crtc);
}

void beada_finish_pageflip(struct drm_simple_display_pipe *pipe)
{
	struct drm_crtc *crtc = &pipe->crtc;
	struct drm_pending_vblank_event *event;

	spin_lock_irq(&crtc->dev->event_lock);
	event = crtc->state->event;
	if (event) {
		drm_crtc_send_vblank_event(crtc, event);
		crtc->state->event = NULL;
	}
	spin_unlock_irq(&crtc->dev->event_lock);
}
