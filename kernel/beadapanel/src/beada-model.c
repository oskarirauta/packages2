// SPDX-License-Identifier: GPL-2.0-only
/*
 * beada-model.c – BeadaPanel model lookup table
 *
 * Maps the GET_PANEL_INFO "model" field to pixel resolution and physical size.
 *
 * Model codes and names are taken from the official NXElec Status-Link USB
 * Panel Control Protocol Specification, Rev 1.3, table 2-3.
 *
 * Physical dimensions (width_mm / height_mm) come from the NXElec product
 * pages and the JT365 beada.c reference driver.
 *
 * To add a new model: append a row to beada_model_table[].  The driver
 * discovers the model at runtime via GET_PANEL_INFO; adding a row here is the
 * only code change needed.
 *
 * IMPORTANT: The model codes in the official spec differ from those used in
 * the earlier JT365 beada.c driver.  Always use the official spec values:
 *   0 = Model 7  (spec)   — NOT Model 5 as the JT365 driver assumed
 *   1 = Model 5  (spec)   — NOT Model 7 as the JT365 driver assumed
 *
 * Where official spec has been missing, fallback to JT365's spec.
 */

#include <linux/kernel.h>

#include "beada-priv.h"

/* ------------------------------------------------------------------ */
/* Model table                                                          */
/* ------------------------------------------------------------------ */

static const struct beada_model_entry beada_model_table[] = {
	/* ── Official models from Status-Link spec Rev 1.3 (table 2-3) ───── */
	/*  code   width  height  wmm  hmm  name */
	{   0,      800,   480,   188, 118,  "7"   }, /* Model 7              */
	{   1,      800,   480,   121,  76,  "5"   }, /* Model 5              */
	{   2,      480,  1280,    67, 181,  "6"   }, /* Model 6              */
	{   3,      320,   480,    70,  99,  "3"   }, /* Model 3              */
	{   4,      480,   800,    78, 116,  "4"   }, /* Model 4              */
	{  10,      800,   480,   121,  76,  "5C"  }, /* Model 5C             */
	{  11,      800,   480,   108,  65,  "5T"  }, /* Model 5T             */
	{  12,      800,   480,   110,  62,  "7C"  }, /* Model 7C             */
	{  13,      480,   320,    62,  40,  "3C"  }, /* Model 3C             */
	{  14,      800,   480,    94,  56,  "4C"  }, /* Model 4C             */
	{  15,     1280,   480,   161,  60,  "6C"  }, /* Model 6C             */
	{  16,     1280,   480,   161,  60,  "6S"  }, /* Model 6S             */
	{  17,      480,   480,    73,  73,  "2"   }, /* Model 2              */
	{  18,      480,   480,    90,  90,  "2W"  }, /* Model 2W             */
	{  19,     1280,   480,   190,  59,  "7S"  }, /* Model 7S             */
	{  20,      480,   854,    77, 140,  "5S"  }, /* Model 5S             */
	{  21,      480,  1920,   69,  235,  "8"   }, /* Model 8              */
	{  22,      440,  1920,   58,  253,  "11"  }, /* Model 11             */
	{  23,      462,  1920,   55,  226,  "9"   }, /* Model 9              */
	{  24,      480,  1920,   54,  219,  "Y"   }, /* Model Y              */
	{  25,      440,  1920,   58,  253,  "X"   }, /* Model X              */
	{  26,      462,  1920,   55,  226,  "Z"   }, /* Model Z              */
};

/*
 * Fallback used when GET_PANEL_INFO returns an unknown model code, or when
 * Status-Link is unavailable and the iProduct string can't be parsed.
 */
static const struct beada_model_entry beada_model_fallback = {
	0xFF, 800, 480, 108, 65, "?"
};

/* ------------------------------------------------------------------ */
/* Public (internal) lookup functions                                   */
/* ------------------------------------------------------------------ */

/**
 * beada_lookup_model() - Find panel geometry by model code
 * @os_version: The "model" field value from a GET_PANEL_INFO response
 *
 * Returns the matching entry, or &beada_model_fallback for unknown codes.
 * Never returns NULL.
 */
const struct beada_model_entry *beada_lookup_model(u8 os_version)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(beada_model_table); i++) {
		if (beada_model_table[i].os_version == os_version)
			return &beada_model_table[i];
	}

	return &beada_model_fallback;
}

/**
 * beada_lookup_model_by_modelid() - Find panel geometry by model identifier
 * @id: model identifier
 *
 * Fallback when Status-Link is unavailable and the model id is parsed from
 * the USB iProduct string ("BeadaPanel WxH").
 *
 * Returns the matching entry, or NULL if no entry matches.
 */
const struct beada_model_entry *beada_lookup_model_by_modelid(char *model_id)
{
        unsigned int i;

        for (i = 0; i < ARRAY_SIZE(beada_model_table); i++) {
		if (strcmp(model_id, beada_model_table[i].name) == 0)
			return &beada_model_table[i];
        }

        return NULL;
}

/**
 * beada_lookup_model_by_res() - Find panel geometry by pixel resolution
 * @width:  Horizontal pixel count
 * @height: Vertical pixel count
 *
 * Fallback when Status-Link is unavailable and the resolution is parsed from
 * the USB iProduct string ("BeadaPanel WxH").  Multiple models can share a
 * resolution; the first matching entry is returned (all such entries have the
 * same physical dimensions, so the returned geometry is accurate).
 *
 * Returns the matching entry, or NULL if no entry matches.
 */
const struct beada_model_entry *beada_lookup_model_by_res(u16 width, u16 height)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(beada_model_table); i++) {
		if (beada_model_table[i].width  == width &&
		    beada_model_table[i].height == height)
			return &beada_model_table[i];
	}

	return NULL;
}
