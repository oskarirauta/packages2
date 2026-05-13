// SPDX-License-Identifier: GPL-2.0
/*
 * mpro_model.c — MPro device model database.
 *
 * Maps the screen_id (and optionally the version) returned by the
 * device's probe queries to a model_info entry containing dimensions,
 * margin, native rotation, and touch calibration flags.
 */

#include <linux/module.h>
#include <drm/drm_mode.h>

#include "mpro.h"

/*
 * Most MPro models share the same firmware orientation: the device's
 * native (0,0) origin is at the top-right from the user's perspective,
 * so the DRM-side native rotation is ROTATE_180 | REFLECT_X.
 */
static const struct mpro_model_info mpro_models[] = {
	{ 0x00000000, "MPRO",      "MPRO",            480,  800,   0,   0,   0,
	  DRM_MODE_ROTATE_180 | DRM_MODE_REFLECT_X, true,  false, false },
	{ 0x00000005, "MPRO-5",    "MPRO 5\"",        480,  854,  62, 110,   0,
	  DRM_MODE_ROTATE_180 | DRM_MODE_REFLECT_X, true,  false, false },
	/* Same screen_id as above, differentiated by margin */
	{ 0x00000005, "MPRO-5",    "MPRO 5\"",        480,  854,  62, 119, 320,
	  DRM_MODE_ROTATE_0,                        false, false, false },
	{ 0x00001005, "MPRO-5H",   "MPRO 5\" (OLED)", 720, 1280,  62, 110,   0,
	  DRM_MODE_ROTATE_180 | DRM_MODE_REFLECT_X, true,  false, false },
	{ 0x00000304, "MPRO-4IN3", "MPRO 4.3\"",      480,  800,  56,  94,   0,
	  DRM_MODE_ROTATE_180 | DRM_MODE_REFLECT_X, true,  false, false },
	{ 0x00000004, "MPRO-4",    "MPRO 4\"",        480,  800,  53,  86,   0,
	  DRM_MODE_ROTATE_180 | DRM_MODE_REFLECT_X, true,  false, false },
	{ 0x00000b04, "MPRO-4",    "MPRO 4\"",        480,  800,  53,  86,   0,
	  DRM_MODE_ROTATE_180 | DRM_MODE_REFLECT_X, true,  false, false },
	{ 0x00000104, "MPRO-4",    "MPRO 4\"",        480,  800,  53,  86,   0,
	  DRM_MODE_ROTATE_180 | DRM_MODE_REFLECT_X, true,  false, false },
	{ 0x00000007, "MPRO-6IN8", "MPRO 6.8\"",      800,  480,  89, 148,   0,
	  DRM_MODE_ROTATE_180 | DRM_MODE_REFLECT_X, true,  false, false },
	{ 0x00000403, "MPRO-3IN4", "MPRO 3.4\"",      800,  800,  88,  88,   0,
	  DRM_MODE_ROTATE_180 | DRM_MODE_REFLECT_X, true,  false, false },
	{ 0x0000000a, "MPRO-10",   "MPRO 10\"",      1024,  600, 222, 125,   0,
	  DRM_MODE_ROTATE_180 | DRM_MODE_REFLECT_X, true,  false, false },
};

static u32 mpro_detect_margin(struct mpro_device *mpro)
{
	u32 margin = 0;

	switch (mpro->screen) {
	case 0x00000005:
		/*
		 * MPRO-5:
		 *   version 0x00000003 = no margin
		 *   others              = 320 px margin
		 */
		if (mpro->version != 0x00000003)
			margin = 320;
		break;

	default:
		margin = 0;
		break;
	}

	dev_dbg(&mpro->intf->dev,
		"margin detect: screen=0x%08x version=0x%08x -> margin=%u\n",
		mpro->screen, mpro->version, margin);

	return margin;
}

int mpro_get_model(struct mpro_device *mpro)
{
	u32 margin = mpro_detect_margin(mpro);
	size_t i;

	for (i = 0; i < ARRAY_SIZE(mpro_models); i++) {
		if (mpro_models[i].screen_id == mpro->screen &&
		    mpro_models[i].margin    == margin) {
			mpro->model = &mpro_models[i];

			/* Fallback: screen_id == 0 is generic/unknown model */
			if (mpro->screen == 0x00000000) {
				dev_warn(&mpro->intf->dev,
					"Unknown MPro model (screen_id=0x%08x, version=0x%08x, "
					"margin=%u), using generic fallback. "
					"Touch calibration and resolution may be incorrect.\n",
					mpro->screen, mpro->version, margin);
			}
			return 0;
		}
	}

	mpro->model = NULL;
	return -ENODEV;
}
