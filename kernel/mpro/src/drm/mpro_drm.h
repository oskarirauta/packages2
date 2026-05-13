/* SPDX-License-Identifier: GPL-2.0 */
/*
 * MPro DRM driver — internal header.
 *
 * Shared types, inline helpers and cross-file function prototypes.
 * Module-internal symbols use the mpro_drm__ prefix to distinguish
 * them from the parent's public API (mpro_*).
 */
#ifndef _MPRO_DRM_H_
#define _MPRO_DRM_H_ 1

#include <drm/drm_drv.h>
#include <drm/drm_device.h>
#include <drm/drm_simple_kms_helper.h>
#include <drm/drm_format_helper.h>
#include <drm/drm_fb_helper.h>
#include <drm/drm_rect.h>
#include <drm/drm_blend.h>
#include <drm/drm_atomic.h>
#include <drm/drm_framebuffer.h>
#include <linux/iosys-map.h>
#include <linux/hrtimer.h>
#include <linux/platform_device.h>

#include "../mpro.h"

#define DRIVER_NAME		"mpro_drm"
#define DRIVER_DESC		"MPRO DRM driver"
#define DRIVER_MAJOR		1
#define DRIVER_MINOR		0

#define MPRO_BRIGHTNESS_DEFAULT	100
#define MPRO_BRIGHTNESS_MIN	0
#define MPRO_BRIGHTNESS_MAX	100

/* LUT size = 3 channels * 256 entries */
#define MPRO_DRM_LUT_SIZE	768

/* Display timing — shared between mode construction and EDID */
#define MPRO_TIMING_HSYNC_OFF	40
#define MPRO_TIMING_HSYNC_PULSE	48
#define MPRO_TIMING_HBACK	40
#define MPRO_TIMING_VSYNC_OFF	13
#define MPRO_TIMING_VSYNC_PULSE	3
#define MPRO_TIMING_VBACK	29
#define MPRO_TIMING_CLOCK_KHZ	29729

/* ------------------------------------------------------------------ */
/* Per-instance state                                                 */
/* ------------------------------------------------------------------ */

struct mpro_drm {
	struct drm_device		drm;
	struct drm_simple_display_pipe	pipe;
	struct drm_connector		connector;
	struct drm_format_conv_state	conv_state;

	struct mpro_device		*mpro;
	struct platform_device		*pdev;
	struct device			*dev;

	u32				width;
	u32				height;

	void				*data;
	size_t				data_size;

	struct hrtimer			vblank_timer;
	ktime_t				frame_time;
	bool				vblank_enabled;
	bool				vblank_was_enabled_pre_suspend;

	u16				native_rotation;
	u16				plane_rotation;
	bool				suspended;

	/* Color management */
	u8				lut[3][256];	  /* per-channel software gamma */
	u8				lut_combined[3][256]; /* gamma * brightness */
	bool				gamma_valid;
	u32				brightness;	  /* 0..MPRO_BRIGHTNESS_MAX */
	struct drm_property		*brightness_prop;
	u32				gamma_x100;	  /* gamma * 100 */
	struct mutex			lut_lock;

	bool				blanked;
	bool				disable_partial;
	bool				active_held;

	struct mpro_screen_listener	screen_listener;
};

/* ------------------------------------------------------------------ */
/* Inline helpers                                                     */
/* ------------------------------------------------------------------ */

static inline bool mpro_drm__fb_valid(struct drm_framebuffer *fb)
{
	return fb && fb->format && fb->pitches[0];
}

static inline bool mpro_drm__clip_valid(const struct drm_rect *r)
{
	return r && r->x2 > r->x1 && r->y2 > r->y1;
}

static inline bool mpro_drm__rotation_swaps_dims(u16 r)
{
	r &= ~(DRM_MODE_REFLECT_X | DRM_MODE_REFLECT_Y);
	return r == DRM_MODE_ROTATE_90 || r == DRM_MODE_ROTATE_270;
}

/* ------------------------------------------------------------------ */
/* Cross-file prototypes                                              */
/* ------------------------------------------------------------------ */

/* mpro_drm_pipe.c */
enum hrtimer_restart mpro_drm__vblank_timer(struct hrtimer *timer);
extern const struct drm_simple_display_pipe_funcs	mpro_drm__pipe_funcs;
extern const struct drm_mode_config_funcs		mpro_drm__mode_config_funcs;
extern const struct drm_connector_funcs			mpro_drm__connector_funcs;
extern const struct drm_connector_helper_funcs		mpro_drm__connector_helper_funcs;

int  mpro_drm__request_update(struct mpro_drm *mdrm,
			      u16 x, u16 y, u16 w, u16 h, bool force);
void mpro_drm__fb_mark_dirty(struct mpro_drm *mdrm, struct iosys_map *src,
			     struct drm_framebuffer *fb, struct drm_rect *rect);
void mpro_drm__screen_wake(void *priv);

/* mpro_drm_color.c */
int  mpro_drm__copy_frame(struct mpro_drm *mdrm, struct iosys_map *src,
			  struct drm_framebuffer *fb, struct drm_rect *clip);
void mpro_drm__rebuild_combined_lut(struct mpro_drm *mdrm);
void mpro_drm__build_power_lut(struct mpro_drm *mdrm, u32 g_x100);
void mpro_drm__apply_color_mgmt(struct mpro_drm *mdrm,
				struct drm_crtc_state *crtc_state);

/* mpro_drm_sysfs.c */
int  mpro_drm__sysfs_create(struct mpro_drm *mdrm);
void mpro_drm__sysfs_remove(struct mpro_drm *mdrm);
void mpro_drm__apply_rotation(struct mpro_drm *mdrm, u16 new_rotation);

/* mpro_drm_edid.c */
int  mpro_drm__create_edid(struct mpro_drm *mdrm);

static inline u16 mpro_drm__compose_rotation(u16 base, u16 extra)
{
	static const u16 angle_add[4][4] = {
		{ DRM_MODE_ROTATE_0,   DRM_MODE_ROTATE_90,
		  DRM_MODE_ROTATE_180, DRM_MODE_ROTATE_270 },
		{ DRM_MODE_ROTATE_90,  DRM_MODE_ROTATE_180,
		  DRM_MODE_ROTATE_270, DRM_MODE_ROTATE_0   },
		{ DRM_MODE_ROTATE_180, DRM_MODE_ROTATE_270,
		  DRM_MODE_ROTATE_0,   DRM_MODE_ROTATE_90  },
		{ DRM_MODE_ROTATE_270, DRM_MODE_ROTATE_0,
		  DRM_MODE_ROTATE_90,  DRM_MODE_ROTATE_180 },
	};
	static const unsigned int angle_index[] = {
		[__builtin_ctz(DRM_MODE_ROTATE_0)]   = 0,
		[__builtin_ctz(DRM_MODE_ROTATE_90)]  = 1,
		[__builtin_ctz(DRM_MODE_ROTATE_180)] = 2,
		[__builtin_ctz(DRM_MODE_ROTATE_270)] = 3,
	};
	unsigned int base_angle  = base  & (DRM_MODE_ROTATE_0  | DRM_MODE_ROTATE_90 |
					    DRM_MODE_ROTATE_180 | DRM_MODE_ROTATE_270);
	unsigned int extra_angle = extra & (DRM_MODE_ROTATE_0  | DRM_MODE_ROTATE_90 |
					    DRM_MODE_ROTATE_180 | DRM_MODE_ROTATE_270);
	bool base_rx  = !!(base  & DRM_MODE_REFLECT_X);
	bool base_ry  = !!(base  & DRM_MODE_REFLECT_Y);
	bool extra_rx = !!(extra & DRM_MODE_REFLECT_X);
	bool extra_ry = !!(extra & DRM_MODE_REFLECT_Y);
	u16  result   = angle_add[angle_index[__builtin_ctz(base_angle)]]
				  [angle_index[__builtin_ctz(extra_angle)]];

	if (base_rx ^ extra_rx)
		result |= DRM_MODE_REFLECT_X;
	if (base_ry ^ extra_ry)
		result |= DRM_MODE_REFLECT_Y;

	return result;
}

#endif /* _MPRO_DRM_H_ */
