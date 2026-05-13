// SPDX-License-Identifier: GPL-2.0-only
/*
 * beada-drm-plane.c – Pipe callbacks and the blit pipeline
 *
 * Owns the three drm_simple_display_pipe callbacks plus the table that
 * wires them together (referenced from beada-drm-kms.c).  The blit core
 * (beada_blit_rect) converts framebuffer pixels into the shadow buffer
 * applying gamma and rotation in one pass.
 *
 * Pipeline summary
 * ────────────────
 *   pipe_enable :
 *       beada_pipe_active() — pin USB autosuspend off
 *       drm_crtc_vblank_on() — seed vblank timestamps
 *       beada_finish_pageflip() — no prior tick exists yet
 *
 *   pipe_update :
 *       flush previous frame's USB transfer (rate limit + buffer reuse)
 *       mirror crtc_state->gamma_lut into bdev->lut if changed
 *       blit the new framebuffer into the shadow
 *       beada_send_frame() — async USB transfer
 *       beada_arm_pageflip() — event delivered on next vblank tick
 *
 *   pipe_disable :
 *       beada_finish_pageflip() — drain any pending event
 *       beada_flush_frame_transfer() — wait for in-flight transfer
 *       drm_crtc_vblank_off() — stops the hrtimer via our callback
 *       if !noblank, beada_send_frame(blank_buf) — clears display
 *       mark shadow invalid
 *       beada_pipe_idle() — release USB autosuspend lock
 *
 * Damage clips
 * ────────────
 * The compositor reports dirty regions, but the device firmware locks
 * the frame format on its first START tag, so we cannot send partial
 * frames.  Damage clips are therefore ignored — we always blit the
 * source framebuffer's full rectangle and send the entire shadow.
 *
 * Resume restoration
 * ──────────────────
 * pipe_enable does NOT re-send the shadow on system-suspend resume.
 * The MFD parent's USB resume callback runs first and dispatches our
 * registered resume handler (beada-drm-pm.c), which re-sends the
 * shadow buffer to hardware before userspace is unfrozen.  By the
 * time pipe_enable would naturally be called again — if ever — the
 * image is already back.
 *
 * Coordinate model
 * ────────────────
 *   Source  (sx, sy)  – framebuffer pixel, in compositor's logical
 *                       coordinates (bdev->width × bdev->height).
 *   Output  (ox, oy)  – shadow buffer pixel, in NATIVE panel coordinates
 *                       (panel->width × panel->height).
 *
 *   rotate_coord() maps source → output for the active rotation.
 */

#include <linux/string.h>
#include <linux/unaligned.h>

#include <drm/drm_atomic_helper.h>
#include <drm/drm_blend.h>
#include <drm/drm_drv.h>
#include <drm/drm_fourcc.h>
#include <drm/drm_framebuffer.h>
#include <drm/drm_gem_atomic_helper.h>
#include <drm/drm_gem_shmem_helper.h>
#include <drm/drm_simple_kms_helper.h>
#include <drm/drm_vblank.h>

#include "../beada.h"
#include "beada-drm.h"

/* ================================================================== */
/* Pixel helpers                                                        */
/* ================================================================== */

static inline u8 lut_or_linear(const u8 *channel_lut, bool valid, u8 val)
{
	return valid ? channel_lut[val] : val;
}

/**
 * pack_rgb565() - Pack 8-bit RGB into RGB565
 * Used for the spec-default "format=RGB16" caps string.
 */
static inline __le16 pack_rgb565(u8 r, u8 g, u8 b)
{
	return cpu_to_le16(((u16)(r >> 3) << 11) |
			   ((u16)(g >> 2) <<  5) |
			    (u16)(b >> 3));
}

/**
 * pack_bgr565() - Pack 8-bit RGB into BGR565
 * Used for the "format=BGR16" caps string (recognised BeadaPanel models).
 */
static inline __le16 pack_bgr565(u8 r, u8 g, u8 b)
{
	return cpu_to_le16(((u16)(b >> 3) << 11) |
			   ((u16)(g >> 2) <<  5) |
			    (u16)(r >> 3));
}

/* ================================================================== */
/* Coordinate rotation                                                  */
/* ================================================================== */

/**
 * rotate_coord() - Map a source pixel to its output (shadow) coordinate
 *
 * @rot       Current rotation bits (DRM_MODE_ROTATE_* | DRM_MODE_REFLECT_X)
 * @sx, @sy   Source pixel inside the logical framebuffer
 * @src_w     Logical framebuffer width  (= bdev->width)
 * @src_h     Logical framebuffer height (= bdev->height)
 * @out_w     Native panel width  (= bdev->panel->width)
 * @out_h     Native panel height (= bdev->panel->height)
 * @ox, @oy   Output pixel in shadow buffer coordinates (returned)
 *
 * The four rotation cases and the orthogonal REFLECT_X flag give all
 * eight orientations supported via sysfs.  REFLECT_Y is normalised into
 * ROTATE_180 + REFLECT_X by the sysfs setter, so we don't handle it
 * here.
 *
 * Coordinates are clamped before being written, so a malformed call
 * never escapes the shadow buffer.
 */
static inline void rotate_coord(u16 rot,
				int sx, int sy, int src_w, int src_h,
				int out_w, int out_h,
				int *ox, int *oy)
{
	int tx, ty;

	switch (rot & DRM_MODE_ROTATE_MASK) {
	case DRM_MODE_ROTATE_90:
		tx = src_h - 1 - sy;
		ty = sx;
		break;
	case DRM_MODE_ROTATE_180:
		tx = src_w - 1 - sx;
		ty = src_h - 1 - sy;
		break;
	case DRM_MODE_ROTATE_270:
		tx = sy;
		ty = src_w - 1 - sx;
		break;
	case DRM_MODE_ROTATE_0:
	default:
		tx = sx;
		ty = sy;
		break;
	}

	if (rot & DRM_MODE_REFLECT_X)
		tx = out_w - 1 - tx;

	*ox = clamp(tx, 0, out_w - 1);
	*oy = clamp(ty, 0, out_h - 1);
}

/* ================================================================== */
/* Blit core                                                            */
/* ================================================================== */

/**
 * beada_blit_rect() - Convert a source rectangle into the shadow buffer
 *
 * Converts each source pixel to RGB565/BGR565 (with optional gamma),
 * rotates it through rotate_coord(), and writes it into the shadow.
 *
 * Caller responsibilities:
 *   - hold bdev->shadow_lock
 *   - guarantee that @clip lies inside the source framebuffer
 *
 * @bdev    Device handle
 * @fb      Source DRM framebuffer
 * @data    iosys_map of the framebuffer's CPU mapping
 * @clip    Source rectangle to blit
 * @rot     Active rotation bits
 * @gamma_p Either a pointer to the per-channel LUT array, or NULL when
 *          gamma is inactive (caller passes a snapshot, see pipe_update)
 * @bgr     true → pack_bgr565, false → pack_rgb565
 */
static void beada_blit_rect(struct beada_drm_dev *bdev,
			    const struct drm_framebuffer *fb,
			    const struct iosys_map *data,
			    const struct drm_rect *clip,
			    u16 rot,
			    const u8 (*gamma_p)[256],
			    bool bgr)
{
	__le16 *shadow = bdev->shadow_buf;
	const int out_w = bdev->panel->width;
	const int out_h = bdev->panel->height;
	const int src_w = bdev->width;
	const int src_h = bdev->height;
	const bool is_xrgb = (fb->format->format == DRM_FORMAT_XRGB8888);
	const unsigned int bpp = is_xrgb ? 4 : 2;
	const u8 *src_base = data->vaddr;
	const bool gamma_on = (gamma_p != NULL);
	int sx, sy;

	for (sy = clip->y1; sy < clip->y2; sy++) {
		const u8 *src_row = src_base + sy * fb->pitches[0]
				    + clip->x1 * bpp;

		for (sx = clip->x1; sx < clip->x2; sx++, src_row += bpp) {
			u8 r, g, b;
			__le16 px;
			int ox, oy;

			if (is_xrgb) {
				u32 argb = get_unaligned_le32(src_row);

				r = (argb >> 16) & 0xff;
				g = (argb >>  8) & 0xff;
				b =  argb        & 0xff;
			} else {
				u16 raw = get_unaligned_le16(src_row);
				u8 r5 = (raw >> 11) & 0x1f;
				u8 g6 = (raw >>  5) & 0x3f;
				u8 b5 =  raw        & 0x1f;

				/* Expand RGB565 to 8-bit before gamma lookup */
				r = (r5 << 3) | (r5 >> 2);
				g = (g6 << 2) | (g6 >> 4);
				b = (b5 << 3) | (b5 >> 2);
			}

			if (gamma_on) {
				r = lut_or_linear((*gamma_p), true, r);
				g = lut_or_linear((*(gamma_p + 1)), true, g);
				b = lut_or_linear((*(gamma_p + 2)), true, b);
			}

			px = bgr ? pack_bgr565(r, g, b)
				 : pack_rgb565(r, g, b);

			rotate_coord(rot, sx, sy, src_w, src_h,
				     out_w, out_h, &ox, &oy);
			shadow[oy * out_w + ox] = px;
		}
	}
}

/* ================================================================== */
/* Pipe callbacks                                                       */
/* ================================================================== */

void beada_pipe_enable(struct drm_simple_display_pipe *pipe,
		       struct drm_crtc_state *crtc_state,
		       struct drm_plane_state *plane_state)
{
	struct beada_drm_dev *bdev = pipe_to_beada_drm(pipe);
	int ret;

	/*
	 * Pin the USB autosuspend timer until pipe_disable.  This keeps
	 * the device powered even when userspace (Weston, etc.) is
	 * sitting on a dim screen with no frame commits.
	 *
	 * Failure here is rare but possible during disconnect races.
	 * We log it and continue; pipe_disable will still call
	 * beada_pipe_idle() unconditionally, which will produce a
	 * harmless WARN about refcount imbalance — a visible signal
	 * that something went wrong.
	 */
	ret = beada_pipe_active(bdev->beada);
	if (ret)
		dev_warn(bdev->drm.dev,
			 "pipe_active failed: %d; autopm may be imbalanced\n",
			 ret);

	/*
	 * Seed vblank timestamps before any frame I/O so the first
	 * armed flip event (from pipe_update) gets a fresh timestamp.
	 */
	drm_crtc_vblank_on(&bdev->pipe.crtc);

	/*
	 * No prior vblank tick exists yet, so we cannot arm a flip
	 * event for "the next tick" — send any pending event
	 * immediately.  Subsequent updates go through
	 * beada_arm_pageflip().
	 */
	beada_finish_pageflip(pipe);
}

void beada_pipe_disable(struct drm_simple_display_pipe *pipe)
{
	struct beada_drm_dev *bdev = pipe_to_beada_drm(pipe);
	int ret;

	beada_finish_pageflip(pipe);

	/* Drain any in-flight transfer before we change what shadow_buf
	 * contains — the worker is still reading it. */
	beada_flush_frame_transfer(bdev->beada);

	drm_crtc_vblank_off(&bdev->pipe.crtc);

	if (!bdev->noblank) {
		ret = beada_send_frame(bdev->beada,
				       bdev->blank_buf,
				       bdev->shadow_size,
				       bdev->panel->width,
				       bdev->panel->height);
		if (ret)
			dev_warn(bdev->drm.dev,
				 "blank frame on disable failed: %d\n", ret);

		/* Wait for the blank frame to actually leave so the
		 * user sees a clean display before we return. */
		beada_flush_frame_transfer(bdev->beada);
	}

	mutex_lock(&bdev->shadow_lock);
	bdev->shadow_valid = false;
	mutex_unlock(&bdev->shadow_lock);

	beada_pipe_idle(bdev->beada);
}

void beada_pipe_update(struct drm_simple_display_pipe *pipe,
		       struct drm_plane_state *old_state)
{
	struct beada_drm_dev *bdev = pipe_to_beada_drm(pipe);
	struct drm_plane_state *new_state = pipe->plane.state;
	struct drm_shadow_plane_state *shadow_state;
	struct drm_framebuffer *fb;
	struct drm_crtc_state *crtc_state = pipe->crtc.state;
	struct drm_rect clip;
	u8 gamma_snapshot[3][256];
	bool gamma_valid;
	int idx;

	if (!new_state || !new_state->fb || !crtc_state->active)
		return;

	if (!drm_dev_enter(&bdev->drm, &idx))
		return;

	/*
	 * Wait for the previous frame's USB transfer to finish.  This
	 * does two things at once:
	 *   1. Ensures the shadow buffer we're about to overwrite isn't
	 *      still being read by the USB worker.
	 *   2. Provides natural rate limiting: pipe_update can't run
	 *      faster than the USB can sustain, but it CAN run as fast
	 *      as the bus allows (~45 fps for 800x480, ~110 fps for
	 *      480x320).
	 */
	beada_flush_frame_transfer(bdev->beada);

	beada_apply_color_mgmt(bdev, crtc_state);

	fb = new_state->fb;
	shadow_state = to_drm_shadow_plane_state(new_state);
	if (!shadow_state || iosys_map_is_null(&shadow_state->data[0]))
		goto out_event;

	mutex_lock(&bdev->lut_lock);
	gamma_valid = bdev->gamma_valid;
	if (gamma_valid)
		memcpy(gamma_snapshot, bdev->lut, sizeof(gamma_snapshot));
	mutex_unlock(&bdev->lut_lock);

	clip.x1 = 0;
	clip.y1 = 0;
	clip.x2 = fb->width;
	clip.y2 = fb->height;

	mutex_lock(&bdev->shadow_lock);
	beada_blit_rect(bdev, fb, &shadow_state->data[0], &clip,
			bdev->rotation,
			gamma_valid ? gamma_snapshot : NULL,
			beada_uses_bgr_format(bdev->beada));
	bdev->shadow_valid = true;
	mutex_unlock(&bdev->shadow_lock);

	/*
	 * Submit to USB asynchronously.  The worker copies shadow_buf
	 * into the parent's DMA-coherent bounce buffer and runs the
	 * bulk transfer in the background; we return to the compositor
	 * immediately so it can start rendering the next frame.
	 */
	beada_send_frame(bdev->beada,
			 bdev->shadow_buf,
			 bdev->shadow_size,
			 bdev->panel->width,
			 bdev->panel->height);

out_event:
	/*
	 * Arm the flip event for the next vblank tick.  The USB
	 * transfer is still in progress at this point, but armed
	 * events fire on hrtimer ticks regardless — and by the time
	 * the next pipe_update arrives we'll flush again, so
	 * timestamps stay aligned with what the device actually shows
	 * (with at most one vblank's slack).
	 */
	beada_arm_pageflip(pipe);

	drm_dev_exit(idx);
}

/* ================================================================== */
/* Pipe funcs table                                                     */
/* ================================================================== */

const struct drm_simple_display_pipe_funcs beada_pipe_funcs = {
	.enable         = beada_pipe_enable,
	.disable        = beada_pipe_disable,
	.update         = beada_pipe_update,
	.enable_vblank  = beada_vblank_enable,
	.disable_vblank = beada_vblank_disable,
	DRM_GEM_SIMPLE_DISPLAY_PIPE_SHADOW_PLANE_FUNCS,
};
