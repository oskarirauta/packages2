// SPDX-License-Identifier: GPL-2.0-only
/*
 * ax206-drm-plane.c - AX206 DRM simple display pipe callbacks
 *
 * Shadow buffer and coalescing blit engine:
 *
 *   pipe_update writes changed pixels into the shadow buffer (fast RAM
 *   operation) and expands a persistent dirty rectangle.  A single work
 *   item (ax206_drm_blit_work_fn) serialises all USB transfers through the
 *   MFD parent workqueue.
 *
 *   The work function sleeps AX206_BLIT_COALESCE_US before each transfer,
 *   giving time for additional damage rects to accumulate.  After the
 *   USB send it loops if new dirty data arrived during the transfer,
 *   ensuring the display always catches up without ever queuing more
 *   than one work item.
 *
 * Copyright (C) 2025 Oskari Rauta <oskari.rauta@gmail.com>
 */

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/workqueue.h>
#include <linux/spinlock.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <linux/swab.h>

#include <drm/drm_atomic.h>
#include <drm/drm_atomic_helper.h>
#include <drm/drm_damage_helper.h>
#include <drm/drm_format_helper.h>
#include <drm/drm_fourcc.h>
#include <drm/drm_framebuffer.h>
#include <drm/drm_gem_atomic_helper.h>
#include <drm/drm_gem_shmem_helper.h>
#include <drm/drm_rect.h>
#include <drm/drm_simple_kms_helper.h>

#include "ax206.h"
#include "ax206-drm-priv.h"

/* ----- Blit work: single persistent work item ----- */

/*
 * ax206_drm_blit_work_fn - coalescing USB blit worker
 *
 * Runs on the MFD parent workqueue.  Each iteration:
 *   1. Sleeps AX206_BLIT_COALESCE_US to let more damage accumulate.
 *   2. Snapshots and clears the dirty rectangle under blit_lock.
 *   3. Extracts the rect from the shadow buffer and sends via USB.
 *   4. Loops if new dirty data arrived while the transfer was running.
 */
void ax206_drm_blit_work_fn(struct work_struct *work)
{
	struct ax206_drm_dev *d =
		container_of(work, struct ax206_drm_dev, blit_work);
	u16 x0, y0, x1, y1;
	u8 *rect_buf;
	u16 rw, rh;
	size_t rect_size;
	int ret;

	do {
		/* Coalescing delay: let rapid commits merge into one blit */
		usleep_range(AX206_BLIT_COALESCE_US,
			     AX206_BLIT_COALESCE_US + 1000);

		/* Snapshot dirty rect and clear it */
		spin_lock(&d->blit_lock);

		if (!d->blit_dirty) {
			d->blit_active = false;
			spin_unlock(&d->blit_lock);
			return;
		}

		d->blit_dirty = false;
		x0 = d->dirty_x0;
		y0 = d->dirty_y0;
		x1 = d->dirty_x1;
		y1 = d->dirty_y1;
		/* Reset to "empty" so next flush starts a fresh rect */
		d->dirty_x0 = d->dpf->width;
		d->dirty_y0 = d->dpf->height;
		d->dirty_x1 = 0;
		d->dirty_y1 = 0;

		spin_unlock(&d->blit_lock);

		/* Skip USB transfer if blanked or disconnected */
		if (d->idle_active || !d->dpf->connected) {
			ax206_stats_frame_skipped(&d->stats);
			continue;
		}

		/* Sanity-check the rect */
		if (x0 >= x1 || y0 >= y1 ||
		    x1 > d->dpf->width || y1 > d->dpf->height) {
			ax206_stats_frame_skipped(&d->stats);
			continue;
		}

		rw = x1 - x0;
		rh = y1 - y0;
		rect_size = (size_t)rw * rh * AX206_BPP;

		/* Copy dirty rect out of shadow buffer */
		rect_buf = kmalloc(rect_size, GFP_KERNEL);
		if (!rect_buf) {
			ax206_stats_frame_error(&d->stats);
			continue;
		}

		{
			u16 row;
			for (row = 0; row < rh; row++) {
				const u8 *src = d->shadow_buf +
					((size_t)(y0 + row) * d->dpf->width
					 + x0) * AX206_BPP;
				memcpy(rect_buf + (size_t)row * rw * AX206_BPP,
				       src, (size_t)rw * AX206_BPP);
			}
		}

		ax206_stats_frame_received(&d->stats);

		ret = ax206_usb_blit(d->dpf, rect_buf, rect_size,
				   x0, y0, x1, y1);
		kfree(rect_buf);

		if (ret)
			ax206_stats_frame_error(&d->stats);
		else
			ax206_stats_frame_drawn(&d->stats);

		/* Loop: process any damage that arrived during the transfer */
	} while (true);
}

/*
 * ax206_drm_flush() - update shadow buffer and schedule a blit
 *
 * Supports DRM_FORMAT_RGB565 and DRM_FORMAT_XRGB8888 input.
 * All pixels are stored in the shadow buffer as big-endian RGB565
 * (swab16 applied) which is the format the AX206 device expects.
 */
static void ax206_drm_flush(struct ax206_drm_dev *d,
			  const void *src,
			  struct drm_framebuffer *fb,
			  const struct drm_rect *clip)
{
	const bool is_xrgb = (fb->format->format == DRM_FORMAT_XRGB8888);
	u16 px0, py0, px1, py1;
	u16 lx, ly;
	int px, py;
	bool changed = false;
	unsigned long flags;

	px0 = d->dpf->width;
	py0 = d->dpf->height;
	px1 = 0;
	py1 = 0;

	for (ly = (u16)clip->y1; ly < (u16)clip->y2; ly++) {
		for (lx = (u16)clip->x1; lx < (u16)clip->x2; lx++) {
			u16 pixel;
			size_t shadow_off;
			u16 *shadow_px;

			if (is_xrgb) {
				/*
				 * XRGB8888: 4 bytes per pixel, convert to RGB565.
				 * Layout in memory (LE): B G R X  (byte order).
				 * As u32: 0x00RRGGBB.
				 */
				const u32 *src32 = (const u32 *)src;
				u32 argb = src32[ly * (fb->pitches[0] / 4) + lx];
				u8 r = (argb >> 16) & 0xff;
				u8 g = (argb >>  8) & 0xff;
				u8 b = (argb      ) & 0xff;
				u16 v = ((u16)(r >> 3) << 11)
				      | ((u16)(g >> 2) <<  5)
				      |  (u16)(b >> 3);
				pixel = swab16(v);
			} else {
				/*
				 * RGB565 little-endian: byte-swap for AX206
				 * big-endian expectation.
				 */
				const u16 *src16 = (const u16 *)src;
				pixel = swab16(
					src16[ly * (fb->pitches[0] / 2) + lx]);
			}

			/* Rotate logical -> physical coordinates */
			ax206_rotate_pixel(d, lx, ly, &px, &py);

			if (px < 0 || px >= (int)d->dpf->width ||
			    py < 0 || py >= (int)d->dpf->height)
				continue;

			shadow_off = (size_t)py * d->dpf->width + px;
			shadow_px  = (u16 *)(d->shadow_buf +
					     shadow_off * AX206_BPP);

			if (*shadow_px == pixel)
				continue;

			*shadow_px = pixel;

			if ((u16)px < px0) px0 = (u16)px;
			if ((u16)px > px1) px1 = (u16)px;
			if ((u16)py < py0) py0 = (u16)py;
			if ((u16)py > py1) py1 = (u16)py;
			changed = true;
		}
	}

	if (!changed)
		return;

	spin_lock_irqsave(&d->blit_lock, flags);

	if (d->blit_dirty) {
		if (px0 < d->dirty_x0) d->dirty_x0 = px0;
		if (py0 < d->dirty_y0) d->dirty_y0 = py0;
		if ((u16)(px1 + 1) > d->dirty_x1) d->dirty_x1 = px1 + 1;
		if ((u16)(py1 + 1) > d->dirty_y1) d->dirty_y1 = py1 + 1;
	} else {
		d->dirty_x0 = px0;
		d->dirty_y0 = py0;
		d->dirty_x1 = px1 + 1;
		d->dirty_y1 = py1 + 1;
	}

	d->blit_dirty = true;

	if (!d->blit_active) {
		d->blit_active = true;
		queue_work(d->dpf->wq, &d->blit_work);
	}

	spin_unlock_irqrestore(&d->blit_lock, flags);
}

/* ----- Simple pipe callbacks ----- */

void ax206_drm_pipe_enable(struct drm_simple_display_pipe *pipe,
			 struct drm_crtc_state *crtc_state,
			 struct drm_plane_state *plane_state)
{
	struct ax206_drm_dev *d = to_ax206_drm(pipe->crtc.dev);

	if (d->idle_active)
		ax206_idle_leave(d);

	ax206_idle_timer_reset(d);
}

void ax206_drm_pipe_disable(struct drm_simple_display_pipe *pipe)
{
	struct ax206_drm_dev *d = to_ax206_drm(pipe->crtc.dev);
	unsigned long flags;

	timer_delete_sync(&d->idle_timer);
	cancel_work_sync(&d->idle_work);

	if (d->noblank || !d->dpf->connected)
		return;

	/*
	 * noblank is off: fill display black.  We do this by zeroing the
	 * shadow buffer and marking the entire screen dirty so the blit
	 * worker sends the black fill.
	 */
	memset(d->shadow_buf, 0, d->shadow_size);

	spin_lock_irqsave(&d->blit_lock, flags);
	d->dirty_x0 = 0;
	d->dirty_y0 = 0;
	d->dirty_x1 = d->dpf->width;
	d->dirty_y1 = d->dpf->height;
	d->blit_dirty  = true;
	d->idle_active = false;
	if (!d->blit_active) {
		d->blit_active = true;
		queue_work(d->dpf->wq, &d->blit_work);
	}
	spin_unlock_irqrestore(&d->blit_lock, flags);
}

void ax206_drm_pipe_update(struct drm_simple_display_pipe *pipe,
			 struct drm_plane_state *old_state)
{
	struct ax206_drm_dev *d = to_ax206_drm(pipe->crtc.dev);
	struct drm_plane_state *new_state = pipe->plane.state;
	struct drm_shadow_plane_state *shadow_state;
	struct drm_atomic_helper_damage_iter iter;
	struct drm_rect damage;
	const void *src;
	struct drm_rect full_clip;

	if (!pipe->crtc.state->active || !new_state->fb)
		return;

	ax206_idle_timer_reset(d);

	shadow_state = to_drm_shadow_plane_state(new_state);
	src = shadow_state->data[0].vaddr;
	if (!src) {
		ax206_stats_frame_error(&d->stats);
		return;
	}
	if (!old_state->fb ||
	    old_state->fb->width  != new_state->fb->width ||
	    old_state->fb->height != new_state->fb->height) {
		full_clip.x1 = 0;
		full_clip.y1 = 0;
		full_clip.x2 = d->lwidth;
		full_clip.y2 = d->lheight;
		ax206_drm_flush(d, src, new_state->fb, &full_clip);
		return;
	}

	drm_atomic_helper_damage_iter_init(&iter, old_state, new_state);
	drm_atomic_for_each_plane_damage(&iter, &damage)
		ax206_drm_flush(d, src, new_state->fb, &damage);
}
