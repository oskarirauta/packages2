// SPDX-License-Identifier: GPL-2.0-only
/*
 * beada-drm-color.c – Gamma LUT management
 *
 * Mirrors the DRM-native gamma_lut property into bdev->lut, a per-channel
 * 8-bit table consumed directly by the blit code in beada-drm-plane.c.
 *
 * DRM's gamma_lut is a 256-entry array of drm_color_lut structs with
 * 16-bit channels.  We sample the high 8 bits of each channel into our
 * own array because the blit operates on 8-bit input pixels.
 *
 * @lut_lock serialises the mirror update (from pipe_update, sleepable
 * context) against the blit's lookup (also pipe_update — same thread).
 * The lock exists primarily to keep userspace gamma_lut changes coherent
 * across the next blit; contention is effectively zero.
 */

#include <drm/drm_color_mgmt.h>
#include <drm/drm_crtc.h>

#include "beada-drm.h"

void beada_apply_color_mgmt(struct beada_drm_dev *bdev,
			    struct drm_crtc_state *crtc_state)
{
	struct drm_color_lut *lut;
	u32 i;

	if (!crtc_state->color_mgmt_changed)
		return;

	mutex_lock(&bdev->lut_lock);

	if (!crtc_state->gamma_lut) {
		/* Userspace cleared the gamma — blit will use linear map */
		bdev->gamma_valid = false;
	} else {
		lut = (struct drm_color_lut *)crtc_state->gamma_lut->data;

		for (i = 0; i < 256; i++) {
			bdev->lut[0][i] = drm_color_lut_extract(lut[i].red,   8);
			bdev->lut[1][i] = drm_color_lut_extract(lut[i].green, 8);
			bdev->lut[2][i] = drm_color_lut_extract(lut[i].blue,  8);
		}
		bdev->gamma_valid = true;
	}

	mutex_unlock(&bdev->lut_lock);
}
