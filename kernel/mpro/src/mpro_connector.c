#include <drm/drm_atomic_helper.h>
#include <drm/drm_connector.h>
#include <drm/drm_edid.h>
#include <drm/drm_probe_helper.h>
#include "mpro.h"

/* ------------------------------------------------------------------ */
/* mpro connector						      */

/*
 * We use fake EDID info so that userspace know that it is dealing with
 * an Acer projector, rather then listing this as an "unknown" monitor.
 * Note this assumes this driver is only ever used with the Acer C120, if we
 * add support for other devices the vendor and model should be parameterized.
 */
static struct edid mpro_edid = {
	.header		= { 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00 },
	.mfg_id		= { 0x59, 0xe3 },	/* "VOC" */
	.prod_code	= { 0x04, 0x10 },	/* 1004 */
	.serial		= 0xdeadbeef,
	.mfg_week	= 16,
	.mfg_year	= 24,
	.version	= 1,			/* EDID 1.3 */
	.revision	= 3,			/* EDID 1.3 */
	.input		= 0x80,			/* Digital input */
	.features	= 0x02,			/* Pref timing in DTD 1 */
	.standard_timings = { { 1, 1 }, { 1, 1 }, { 1, 1 }, { 1, 1 },
			      { 1, 1 }, { 1, 1 }, { 1, 1 }, { 1, 1 } },
	.detailed_timings = { {
		.pixel_clock = 3383,
		/* hactive = 848, hblank = 256 */
		.data.pixel_data.hactive_lo = 0x50,
		.data.pixel_data.hblank_lo = 0x00,
		.data.pixel_data.hactive_hblank_hi = 0x31,
		/* vactive = 480, vblank = 28 */
		.data.pixel_data.vactive_lo = 0xe0,
		.data.pixel_data.vblank_lo = 0x1c,
		.data.pixel_data.vactive_vblank_hi = 0x10,
		/* hsync offset 40 pw 128, vsync offset 1 pw 4 */
		.data.pixel_data.hsync_offset_lo = 0x28,
		.data.pixel_data.hsync_pulse_width_lo = 0x80,
		.data.pixel_data.vsync_offset_pulse_width_lo = 0x14,
		.data.pixel_data.hsync_vsync_offset_pulse_width_hi = 0x00,
		/* Digital separate syncs, hsync+, vsync+ */
		.data.pixel_data.misc = 0x1e,
	}, {
		.pixel_clock = 0,
		.data.other_data.type = 0xfd, /* Monitor ranges */
		.data.other_data.data.range.min_vfreq = 59,
		.data.other_data.data.range.max_vfreq = 61,
		.data.other_data.data.range.min_hfreq_khz = 29,
		.data.other_data.data.range.max_hfreq_khz = 32,
		.data.other_data.data.range.pixel_clock_mhz = 4, /* 40 MHz */
		.data.other_data.data.range.flags = 0,
		.data.other_data.data.range.formula.cvt = {
			0xa0, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20 },
	}, {
		.pixel_clock = 0,
		.data.other_data.type = 0xfc, /* Model string */
		.data.other_data.data.str.str = {
			'M', 'P', 'R', 'O', '\n', ' ', ' ', ' ', ' ', ' ',
			' ', ' ', ' ' },
	}, {
		.pixel_clock = 0,
		.data.other_data.type = 0xff, /* Serial Number */
		.data.other_data.data.str.str = {
			'\n', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
			' ', ' ',  ' ' },
	} },
	.checksum = 0x00,
};

static int mpro_conn_get_modes(struct drm_connector *connector) {

	drm_connector_update_edid_property(connector, &mpro_edid);
	return drm_add_edid_modes(connector, &mpro_edid);
}

static const struct drm_connector_helper_funcs mpro_conn_helper_funcs = {
	.get_modes = mpro_conn_get_modes,
};

static const struct drm_connector_funcs mpro_conn_funcs = {
	.fill_modes = drm_helper_probe_single_connector_modes,
	.destroy = drm_connector_cleanup,
	.reset = drm_atomic_helper_connector_reset,
	.atomic_duplicate_state = drm_atomic_helper_connector_duplicate_state,
	.atomic_destroy_state = drm_atomic_helper_connector_destroy_state,
};

int mpro_conn_init(struct mpro_device *mpro) {

	drm_connector_helper_add(&mpro -> conn, &mpro_conn_helper_funcs);
/*
	struct drm_property *prop = drm_property_create_bool(mpro -> conn.dev, DRM_MODE_PROP_IMMUTABLE, "partial_updates");
	if ( prop )
		drm_object_attach_property(&mpro -> conn.base, prop, mpro -> partial);
*/
	return drm_connector_init(&mpro -> dev, &mpro -> conn,
				  &mpro_conn_funcs, DRM_MODE_CONNECTOR_USB);
}

static int mpro_edid_block_checksum(u8 *raw_edid) {

	int i;
	u8 csum = 0;

	for ( i = 0; i < EDID_LENGTH - 1; i++ )
		csum += raw_edid[i];

	return 0x100 - csum;
}

void mpro_edid_setup(struct mpro_device *mpro) {

	unsigned int width, height, width_mm, height_mm;
	char buf[16];

	width = mpro -> width;
	height = mpro -> height;
	width_mm = mpro -> width_mm;
	height_mm = mpro -> height_mm;

	mpro_edid.detailed_timings[0].data.pixel_data.hactive_lo = width % 256;
	mpro_edid.detailed_timings[0].data.pixel_data.hactive_hblank_hi &= 0x0f;
	mpro_edid.detailed_timings[0].data.pixel_data.hactive_hblank_hi |= \
						((u8)(width / 256) << 4);

	mpro_edid.detailed_timings[0].data.pixel_data.vactive_lo = height % 256;
	mpro_edid.detailed_timings[0].data.pixel_data.vactive_vblank_hi &= 0x0f;
	mpro_edid.detailed_timings[0].data.pixel_data.vactive_vblank_hi |= \
						((u8)(height / 256) << 4);

	mpro_edid.detailed_timings[0].data.pixel_data.width_mm_lo = \
							width_mm % 256;
	mpro_edid.detailed_timings[0].data.pixel_data.height_mm_lo = \
							height_mm % 256;
	mpro_edid.detailed_timings[0].data.pixel_data.width_height_mm_hi = \
					((u8)(width_mm / 256) << 4) | \
					((u8)(height_mm / 256) & 0xf);

	memcpy(&mpro_edid.detailed_timings[2].data.other_data.data.str.str,
		mpro -> model, strlen(mpro -> model));

	snprintf(buf, 16, "%02X%02X%02X%02X\n",
		mpro -> id[4], mpro -> id[5], mpro -> id[6], mpro -> id[7]);

	memcpy(&mpro_edid.detailed_timings[3].data.other_data.data.str.str,
		buf, strlen(buf));

	mpro_edid.checksum = mpro_edid_block_checksum((u8*)&mpro_edid);
}
