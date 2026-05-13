// SPDX-License-Identifier: GPL-2.0
/*
 * mpro_drm_edid.c — minimal EDID 1.3 emulation.
 *
 * Builds a 128-byte EDID block exposing the device name, physical
 * dimensions and preferred timing. This gives userspace tools (xrandr,
 * edid-decode, compositors) an identifiable display without a real
 * EDID EEPROM on the device.
 */

#include <linux/module.h>
#include <linux/string.h>
#include <linux/rtc.h>
#include <linux/ktime.h>

#include <drm/drm_edid.h>
#include <drm/drm_connector.h>
#include <drm/drm_print.h>

#include "mpro_drm.h"

int mpro_drm__create_edid(struct mpro_drm *mdrm)
{
	static const char monitor_name[] = "VoCore MPRO";
	const u32 w  = mdrm->mpro->model->width;
	const u32 h  = mdrm->mpro->model->height;
	const u32 wm = mdrm->mpro->model->width_mm;
	const u32 hm = mdrm->mpro->model->height_mm;
	const u32 clock_10khz  = MPRO_TIMING_CLOCK_KHZ / 10;	/* 29.73 MHz */
	const u32 hblank = MPRO_TIMING_HSYNC_OFF +
			   MPRO_TIMING_HSYNC_PULSE +
			   MPRO_TIMING_HBACK;		/* 40 + 48 + 40 */
	const u32 vblank = MPRO_TIMING_VSYNC_OFF +
			   MPRO_TIMING_VSYNC_PULSE +
			   MPRO_TIMING_VBACK;		/* 13 + 3 + 29 */
	const u32 hsync_off   = MPRO_TIMING_HSYNC_OFF;
	const u32 hsync_pulse = MPRO_TIMING_HSYNC_PULSE;
	const u32 vsync_off   = MPRO_TIMING_VSYNC_OFF;
	const u32 vsync_pulse = MPRO_TIMING_VSYNC_PULSE;
	struct rtc_time tm;
	time64_t now;
	u8 edid[128];
	u8 *p;
	u16 mfg;
	size_t nlen;
	int i, sum = 0, ret;

	memset(edid, 0, sizeof(edid));

	/* 0x00-0x07: Header */
	edid[0] = 0x00;
	for (i = 1; i < 7; i++)
		edid[i] = 0xff;
	edid[7] = 0x00;

	/* 0x08-0x09: Manufacturer "VOC" big-endian, 5 bits per letter, A=1 */
	mfg = (('V' - '@') << 10) | (('O' - '@') << 5) | ('C' - '@');
	edid[8] = (mfg >> 8) & 0xff;
	edid[9] =  mfg       & 0xff;

	/* 0x0a-0x0b: Product code (LE) */
	edid[10] =  mdrm->mpro->screen       & 0xff;
	edid[11] = (mdrm->mpro->screen >> 8) & 0xff;

	/* 0x0c-0x0f: Serial (LE) */
	edid[12] =  mdrm->mpro->screen        & 0xff;
	edid[13] = (mdrm->mpro->screen >>  8) & 0xff;
	edid[14] = (mdrm->mpro->screen >> 16) & 0xff;
	edid[15] = (mdrm->mpro->screen >> 24) & 0xff;

	now = ktime_get_real_seconds();
	rtc_time64_to_tm(now, &tm);

	edid[16] = 1;				/* week of manufacture */
	edid[17] = (tm.tm_year + 1900) - 1990;	/* year - 1990 */
	edid[18] = 1;				/* EDID 1.3 */
	edid[19] = 3;
	edid[20] = 0x80;			/* digital input */
	edid[21] = (wm + 9) / 10;		/* horizontal size, cm */
	edid[22] = (hm + 9) / 10;		/* vertical size, cm */
	edid[23] = 120;				/* gamma 2.20 */
	edid[24] = 0x0a;			/* digital, RGB, preferred timing */

	/* 0x19-0x22: chromaticity (sRGB-style defaults) */
	edid[25] = 0xee;
	edid[26] = 0x91;
	edid[27] = 0xa3;
	edid[28] = 0x54;
	edid[29] = 0x4c;
	edid[30] = 0x99;
	edid[31] = 0x26;
	edid[32] = 0x0f;
	edid[33] = 0x50;
	edid[34] = 0x54;

	/* 0x26-0x35: standard timings — 0x01 0x01 means "unused" */
	for (i = 0x26; i < 0x36; i++)
		edid[i] = 0x01;

	/* 0x36-0x47: Detailed timing #0 (preferred) */
	p = &edid[0x36];
	p[0]  =  clock_10khz       & 0xff;
	p[1]  = (clock_10khz >> 8) & 0xff;
	p[2]  =  w                 & 0xff;
	p[3]  =  hblank            & 0xff;
	p[4]  = (((w      >> 8) & 0xf) << 4) | ((hblank >> 8) & 0xf);
	p[5]  =  h                 & 0xff;
	p[6]  =  vblank            & 0xff;
	p[7]  = (((h      >> 8) & 0xf) << 4) | ((vblank >> 8) & 0xf);
	p[8]  =  hsync_off         & 0xff;
	p[9]  =  hsync_pulse       & 0xff;
	p[10] = ((vsync_off  & 0xf) << 4) | (vsync_pulse & 0xf);
	p[11] = (((hsync_off   >> 8) & 0x3) << 6) |
		(((hsync_pulse >> 8) & 0x3) << 4) |
		(((vsync_off   >> 4) & 0x3) << 2) |
		 ((vsync_pulse >> 4) & 0x3);
	p[12] =  wm                & 0xff;
	p[13] =  hm                & 0xff;
	p[14] = (((wm >> 8) & 0xf) << 4) | ((hm >> 8) & 0xf);
	p[15] = 0;
	p[16] = 0;
	p[17] = 0x18;	/* digital separate sync, +/+ */

	/* 0x48-0x59: Descriptor #1 — monitor name */
	p = &edid[0x48];
	p[0] = 0;
	p[1] = 0;
	p[2] = 0;
	p[3] = 0xfc;
	p[4] = 0;
	nlen = sizeof(monitor_name) - 1;
	memcpy(&p[5], monitor_name, nlen);
	p[5 + nlen] = 0x0a;
	for (i = 5 + nlen + 1; i < 18; i++)
		p[i] = 0x20;

	/* 0x5a-0x6b: Descriptor #2 — range limits */
	p = &edid[0x5a];
	p[0]  = 0;
	p[1]  = 0;
	p[2]  = 0;
	p[3]  = 0xfd;
	p[4]  = 0;
	p[5]  = 50;			/* min vertical rate (Hz) */
	p[6]  = 75;			/* max vertical rate (Hz) */
	p[7]  = 30;			/* min horizontal rate (kHz) */
	p[8]  = 80;			/* max horizontal rate (kHz) */
	p[9]  = (clock_10khz / 100) + 1;/* max pixel clock (10 MHz units) */
	p[10] = 0;
	p[11] = 0x0a;
	for (i = 12; i < 18; i++)
		p[i] = 0x20;

	/* 0x6c-0x7d: Descriptor #3 — dummy */
	p = &edid[0x6c];
	p[0] = 0;
	p[1] = 0;
	p[2] = 0;
	p[3] = 0x10;
	p[4] = 0;
	for (i = 5; i < 18; i++)
		p[i] = 0;

	edid[126] = 0;

	/* Checksum: the 128 bytes must sum to 0 mod 256 */
	for (i = 0; i < 127; i++)
		sum += edid[i];
	edid[127] = (256 - (sum & 0xff)) & 0xff;

	ret = drm_connector_update_edid_property(&mdrm->connector,
						 (struct edid *)edid);
	if (ret)
		drm_warn(&mdrm->drm, "EDID update failed: %d\n", ret);
	return ret;
}
