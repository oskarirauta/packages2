// SPDX-License-Identifier: GPL-2.0-only
/*
 * beada-drm-kms.c – BeadaPanel DRM: mode_config and rotation handling
 *
 * Sets up the mode-setting topology (mode_config, simple display pipe,
 * connector, gamma LUT) and exposes the runtime rotation switch.
 *
 * Connector, EDID, plane callbacks and vblank live in their own files;
 * this one only wires them together.
 */

#include <drm/drm_atomic_helper.h>
#include <drm/drm_color_mgmt.h>
#include <drm/drm_connector.h>
#include <drm/drm_fourcc.h>
#include <drm/drm_gem_atomic_helper.h>
#include <drm/drm_gem_framebuffer_helper.h>
#include <drm/drm_managed.h>
#include <drm/drm_probe_helper.h>
#include <drm/drm_simple_kms_helper.h>
#include <drm/drm_vblank.h>
#include <drm/drm_print.h>

#include "beada-drm.h"

/* ------------------------------------------------------------------ */
/* Pixel formats / modifiers                                            */
/* ------------------------------------------------------------------ */

/*
 * XRGB8888 is what compositors render; RGB565 is the native hardware
 * format and allows a fast path when the client already produces it.
 */
static const u32 beada_formats[] = {
	DRM_FORMAT_XRGB8888,
	DRM_FORMAT_RGB565,
};

static const u64 beada_modifiers[] = {
	DRM_FORMAT_MOD_LINEAR,
	DRM_FORMAT_MOD_INVALID,
};

/* ------------------------------------------------------------------ */
/* mode_config funcs                                                    */
/* ------------------------------------------------------------------ */

static const struct drm_mode_config_funcs beada_mode_config_funcs = {
	.fb_create     = drm_gem_fb_create_with_dirty,
	.atomic_check  = drm_atomic_helper_check,
	.atomic_commit = drm_atomic_helper_commit,
};

/* ------------------------------------------------------------------ */
/* Rotation switch                                                      */
/* ------------------------------------------------------------------ */

/**
 * beada_apply_rotation() - Change the active rotation
 *
 * Called from sysfs after validation.  Updates @rotation, swaps the
 * logical width/height when the orientation transposes the panel, retunes
 * the mode_config dimension limits, rebuilds the EDID, and fires a hotplug
 * event so compositors re-probe and pick up the new mode.
 *
 * Holds mode_config.mutex around the limits change so it can't race a
 * concurrent atomic_check.  EDID rebuild and the hotplug event happen
 * after the mutex is released; both helpers take their own locking.
 */
void beada_apply_rotation(struct beada_drm_dev *bdev, u16 new_rotation)
{
	struct drm_device *drm = &bdev->drm;
	bool swap;
	u32 w, h;

	if (bdev->rotation == new_rotation)
		return;

	swap = beada_rotation_swaps_dims(new_rotation);
	w = swap ? bdev->panel->height : bdev->panel->width;
	h = swap ? bdev->panel->width  : bdev->panel->height;

	mutex_lock(&drm->mode_config.mutex);
	bdev->rotation = new_rotation;
	bdev->width    = w;
	bdev->height   = h;

	drm->mode_config.min_width  = w;
	drm->mode_config.max_width  = w;
	drm->mode_config.min_height = h;
	drm->mode_config.max_height = h;
	mutex_unlock(&drm->mode_config.mutex);

	beada_edid_update(bdev);
	drm_kms_helper_hotplug_event(drm);
}

/* ------------------------------------------------------------------ */
/* beada_kms_init()                                                     */
/* ------------------------------------------------------------------ */

int beada_kms_init(struct beada_drm_dev *bdev)
{
	struct drm_device *drm = &bdev->drm;
	int ret;

	/* ── mode_config ─────────────────────────────────────────────── */
	ret = drmm_mode_config_init(drm);
	if (ret)
		return ret;

	/*
	 * The connector advertises ONE mode at a time (the one matching the
	 * current rotation).  min == max for both dimensions because the
	 * compositor must not pick anything else — the firmware doesn't
	 * allow it.
	 */
	drm->mode_config.funcs           = &beada_mode_config_funcs;
	drm->mode_config.min_width       = bdev->width;
	drm->mode_config.max_width       = bdev->width;
	drm->mode_config.min_height      = bdev->height;
	drm->mode_config.max_height      = bdev->height;
	drm->mode_config.preferred_depth = 16;
	drm->mode_config.prefer_shadow   = 1;

	/* ── Connector ───────────────────────────────────────────────── */
	drm_connector_helper_add(&bdev->conn, &beada_conn_helper_funcs);

	ret = drm_connector_init(drm, &bdev->conn, &beada_conn_funcs,
				 DRM_MODE_CONNECTOR_USB);
	if (ret)
		return ret;

	bdev->conn.status = connector_status_connected;
	bdev->conn.polled = 0;	/* always considered connected */

	drm_connector_attach_edid_property(&bdev->conn);

	/* Initial EDID for the default ROTATE_0 orientation */
	beada_edid_update(bdev);

	/* ── Simple display pipe ─────────────────────────────────────── */
	ret = drm_simple_display_pipe_init(drm, &bdev->pipe,
					   &beada_pipe_funcs,
					   beada_formats,
					   ARRAY_SIZE(beada_formats),
					   beada_modifiers,
					   &bdev->conn);
	if (ret)
		return ret;

	/* Damage clips are reported by the compositor but we always send
	 * the full frame to the device (firmware locks dimensions; partial
	 * updates not possible — see beada.h).  Enabling the iterator is
	 * still useful because it keeps userspace happy and the helper is
	 * a no-op for our purposes when we ignore the clips. */
	drm_plane_enable_fb_damage_clips(&bdev->pipe.plane);

	/* ── Gamma LUT on the CRTC ───────────────────────────────────── */
	/*
	 * 256-entry gamma LUT, no degamma, no CTM.  The plane update
	 * callback applies bdev->lut in beada_blit_rect() via the
	 * beada-drm-color.c helper.
	 */
	drm_crtc_enable_color_mgmt(&bdev->pipe.crtc, 0, false, 256);

	/* ── Vblank infrastructure ───────────────────────────────────── */
	ret = drm_vblank_init(drm, 1);
	if (ret)
		drm_warn(drm, "drm_vblank_init failed: %d\n", ret);

	/* ── Reset to sane defaults ──────────────────────────────────── */
	drm_mode_config_reset(drm);

	return 0;
}
