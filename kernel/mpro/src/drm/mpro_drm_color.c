// SPDX-License-Identifier: GPL-2.0
/*
 * mpro_drm_color.c — color management & pixel-format conversion.
 *
 * Contents:
 *   - gamma LUT computation via mpro_pow_lut() from the parent module,
 *     wrapped by the local builder mpro_drm__build_power_lut()
 *   - combined gamma + brightness LUT (mpro_drm__rebuild_combined_lut)
 *   - DRM color_mgmt synchronisation (mpro_drm__apply_color_mgmt)
 *   - pixel-format conversion with LUT lookup (mpro_drm__copy_*)
 *   - 90/180/270-degree rotation + reflect, in a temp buffer
 *     (mpro_drm__rotate_buffer)
 *   - mpro_drm__copy_frame() — orchestrator that drives the above
 */

#include <linux/module.h>
#include <linux/slab.h>

#include <drm/drm_color_mgmt.h>
#include <drm/drm_format_helper.h>
#include <drm/drm_fourcc.h>
#include <drm/drm_atomic.h>
#include <drm/drm_print.h>

#include "mpro_drm.h"

/* ------------------------------------------------------------------ */
/* Gamma curve                                                        */
/* ------------------------------------------------------------------ */

/* Caller must hold mdrm->lut_lock */
void mpro_drm__rebuild_combined_lut(struct mpro_drm *mdrm)
{
	u32 b = mdrm->brightness;
	int c, i;

	for (c = 0; c < 3; c++) {
		for (i = 0; i < 256; i++) {
			u32 v = mdrm->gamma_valid ? mdrm->lut[c][i] : (u32)i;

			if (b < MPRO_BRIGHTNESS_MAX)
				v = v * b / MPRO_BRIGHTNESS_MAX;

			mdrm->lut_combined[c][i] = (u8)v;
		}
	}
}

void mpro_drm__build_power_lut(struct mpro_drm *mdrm, u32 g_x100)
{
	int i;

	mutex_lock(&mdrm->lut_lock);

	if (g_x100 == 100) {
		mdrm->gamma_valid = false;
	} else {

		for (i = 0; i < 256; i++) {
			u8 v = mpro_pow_lut(i, g_x100);

			mdrm->lut[0][i] = v;
			mdrm->lut[1][i] = v;
			mdrm->lut[2][i] = v;
		}

		mdrm->gamma_valid = true;
	}

	mpro_drm__rebuild_combined_lut(mdrm);
	mutex_unlock(&mdrm->lut_lock);
}

void mpro_drm__apply_color_mgmt(struct mpro_drm *mdrm,
				struct drm_crtc_state *crtc_state)
{
	struct drm_color_lut *lut;
	u32 i;

	if (!crtc_state->color_mgmt_changed)
		return;

	mutex_lock(&mdrm->lut_lock);

	if (!crtc_state->gamma_lut) {
		mdrm->gamma_valid = false;
	} else {

		lut = (struct drm_color_lut *)crtc_state->gamma_lut->data;

		for (i = 0; i < 256; i++) {
			mdrm->lut[0][i] = drm_color_lut_extract(lut[i].red,   8);
			mdrm->lut[1][i] = drm_color_lut_extract(lut[i].green, 8);
			mdrm->lut[2][i] = drm_color_lut_extract(lut[i].blue,  8);
		}

		mdrm->gamma_valid = true;
	}

	mpro_drm__rebuild_combined_lut(mdrm);
	mutex_unlock(&mdrm->lut_lock);
}

/* ------------------------------------------------------------------ */
/* Pixel format conversions                                           */
/* ------------------------------------------------------------------ */

static void mpro_drm__copy_rgb565(u8 *dst, const u8 *src,
				  u32 width, u32 height,
				  u32 dst_pitch, u32 src_pitch,
				  const u8 lut[3][256], bool transform_needed)
{
	int x, y;

	for (y = 0; y < height; y++) {
		const u16 *s = (const u16 *)(src + y * src_pitch);
		u16       *d = (u16 *)(dst + y * dst_pitch);

		if (!transform_needed) {
			memcpy(d, s, width * 2);
			continue;
		}

		for (x = 0; x < width; x++) {
			u16 px = s[x];
			u8  r  = lut[0][(px >> 11) & 0x1f];
			u8  g  = lut[1][(px >>  5) & 0x3f];
			u8  b  = lut[2][ px        & 0x1f];

			d[x] = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
		}
	}
}

static void mpro_drm__convert_xrgb8888_to_rgb565(u16 *dst, const u32 *src,
						 int width, int height,
						 int dst_pitch, int src_pitch,
						 const u8 lut[3][256],
						 bool transform_needed)
{
	int x, y;

	for (y = 0; y < height; y++) {
		const u32 *s = (const u32 *)((const u8 *)src + y * src_pitch);
		u16       *d = (u16 *)((u8 *)dst + y * dst_pitch);

		if (!transform_needed) {
			for (x = 0; x < width; x++) {
				u32 px = s[x];
				u8  r  = (px >> 16) & 0xff;
				u8  g  = (px >>  8) & 0xff;
				u8  b  =  px        & 0xff;

				d[x] = ((r >> 3) << 11) |
				       ((g >> 2) <<  5) |
					(b >> 3);
			}
		} else {
			for (x = 0; x < width; x++) {
				u32 px = s[x];
				u8  r  = lut[0][(px >> 16) & 0xff];
				u8  g  = lut[1][(px >>  8) & 0xff];
				u8  b  = lut[2][ px        & 0xff];

				d[x] = ((r >> 3) << 11) |
				       ((g >> 2) <<  5) |
					(b >> 3);
			}
		}
	}
}

/* ------------------------------------------------------------------ */
/* In-buffer rotation + reflect                                       */
/* ------------------------------------------------------------------ */

/*
 * Note: the caller is responsible for sizing `dst` correctly:
 *   - (width x height) for ROTATE_0 / ROTATE_180
 *   - (height x width) for ROTATE_90 / ROTATE_270
 *
 * mpro_drm__copy_frame() currently allocates temp_buffer at
 * width*height, so 90/270 rotation requires a full-frame update.
 * pipe_update() forces this whenever rotation != 0.
 */
static void mpro_drm__rotate_buffer(u16 *dst, u16 *src, int width, int height,
				    unsigned int rotation)
{
	const bool reflect_x = !!(rotation & DRM_MODE_REFLECT_X);
	const bool reflect_y = !!(rotation & DRM_MODE_REFLECT_Y);
	const unsigned int rot = rotation &
		(DRM_MODE_ROTATE_0  | DRM_MODE_ROTATE_90 |
		 DRM_MODE_ROTATE_180 | DRM_MODE_ROTATE_270);
	int x, y;

	/* Fast path: no rotation and no reflection → straight memcpy */
	if (rot == DRM_MODE_ROTATE_0 && !reflect_x && !reflect_y) {
		memcpy(dst, src, (size_t)width * height * 2);
		return;
	}

	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			int sx = reflect_x ? (width  - 1 - x) : x;
			int sy = reflect_y ? (height - 1 - y) : y;
			u16 px = src[sy * width + sx];
			int dx, dy, dw;

			switch (rot) {
			case DRM_MODE_ROTATE_0:
			default:
				dx = x;
				dy = y;
				dw = width;
				break;
			case DRM_MODE_ROTATE_90:
				dx = height - 1 - y;
				dy = x;
				dw = height;
				break;
			case DRM_MODE_ROTATE_180:
				dx = width  - 1 - x;
				dy = height - 1 - y;
				dw = width;
				break;
			case DRM_MODE_ROTATE_270:
				dx = y;
				dy = width - 1 - x;
				dw = height;
				break;
			}
			dst[dy * dw + dx] = px;
		}
	}
}

/* ------------------------------------------------------------------ */
/* High-level frame copy                                              */
/* ------------------------------------------------------------------ */

int mpro_drm__copy_frame(struct mpro_drm *mdrm, struct iosys_map *src,
			 struct drm_framebuffer *fb, struct drm_rect *clip)
{
	u32 width, height;
	u8  *src_ptr;
	u8  *dst_ptr;
	u32 dst_pitch = mdrm->width * 2;
	u16 *temp_buffer = NULL;
	u16 *final_dst;
	u8 lut_local[3][256];
	bool transform_needed;
	u16 effective = mpro_drm__compose_rotation(
		READ_ONCE(mdrm->native_rotation),
		READ_ONCE(mdrm->plane_rotation));
	bool needs_rotation = (effective != DRM_MODE_ROTATE_0);

	mutex_lock(&mdrm->lut_lock);
	transform_needed = mdrm->gamma_valid ||
			mdrm->brightness < MPRO_BRIGHTNESS_MAX;
	memcpy(lut_local, mdrm->lut_combined, sizeof(lut_local));
	mutex_unlock(&mdrm->lut_lock);

	if (clip->x2 > (int)mdrm->width || clip->y2 > (int)mdrm->height ||
	    clip->x1 < 0 || clip->y1 < 0)
		return -EINVAL;

	if (!mpro_drm__fb_valid(fb) || !src || iosys_map_is_null(src))
		return -EINVAL;
	if (!mpro_drm__clip_valid(clip))
		return -EINVAL;

	width  = clip->x2 - clip->x1;
	height = clip->y2 - clip->y1;

	src_ptr = src->vaddr +
		  clip->y1 * fb->pitches[0] +
		  clip->x1 * fb->format->cpp[0];

	dst_ptr = mdrm->data +
		  clip->y1 * dst_pitch +
		  clip->x1 * 2;

	if (needs_rotation) {
		temp_buffer = kmalloc((size_t)width * (size_t)height * 2,
				      GFP_KERNEL);
		if (!temp_buffer)
			return -ENOMEM;
	}

	final_dst = needs_rotation ? temp_buffer : (u16 *)dst_ptr;

	switch (fb->format->format) {
	case DRM_FORMAT_RGB565:
		mpro_drm__copy_rgb565((u8 *)final_dst, src_ptr,
				      width, height,
				      dst_pitch, fb->pitches[0],
				      (const u8 (*)[256])lut_local,
				      transform_needed);
		break;

	case DRM_FORMAT_XRGB8888:
		if (!transform_needed) {
			struct iosys_map dst_map =
				IOSYS_MAP_INIT_VADDR(final_dst);
			const unsigned int dst_pitch_arr[1] = { dst_pitch };

			drm_fb_xrgb8888_to_rgb565(&dst_map, dst_pitch_arr,
						  src, fb, clip,
						  &mdrm->conv_state);
			break;
		}
		mpro_drm__convert_xrgb8888_to_rgb565(final_dst,
				(u32 *)src_ptr,
				width, height,
				dst_pitch, fb->pitches[0],
				(const u8 (*)[256])lut_local,
				transform_needed);
		break;

	default:
		kfree(temp_buffer);
		DRM_ERROR("unsupported format: %08x\n", fb->format->format);
		return -EINVAL;
	}

	if (needs_rotation) {
		mpro_drm__rotate_buffer((u16 *)dst_ptr, temp_buffer,
					width, height, effective);
		kfree(temp_buffer);
	}
	return 0;
}
