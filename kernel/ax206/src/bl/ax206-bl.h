/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * ax206-bl.h - AX206 backlight child driver header
 *
 * Copyright (C) 2025 Oskari Rauta <oskari.rauta@gmail.com>
 */

#ifndef _AX206_BL_H
#define _AX206_BL_H

#include <linux/backlight.h>

/*
 * ax206_bl_get_brightness() - return the current backlight level (0-7)
 *
 * Used by the DRM child to restore brightness on resume.
 */
int ax206_bl_get_brightness(struct backlight_device *bd);

#endif /* _AX206_BL_H */
