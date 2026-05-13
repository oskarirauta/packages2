// SPDX-License-Identifier: GPL-2.0-only
/*
 * ax206-drm-rotation.c - AX206 DRM rotation support
 *
 * Supports 8 rotation modes:
 *   0   = 0°              (landscape, natural)
 *   1   = 90°  CW
 *   2   = 180°
 *   3   = 270° CW
 *   4   = 0°   + horizontal flip
 *   5   = 90°  + horizontal flip
 *   6   = 180° + horizontal flip
 *   7   = 270° + horizontal flip
 *
 * ax206_rotate_pixel() maps a logical (lx,ly) coordinate to a physical
 * (px,py) coordinate on the AX206 framebuffer.
 *
 * Copyright (C) 2025 Oskari Rauta <oskari.rauta@gmail.com>
 */

#include <linux/kernel.h>

#include "ax206.h"
#include "ax206-drm-priv.h"

static const char * const ax206_rotation_names[AX206_ROTATE_COUNT] = {
	[AX206_ROTATE_0]        = "0",
	[AX206_ROTATE_90]       = "90",
	[AX206_ROTATE_180]      = "180",
	[AX206_ROTATE_270]      = "270",
	[AX206_ROTATE_0_HFLIP]  = "0-hflip",
	[AX206_ROTATE_90_HFLIP] = "90-hflip",
	[AX206_ROTATE_180_HFLIP]= "180-hflip",
	[AX206_ROTATE_270_HFLIP]= "270-hflip",
};

const char *ax206_rotation_name(int rot)
{
	if (rot < 0 || rot >= AX206_ROTATE_COUNT)
		return "unknown";
	return ax206_rotation_names[rot];
}

/*
 * ax206_drm_set_rotation() - configure rotation and update logical dimensions
 *
 * For 90/270-degree rotations the logical width and height are swapped
 * relative to the physical display.
 */
void ax206_drm_set_rotation(struct ax206_drm_dev *d, int rot)
{
	d->rotation = rot;

	switch (rot) {
	case AX206_ROTATE_90:
	case AX206_ROTATE_270:
	case AX206_ROTATE_90_HFLIP:
	case AX206_ROTATE_270_HFLIP:
		d->lwidth  = d->dpf->height;
		d->lheight = d->dpf->width;
		break;
	default:
		d->lwidth  = d->dpf->width;
		d->lheight = d->dpf->height;
		break;
	}
}

/*
 * ax206_rotate_pixel() - map logical (lx, ly) -> physical (px, py)
 *
 * Physical coordinates index directly into the AX206 frame buffer.
 */
void ax206_rotate_pixel(const struct ax206_drm_dev *d,
		      int lx, int ly, int *px, int *py)
{
	int w = (int)d->dpf->width;
	int h = (int)d->dpf->height;

	switch (d->rotation) {
	case AX206_ROTATE_0:
	default:
		*px = lx;
		*py = ly;
		break;

	case AX206_ROTATE_90:
		/* 90° CW: (lx, ly) -> (h-1-ly, lx) */
		*px = h - 1 - ly;
		*py = lx;
		break;

	case AX206_ROTATE_180:
		*px = w - 1 - lx;
		*py = h - 1 - ly;
		break;

	case AX206_ROTATE_270:
		/* 270° CW: (lx, ly) -> (ly, w-1-lx) */
		*px = ly;
		*py = w - 1 - lx;
		break;

	case AX206_ROTATE_0_HFLIP:
		*px = w - 1 - lx;
		*py = ly;
		break;

	case AX206_ROTATE_90_HFLIP:
		*px = ly;
		*py = lx;
		break;

	case AX206_ROTATE_180_HFLIP:
		*px = lx;
		*py = h - 1 - ly;
		break;

	case AX206_ROTATE_270_HFLIP:
		*px = h - 1 - ly;
		*py = w - 1 - lx;
		break;
	}
}
