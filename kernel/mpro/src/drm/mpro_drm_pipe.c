// SPDX-License-Identifier: GPL-2.0
/*
 * mpro_drm_pipe.c — DRM pipe & connector callbacks.
 *
 * Contents:
 *   - simple_display_pipe enable/disable/update/check
 *   - hrtimer-based vblank emulation
 *   - connector functions (atomic property get/set, get_modes)
 *   - mpro_drm__request_update() — DRM's interface to the parent pipeline
 *   - mpro_drm__fb_mark_dirty()  — push a damage region to the device
 *
 * All USB I/O is delegated to the parent (mpro_send_*). This file is
 * USB-agnostic.
 */

#include <linux/module.h>
#include <linux/slab.h>
#include <linux/usb.h>

#include <drm/drm_atomic_helper.h>
#include <drm/drm_damage_helper.h>
#include <drm/drm_gem_atomic_helper.h>
#include <drm/drm_gem_framebuffer_helper.h>
#include <drm/drm_probe_helper.h>
#include <drm/drm_simple_kms_helper.h>
#include <drm/drm_color_mgmt.h>
#include <drm/drm_vblank.h>
#include <drm/drm_print.h>

#include "mpro_drm.h"

/* ------------------------------------------------------------------ */
/* Idle state manager (mpro_pm.c)                                     */
/* ------------------------------------------------------------------ */

void mpro_drm__screen_wake(void *priv)
{
	struct mpro_drm *mdrm = priv;

	/*
	 * Re-push the last rendered frame after waking from virtual idle.
	 * The DRM pipe may still be enabled with valid content in the
	 * shadow buffer — without this the panel shows black until the
	 * compositor repaints.
	 */
	if (!READ_ONCE(mdrm->blanked) && mdrm->data)
		mpro_drm__request_update(mdrm, 0, 0,
					 mdrm->width, mdrm->height, true);
}

/* ------------------------------------------------------------------ */
/* Frame submission                                                   */
/* ------------------------------------------------------------------ */

int mpro_drm__request_update(struct mpro_drm *mdrm,
			     u16 x, u16 y, u16 w, u16 h, bool force)
{
	const size_t pitch     = (size_t)mdrm->width * 2;
	const size_t row_bytes = (size_t)w * 2;
	const u32    full_w    = mdrm->width;
	bool full_frame;
	void *bounce = NULL;
	int ret;
	u16 i;

	if (!mdrm->mpro)
		return -EINVAL;

	if (mdrm->blanked && !force)
		return 0;

	if (mdrm->disable_partial) {
		x = 0;
		y = 0;
		w = full_w;
		h = mdrm->height;
	}

	full_frame = (x == 0 && y == 0 && w == full_w && h == mdrm->height);

	if (full_frame)
		return mpro_send_full_frame(mdrm->mpro, mdrm->data,
					    mdrm->data_size);

	/* Contiguous region (full width, x=0): pass the pointer directly */
	if (w == full_w && x == 0) {
		void *src = (u8 *)mdrm->data + (size_t)y * pitch;

		return mpro_send_partial_frame(mdrm->mpro, src, x, y, w, h);
	}

	/* Non-contiguous: copy rows into a contiguous bounce buffer */
	bounce = kmalloc(row_bytes * h, GFP_KERNEL);
	if (!bounce)
		return -ENOMEM;

	for (i = 0; i < h; i++)
		memcpy((u8 *)bounce + (size_t)i * row_bytes,
		       (u8 *)mdrm->data + ((size_t)y + i) * pitch +
				(size_t)x * 2,
		       row_bytes);

	ret = mpro_send_partial_frame(mdrm->mpro, bounce, x, y, w, h);
	kfree(bounce);
	return ret;
}

void mpro_drm__fb_mark_dirty(struct mpro_drm *mdrm, struct iosys_map *src,
			     struct drm_framebuffer *fb, struct drm_rect *rect)
{
	struct drm_device *dev = fb->dev;
	struct drm_rect safe = *rect;
	int idx, ret = 0;
	u16 x, y, w, h;

	if (!drm_dev_enter(dev, &idx))
		return;

	if (!mpro_drm__clip_valid(&safe))
		goto out;

	safe.x1 = max(safe.x1, 0);
	safe.y1 = max(safe.y1, 0);
	safe.x2 = min(safe.x2, (int)mdrm->width);
	safe.y2 = min(safe.y2, (int)mdrm->height);

	if (!mpro_drm__clip_valid(&safe))
		goto out;

	ret = mpro_drm__copy_frame(mdrm, src, fb, &safe);
	if (ret)
		goto out;

	x = safe.x1;
	y = safe.y1;
	w = safe.x2 - safe.x1;
	h = safe.y2 - safe.y1;

	ret = mpro_drm__request_update(mdrm, x, y, w, h, false);

out:
	drm_dev_exit(idx);
	if (ret && ret != -ESHUTDOWN)
		DRM_ERROR("frame update failed: %d\n", ret);
}

/* ------------------------------------------------------------------ */
/* mode_config_funcs                                                  */
/* ------------------------------------------------------------------ */

const struct drm_mode_config_funcs mpro_drm__mode_config_funcs = {
	.fb_create	= drm_gem_fb_create_with_dirty,
	.atomic_check	= drm_atomic_helper_check,
	.atomic_commit	= drm_atomic_helper_commit,
};

/* ------------------------------------------------------------------ */
/* Connector                                                          */
/* ------------------------------------------------------------------ */

static int mpro_drm__connector_get_modes(struct drm_connector *connector)
{
	struct mpro_drm *mdrm =
		container_of(connector, struct mpro_drm, connector);
	struct drm_display_mode *mode;

	mode = drm_mode_create(connector->dev);
	if (!mode)
		return 0;

	mode->hdisplay    = mdrm->width;
	mode->hsync_start = mdrm->width + MPRO_TIMING_HSYNC_OFF;
	mode->hsync_end   = mdrm->width + MPRO_TIMING_HSYNC_OFF +
			    MPRO_TIMING_HSYNC_PULSE;
	mode->htotal      = mdrm->width + MPRO_TIMING_HSYNC_OFF +
			    MPRO_TIMING_HSYNC_PULSE + MPRO_TIMING_HBACK;

	mode->vdisplay    = mdrm->height;
	mode->vsync_start = mdrm->height + MPRO_TIMING_VSYNC_OFF;
	mode->vsync_end   = mdrm->height + MPRO_TIMING_VSYNC_OFF +
			    MPRO_TIMING_VSYNC_PULSE;
	mode->vtotal      = mdrm->height + MPRO_TIMING_VSYNC_OFF +
			    MPRO_TIMING_VSYNC_PULSE + MPRO_TIMING_VBACK;

	mode->clock = MPRO_TIMING_CLOCK_KHZ;
	mode->type  = DRM_MODE_TYPE_DRIVER | DRM_MODE_TYPE_PREFERRED;

	drm_mode_set_name(mode);
	drm_mode_probed_add(connector, mode);
	return 1;
}

static int mpro_drm__connector_set_property(struct drm_connector *connector,
					    struct drm_connector_state *state,
					    struct drm_property *property,
					    uint64_t val)
{
	struct mpro_drm *mdrm =
		container_of(connector, struct mpro_drm, connector);

	if (property == mdrm->brightness_prop) {
		mutex_lock(&mdrm->lut_lock);
		mdrm->brightness = (u32)val;
		mpro_drm__rebuild_combined_lut(mdrm);
		mutex_unlock(&mdrm->lut_lock);
		return 0;
	}
	return -EINVAL;
}

static int mpro_drm__connector_get_property(struct drm_connector *connector,
					    const struct drm_connector_state *state,
					    struct drm_property *property,
					    uint64_t *val)
{
	struct mpro_drm *mdrm =
		container_of(connector, struct mpro_drm, connector);

	if (property == mdrm->brightness_prop) {
		*val = mdrm->brightness;
		return 0;
	}
	return -EINVAL;
}

const struct drm_connector_helper_funcs mpro_drm__connector_helper_funcs = {
	.get_modes = mpro_drm__connector_get_modes,
};

const struct drm_connector_funcs mpro_drm__connector_funcs = {
	.fill_modes		= drm_helper_probe_single_connector_modes,
	.destroy		= drm_connector_cleanup,
	.reset			= drm_atomic_helper_connector_reset,
	.atomic_duplicate_state	= drm_atomic_helper_connector_duplicate_state,
	.atomic_destroy_state	= drm_atomic_helper_connector_destroy_state,
	.atomic_set_property	= mpro_drm__connector_set_property,
	.atomic_get_property	= mpro_drm__connector_get_property,
};

/* ------------------------------------------------------------------ */
/* vblank (hrtimer-emulated)                                          */
/* ------------------------------------------------------------------ */

enum hrtimer_restart mpro_drm__vblank_timer(struct hrtimer *timer)
{
	struct mpro_drm *mdrm =
		container_of(timer, struct mpro_drm, vblank_timer);

	/*
	 * The flag is only flipped from enable_vblank/disable_vblank.
	 * READ_ONCE prevents the compiler from caching across the check.
	 */
	if (!READ_ONCE(mdrm->vblank_enabled))
		return HRTIMER_NORESTART;

	drm_crtc_handle_vblank(&mdrm->pipe.crtc);
	hrtimer_forward_now(timer, mdrm->frame_time);
	return HRTIMER_RESTART;
}

static int mpro_drm__crtc_enable_vblank(struct drm_simple_display_pipe *pipe)
{
	struct mpro_drm *mdrm = container_of(pipe, struct mpro_drm, pipe);

	WRITE_ONCE(mdrm->vblank_enabled, true);
	hrtimer_start(&mdrm->vblank_timer, mdrm->frame_time, HRTIMER_MODE_REL);
	return 0;
}

static void mpro_drm__crtc_disable_vblank(struct drm_simple_display_pipe *pipe)
{
	struct mpro_drm *mdrm = container_of(pipe, struct mpro_drm, pipe);

	WRITE_ONCE(mdrm->vblank_enabled, false);
	hrtimer_cancel(&mdrm->vblank_timer);
}

/* ------------------------------------------------------------------ */
/* Pageflip helper                                                    */
/* ------------------------------------------------------------------ */

/*
 * Arm the pending page-flip event to fire on the NEXT vblank interrupt.
 *
 * drm_crtc_arm_vblank_event() internally calls drm_vblank_get(), which
 * starts the hrtimer if it isn't already running, and puts a reference
 * that is released automatically when the event fires.  The event
 * timestamp therefore always comes from an actual hrtimer fire — never
 * from drm_reset_vblank_timestamp() or a stale counter — so it is
 * guaranteed to be monotonically non-decreasing from Weston's
 * weston_output_finish_frame() perspective.
 *
 * While the event is armed, Weston sees a "pending page flip" and will
 * NOT call weston_output_finish_frame(now) from start_repaint_loop().
 * This eliminates the race that caused:
 *   Assertion failed: timespec_sub_to_nsec(stamp, &output->frame_time) >= 0
 */
static void mpro_drm__arm_pageflip(struct drm_simple_display_pipe *pipe)
{
	struct drm_crtc *crtc = &pipe->crtc;
	struct drm_pending_vblank_event *event;
	bool got_vblank;

	/*
	 * Acquire a vblank reference before taking event_lock.
	 * drm_crtc_arm_vblank_event() requires the caller to hold one —
	 * it does not call drm_vblank_get() internally. The reference is
	 * released automatically when the armed event fires via
	 * drm_crtc_handle_vblank(). If vblank is unavailable, fall back
	 * to drm_crtc_send_vblank_event() which timestamps immediately.
	 */
	got_vblank = (drm_crtc_vblank_get(crtc) == 0);

	spin_lock_irq(&crtc->dev->event_lock);
	event = crtc->state->event;
	if (event) {
		crtc->state->event = NULL;
		if (got_vblank) {
			drm_crtc_arm_vblank_event(crtc, event);
			got_vblank = false;  /* reference ownership transferred */
		} else {
			drm_crtc_send_vblank_event(crtc, event);
		}
	}
	spin_unlock_irq(&crtc->dev->event_lock);

	/* Drop the reference if no event consumed it */
	if (got_vblank)
		drm_crtc_vblank_put(crtc);
}

/*
 * Consume the CRTC's vblank event whenever an atomic commit completes,
 * including the fb=NULL (rmfb) path. Otherwise the DRM core warns:
 *   drm_atomic_helper_commit_hw_done: crtc_state->event != NULL
 */
static void mpro_drm__finish_pageflip(struct drm_simple_display_pipe *pipe)
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

/* ------------------------------------------------------------------ */
/* Pipe enable/disable/update                                         */
/* ------------------------------------------------------------------ */

static void mpro_drm__pipe_disable(struct drm_simple_display_pipe *pipe)
{
	struct mpro_drm *mdrm = container_of(pipe, struct mpro_drm, pipe);
	void *orig = mdrm->data;
	void *black;

	mdrm->blanked = true;

	/* Push a black frame before tearing the pipe down */
	black = kzalloc(mdrm->data_size, GFP_KERNEL);
	if (black) {
		mdrm->data = black;
		mpro_drm__request_update(mdrm, 0, 0,
					 mdrm->width, mdrm->height, true);
		mdrm->data = orig;
		kfree(black);
	}

	/* Send any pending vblank event BEFORE drm_crtc_vblank_off() */
	mpro_drm__finish_pageflip(pipe);

	/*
	 * Stop vblank — drm_crtc_vblank_off() invokes the disable_vblank
	 * callback, which in turn cancels the hrtimer.
	 */
	drm_crtc_vblank_off(&mdrm->pipe.crtc);

	mpro_screen_notify_off(mdrm->mpro);
	mpro_active_put(mdrm->mpro, &mdrm->active_held);
}

static void mpro_drm__pipe_enable(struct drm_simple_display_pipe *pipe,
				  struct drm_crtc_state *crtc_state,
				  struct drm_plane_state *plane_state)
{
	struct mpro_drm *mdrm = container_of(pipe, struct mpro_drm, pipe);
	struct drm_shadow_plane_state *shadow;
	struct drm_framebuffer *fb;
	struct drm_rect rect;
	bool was_blanked = mdrm->blanked;

	mpro_active_get(mdrm->mpro, &mdrm->active_held);
	mdrm->blanked = false;

	if (plane_state)
		WRITE_ONCE(mdrm->plane_rotation, (u16)plane_state->rotation);

	/*
	 * Start vblank BEFORE pushing any frames — this seeds the vblank
	 * timestamps the compositor relies on.
	 */
	drm_crtc_vblank_on(&mdrm->pipe.crtc);

	mpro_drm__apply_color_mgmt(mdrm, crtc_state);

	if (plane_state && plane_state->fb) {
		shadow = to_drm_shadow_plane_state(plane_state);
		if (shadow && !iosys_map_is_null(&shadow->data[0])) {
			fb = plane_state->fb;
			rect.x1 = 0;
			rect.y1 = 0;
			rect.x2 = fb->width;
			rect.y2 = fb->height;
			mpro_drm__fb_mark_dirty(mdrm, &shadow->data[0],
						fb, &rect);
		}
	} else if (was_blanked && mdrm->data) {
		mpro_drm__request_update(mdrm, 0, 0,
					 mdrm->width, mdrm->height, false);
	}

	mpro_screen_notify_on(mdrm->mpro);

	/*
	 * Send the page-flip event immediately on enable rather than
	 * arming it for the next hrtimer tick. There is no prior
	 * frame_time at this point, so drm_crtc_send_vblank_event()
	 * with the current timestamp is safe. This avoids the large
	 * negative repaint delay warning Weston emits when the first
	 * vblank arrives with a stale timestamp from before
	 * drm_crtc_vblank_on() had a chance to seed a proper value.
	 *
	 * Subsequent frame events go through mpro_drm__arm_pageflip()
	 * in pipe_update, which ensures monotonically increasing
	 * timestamps for the ongoing repaint loop.
	 */
	mpro_drm__finish_pageflip(pipe);
}

static void mpro_drm__pipe_update(struct drm_simple_display_pipe *pipe,
				  struct drm_plane_state *old_state)
{
	struct mpro_drm *mdrm = container_of(pipe, struct mpro_drm, pipe);
	struct drm_plane_state *state = pipe->plane.state;
	struct drm_crtc *crtc = &pipe->crtc;
	struct drm_crtc_state *crtc_state = crtc->state;
	struct drm_shadow_plane_state *shadow;
	struct drm_atomic_helper_damage_iter iter;
	struct drm_framebuffer *fb;
	struct drm_rect damage;
	struct drm_rect bb = {
		.x1 = INT_MAX, .y1 = INT_MAX,
		.x2 = INT_MIN, .y2 = INT_MIN,
	};
	bool any = false;
	u16 effective;

	mpro_drm__apply_color_mgmt(mdrm, crtc_state);

	if (state && state->fb) {

		WRITE_ONCE(mdrm->plane_rotation, (u16)state->rotation);

		effective = mpro_drm__compose_rotation(
			READ_ONCE(mdrm->native_rotation),
			(u16)state->rotation);

		shadow = to_drm_shadow_plane_state(state);
		if (shadow && !iosys_map_is_null(&shadow->data[0])) {
			fb = state->fb;

			drm_atomic_helper_damage_iter_init(&iter,
							   old_state, state);
			drm_atomic_for_each_plane_damage(&iter, &damage) {
				bb.x1 = min(bb.x1, damage.x1);
				bb.y1 = min(bb.y1, damage.y1);
				bb.x2 = max(bb.x2, damage.x2);
				bb.y2 = max(bb.y2, damage.y2);
				any = true;
			}

			/*
			 * For 90/180/270 rotations and reflected modes the
			 * damage region cannot be an arbitrary rectangle: the
			 * destination buffer used by the rotation helper has
			 * full-frame dimensions, not clip-bounds dimensions.
			 * Force a full-frame update so buffer size and
			 * coordinates stay consistent.
			 */
			if (effective != DRM_MODE_ROTATE_0) {
				bb.x1 = 0;
				bb.y1 = 0;
				bb.x2 = mdrm->width;
				bb.y2 = mdrm->height;
			}

			if (any)
				mpro_drm__fb_mark_dirty(mdrm, &shadow->data[0],
							fb, &bb);
		}
	}

	/*
	 * Arm the flip event for the next vblank, not the last one.
	 * See mpro_drm__arm_pageflip() for the full rationale.
	 */
	mpro_drm__arm_pageflip(pipe);
}

static int mpro_drm__crtc_atomic_check(struct drm_simple_display_pipe *pipe,
				       struct drm_plane_state *plane_state,
				       struct drm_crtc_state *crtc_state)
{
	struct mpro_drm *mdrm = container_of(pipe, struct mpro_drm, pipe);

	dev_dbg(&mdrm->mpro->intf->dev,
		"atomic_check: enable=%d active=%d mode_changed=%d "
		"plane_fb=%p\n",
		crtc_state->enable, crtc_state->active,
		crtc_state->mode_changed, plane_state ? plane_state->fb : NULL);

	/*
	 * Note: rotation is driven only through sysfs. mdrm->rotation is
	 * deliberately not synchronised with plane_state->rotation, since
	 * userspace doesn't set the latter — it defaults to ROTATE_0,
	 * which would clobber the value configured via sysfs.
	 */

	/* All-disable is always allowed (modeset shutdown) */
	if (!crtc_state->enable || !crtc_state->active)
		return 0;

	if (crtc_state->gamma_lut) {
		u32 length = drm_color_lut_size(crtc_state->gamma_lut);

		if (length != 256) {
			drm_dbg(&mdrm->drm,
				"Gamma LUT size must be 256, got %u\n",
				length);
			return -EINVAL;
		}
	}
	return 0;
}

const struct drm_simple_display_pipe_funcs mpro_drm__pipe_funcs = {
	.enable		= mpro_drm__pipe_enable,
	.disable	= mpro_drm__pipe_disable,
	.update		= mpro_drm__pipe_update,
	.check		= mpro_drm__crtc_atomic_check,
	.enable_vblank	= mpro_drm__crtc_enable_vblank,
	.disable_vblank	= mpro_drm__crtc_disable_vblank,
	DRM_GEM_SIMPLE_DISPLAY_PIPE_SHADOW_PLANE_FUNCS,
};
