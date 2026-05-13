// SPDX-License-Identifier: GPL-2.0-only
/*
 * beada-drm-conn.c – BeadaPanel DRM: connector callbacks
 *
 * One connector, always reported connected.  Advertises ONE mode at a
 * time, matching the current rotation in @bdev->width/@bdev->height.
 * The timings are CVT-style and only need to be self-consistent with
 * the EDID — there is no real scan-out.
 */

#include <drm/drm_atomic_helper.h>
#include <drm/drm_connector.h>
#include <drm/drm_modes.h>
#include <drm/drm_probe_helper.h>
#include <drm/drm_edid.h>

#include "beada-drm.h"

/* ------------------------------------------------------------------ */
/* Helpers                                                              */
/* ------------------------------------------------------------------ */

/**
 * beada_build_mode() - Construct one drm_display_mode for the current
 *                      logical resolution.
 *
 * Pixel clock is computed so that htotal × vtotal × refresh ≈ clock;
 * this is purely cosmetic but xrandr / edid-decode want it.
 */
static struct drm_display_mode *beada_build_mode(struct drm_connector *conn,
						 struct beada_drm_dev *bdev)
{
	struct drm_display_mode *mode = drm_mode_create(conn->dev);
	u32 htotal, vtotal, clock_khz;

	if (!mode)
		return NULL;

	mode->hdisplay    = bdev->width;
	mode->hsync_start = bdev->width  + BEADA_TIMING_HFP;
	mode->hsync_end   = mode->hsync_start + BEADA_TIMING_HSYNC;
	mode->htotal      = mode->hsync_end   + BEADA_TIMING_HBP;

	mode->vdisplay    = bdev->height;
	mode->vsync_start = bdev->height + BEADA_TIMING_VFP;
	mode->vsync_end   = mode->vsync_start + BEADA_TIMING_VSYNC;
	mode->vtotal      = mode->vsync_end   + BEADA_TIMING_VBP;

	htotal = mode->htotal;
	vtotal = mode->vtotal;
	clock_khz = DIV_ROUND_CLOSEST(htotal * vtotal *
				      BEADA_TIMING_REFRESH, 1000);
	mode->clock = clock_khz;

	mode->type = DRM_MODE_TYPE_DRIVER | DRM_MODE_TYPE_PREFERRED;
	drm_mode_set_name(mode);
	return mode;
}

/* ------------------------------------------------------------------ */
/* Connector helper funcs                                               */
/* ------------------------------------------------------------------ */

static int beada_conn_get_modes(struct drm_connector *conn)
{
	struct beada_drm_dev *bdev = conn_to_beada_drm(conn);
	struct drm_display_mode *mode;

	/*
	 * Physical dimensions, swapped with rotation: a 152×92 mm panel
	 * presents itself as 92×152 mm when rotated 90°/270°, which is
	 * what userspace expects for correct DPI calculations.
	 */
	if (beada_rotation_swaps_dims(bdev->rotation)) {
		conn->display_info.width_mm  = bdev->panel->height_mm;
		conn->display_info.height_mm = bdev->panel->width_mm;
	} else {
		conn->display_info.width_mm  = bdev->panel->width_mm;
		conn->display_info.height_mm = bdev->panel->height_mm;
	}

	mode = beada_build_mode(conn, bdev);
	if (!mode) {
		/* Last-resort fallback so the connector isn't completely
		 * mode-less if drm_mode_create() ever fails. */
		drm_add_modes_noedid(conn, bdev->width, bdev->height);
		drm_set_preferred_mode(conn, bdev->width, bdev->height);
		return 1;
	}

	drm_mode_probed_add(conn, mode);
	return 1;
}

const struct drm_connector_helper_funcs beada_conn_helper_funcs = {
	.get_modes = beada_conn_get_modes,
};

/* ------------------------------------------------------------------ */
/* Connector funcs                                                      */
/* ------------------------------------------------------------------ */

static enum drm_connector_status
beada_conn_detect(struct drm_connector *conn, bool force)
{
	/*
	 * Our connector is bound to a platform device whose lifetime is
	 * the USB device's lifetime, so by the time anyone asks it must
	 * be connected.
	 */
	return connector_status_connected;
}

const struct drm_connector_funcs beada_conn_funcs = {
	.detect                 = beada_conn_detect,
	.fill_modes             = drm_helper_probe_single_connector_modes,
	.destroy                = drm_connector_cleanup,
	.reset                  = drm_atomic_helper_connector_reset,
	.atomic_duplicate_state = drm_atomic_helper_connector_duplicate_state,
	.atomic_destroy_state   = drm_atomic_helper_connector_destroy_state,
};
