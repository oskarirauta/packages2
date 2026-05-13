// SPDX-License-Identifier: GPL-2.0-only
/*
 * beada-drm-edid.c – BeadaPanel DRM: EDID 1.3 builder
 *
 * BeadaPanel has no real EDID EEPROM; this file builds a 128-byte EDID
 * block in memory and attaches it to the connector so userspace tools
 * (xrandr, edid-decode, compositor policy) see a properly identified
 * display.
 *
 * The block is rebuilt every time the rotation changes (so the detailed
 * timing descriptor and physical dimension fields stay in sync with the
 * logical orientation).
 *
 * Layout follows VESA E-EDID 1.3, derived from the mpro driver's
 * reference implementation with these BeadaPanel-specific changes:
 *
 *   - Manufacturer "NXE" (closest 3-letter to NXElec; AOC's "NXE" code is
 *     unassigned in the PNP ID registry as of 2025)
 *   - Product ID encoded from BeadaPanel model name + dimensions
 *   - Monitor name "BeadaPanel <model>" (e.g. "BeadaPanel 5C")
 *   - Detailed timing matches beada-drm-conn.c (same CVT-style numbers)
 */

#include <linux/string.h>

#include <drm/drm_connector.h>
#include <drm/drm_edid.h>
#include <drm/drm_print.h>

#include "beada-drm.h"

/* ------------------------------------------------------------------ */
/* Constants                                                            */
/* ------------------------------------------------------------------ */

#define BEADA_EDID_SIZE		128
#define BEADA_EDID_MFG		"NXE"
/*
 * Year placed in the EDID "year of manufacture" field.  EDID encodes it
 * as offset from 1990 in a single byte, so we pick a fixed plausible year
 * rather than reading the wall clock — that would put the value in the
 * EDID dependent on the host's RTC state at probe, which is messy and
 * has no semantic value.
 */
#define BEADA_EDID_YEAR		2025

/* ------------------------------------------------------------------ */
/* Helpers                                                              */
/* ------------------------------------------------------------------ */

/**
 * pnp_id() - Encode a 3-letter PNP manufacturer code as two big-endian
 *            bytes per VESA E-EDID 1.3, 3.4.1.
 */
static u16 pnp_id(const char *mfg)
{
	return ((mfg[0] - '@') << 10) |
	       ((mfg[1] - '@') <<  5) |
		(mfg[2] - '@');
}

/**
 * write_descriptor_text() - Fill an 18-byte descriptor block with text
 * @desc: 18-byte descriptor area
 * @type: Descriptor tag (0xFC = monitor name, 0xFD = range limits, etc.)
 * @text: NUL-terminated text, truncated to 13 chars then NL-terminated
 *
 * Descriptors have the layout:
 *   [0..1]  0x0000  (clock = 0 marks this as a descriptor not a timing)
 *   [2]     0x00    reserved
 *   [3]     tag
 *   [4]     0x00    reserved
 *   [5..17] payload (text padded with 0x20, terminated with 0x0a)
 */
static void write_descriptor_text(u8 *desc, u8 type, const char *text)
{
	size_t len = min_t(size_t, strlen(text), 13);

	desc[0] = 0x00;
	desc[1] = 0x00;
	desc[2] = 0x00;
	desc[3] = type;
	desc[4] = 0x00;
	memcpy(&desc[5], text, len);
	if (len < 13)
		desc[5 + len] = 0x0a;	/* terminator */
	for (size_t i = 5 + len + (len < 13 ? 1 : 0); i < 18; i++)
		desc[i] = 0x20;		/* space padding */
}

/**
 * write_detailed_timing() - Fill an 18-byte detailed timing descriptor
 *
 * Encodes a CVT-style 60 Hz timing whose numbers are picked to match the
 * mode advertised by beada-drm-conn.c (BEADA_TIMING_* constants).  All
 * values are derived from @bdev so that swapping rotation automatically
 * swaps the timing too.
 */
static void write_detailed_timing(u8 *dt, struct beada_drm_dev *bdev)
{
	u32 w  = bdev->width;
	u32 h  = bdev->height;
	u32 wm, hm;
	u32 hblank = BEADA_TIMING_HFP + BEADA_TIMING_HSYNC + BEADA_TIMING_HBP;
	u32 vblank = BEADA_TIMING_VFP + BEADA_TIMING_VSYNC + BEADA_TIMING_VBP;
	u32 htotal = w + hblank;
	u32 vtotal = h + vblank;
	u32 clock_10khz =
		DIV_ROUND_CLOSEST(htotal * vtotal * BEADA_TIMING_REFRESH,
				  10000);

	/* Physical mm — swapped with rotation, same as the connector reports */
	if (beada_rotation_swaps_dims(bdev->rotation)) {
		wm = bdev->panel->height_mm;
		hm = bdev->panel->width_mm;
	} else {
		wm = bdev->panel->width_mm;
		hm = bdev->panel->height_mm;
	}

	/* Pixel clock in 10 kHz units, little-endian */
	dt[0]  =  clock_10khz       & 0xff;
	dt[1]  = (clock_10khz >> 8) & 0xff;

	/* Horizontal active / blanking */
	dt[2]  =  w      & 0xff;
	dt[3]  =  hblank & 0xff;
	dt[4]  = (((w      >> 8) & 0xf) << 4) | ((hblank >> 8) & 0xf);

	/* Vertical active / blanking */
	dt[5]  =  h      & 0xff;
	dt[6]  =  vblank & 0xff;
	dt[7]  = (((h      >> 8) & 0xf) << 4) | ((vblank >> 8) & 0xf);

	/* Sync offsets and widths */
	dt[8]  = BEADA_TIMING_HFP   & 0xff;
	dt[9]  = BEADA_TIMING_HSYNC & 0xff;
	dt[10] = ((BEADA_TIMING_VFP   & 0xf) << 4) |
		  (BEADA_TIMING_VSYNC & 0xf);
	dt[11] = (((BEADA_TIMING_HFP   >> 8) & 0x3) << 6) |
		 (((BEADA_TIMING_HSYNC >> 8) & 0x3) << 4) |
		 (((BEADA_TIMING_VFP   >> 4) & 0x3) << 2) |
		  ((BEADA_TIMING_VSYNC >> 4) & 0x3);

	/* Physical size in mm */
	dt[12] =  wm        & 0xff;
	dt[13] =  hm        & 0xff;
	dt[14] = (((wm >> 8) & 0xf) << 4) | ((hm >> 8) & 0xf);

	/* Border (zero), features (digital separate sync, +H/+V) */
	dt[15] = 0;
	dt[16] = 0;
	dt[17] = 0x18;
}

/**
 * write_range_limits() - Fill an 18-byte range-limits descriptor (tag 0xFD)
 *
 * Tells userspace the min/max horizontal and vertical rates the display
 * accepts.  Numbers chosen wide enough that BeadaPanel resolutions all
 * fit comfortably.
 */
static void write_range_limits(u8 *desc, u32 max_clock_10khz)
{
	desc[0]  = 0x00;
	desc[1]  = 0x00;
	desc[2]  = 0x00;
	desc[3]  = 0xfd;
	desc[4]  = 0x00;
	desc[5]  = 50;					/* min V rate (Hz)  */
	desc[6]  = 75;					/* max V rate (Hz)  */
	desc[7]  = 30;					/* min H rate (kHz) */
	desc[8]  = 80;					/* max H rate (kHz) */
	desc[9]  = (max_clock_10khz / 100) + 1;		/* max pixel clock,
							   10 MHz units     */
	desc[10] = 0;
	desc[11] = 0x0a;
	for (size_t i = 12; i < 18; i++)
		desc[i] = 0x20;
}

/* ------------------------------------------------------------------ */
/* Public entry point                                                   */
/* ------------------------------------------------------------------ */

int beada_edid_update(struct beada_drm_dev *bdev)
{
	u8 edid[BEADA_EDID_SIZE];
	char monitor_name[16];
	u16 mfg;
	u32 max_clock_10khz;
	int sum = 0;
	size_t i;
	int ret;

	memset(edid, 0, sizeof(edid));

	/* 0x00..0x07 – Fixed header pattern (00 FF FF FF FF FF FF 00) */
	for (i = 1; i < 7; i++)
		edid[i] = 0xff;

	/* 0x08..0x09 – PNP manufacturer ID (big-endian) */
	mfg = pnp_id(BEADA_EDID_MFG);
	edid[0x08] = (mfg >> 8) & 0xff;
	edid[0x09] =  mfg       & 0xff;

	/* 0x0a..0x0b – Product code: derived from native panel size */
	edid[0x0a] =  bdev->panel->width        & 0xff;
	edid[0x0b] = (bdev->panel->width  >> 8) & 0xff;

	/*
	 * 0x0c..0x0f – 32-bit serial number.  We weave the resolution
	 * together to give each panel model a unique value without any
	 * runtime state.
	 */
	edid[0x0c] =  bdev->panel->width        & 0xff;
	edid[0x0d] =  bdev->panel->height       & 0xff;
	edid[0x0e] = (bdev->panel->width  >> 8) & 0xff;
	edid[0x0f] = (bdev->panel->height >> 8) & 0xff;

	/* 0x10..0x11 – Week and year of manufacture */
	edid[0x10] = 1;
	edid[0x11] = BEADA_EDID_YEAR - 1990;

	/* 0x12..0x13 – EDID version 1.3 */
	edid[0x12] = 1;
	edid[0x13] = 3;

	/* 0x14 – Basic display parameters: digital input */
	edid[0x14] = 0x80;

	/*
	 * 0x15..0x16 – Maximum image size in cm.  Uses NATIVE dimensions
	 * (not rotated) — this field describes the physical panel, not the
	 * current viewing orientation.
	 */
	edid[0x15] = (bdev->panel->width_mm  + 9) / 10;
	edid[0x16] = (bdev->panel->height_mm + 9) / 10;

	/* 0x17 – Gamma 2.20 encoded as 220 - 100 = 120 */
	edid[0x17] = 120;

	/* 0x18 – Feature support: digital, RGB, preferred timing in DTD0 */
	edid[0x18] = 0x0a;

	/*
	 * 0x19..0x22 – sRGB-style chromaticity defaults.  We don't have
	 * measured panel chromaticities; sRGB is the best generic guess.
	 */
	edid[0x19] = 0xee;
	edid[0x1a] = 0x91;
	edid[0x1b] = 0xa3;
	edid[0x1c] = 0x54;
	edid[0x1d] = 0x4c;
	edid[0x1e] = 0x99;
	edid[0x1f] = 0x26;
	edid[0x20] = 0x0f;
	edid[0x21] = 0x50;
	edid[0x22] = 0x54;

	/* 0x23..0x25 – Established timings: none (all zero already) */

	/* 0x26..0x35 – Standard timings: unused (0x01 0x01) */
	for (i = 0x26; i < 0x36; i++)
		edid[i] = 0x01;

	/* 0x36..0x47 – Detailed timing descriptor (preferred mode) */
	write_detailed_timing(&edid[0x36], bdev);

	/* 0x48..0x59 – Descriptor #2: monitor name */
	scnprintf(monitor_name, sizeof(monitor_name),
		  "BeadaPanel %s", bdev->panel->model);
	write_descriptor_text(&edid[0x48], 0xfc, monitor_name);

	/* 0x5a..0x6b – Descriptor #3: range limits */
	max_clock_10khz =
		DIV_ROUND_CLOSEST((u32)(bdev->width + 200) *
				  (bdev->height + 50) *
				  BEADA_TIMING_REFRESH, 10000);
	write_range_limits(&edid[0x5a], max_clock_10khz);

	/* 0x6c..0x7d – Descriptor #4: dummy (tag 0x10) */
	edid[0x6f] = 0x10;

	/* 0x7e – Extension count: 0 */
	edid[0x7e] = 0;

	/* 0x7f – Checksum: sum of all 128 bytes must equal 0 mod 256 */
	for (i = 0; i < 127; i++)
		sum += edid[i];
	edid[0x7f] = (256 - (sum & 0xff)) & 0xff;

	ret = drm_connector_update_edid_property(&bdev->conn,
						 (struct edid *)edid);
	if (ret)
		drm_warn(&bdev->drm, "EDID update failed: %d\n", ret);
	return ret;
}
