/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * ax206-drm-priv.h - AX206 DRM child driver private header
 *
 * Copyright (C) 2025 Oskari Rauta <oskari.rauta@gmail.com>
 */

#ifndef _AX206_DRM_PRIV_H
#define _AX206_DRM_PRIV_H

#include <linux/types.h>
#include <linux/mutex.h>
#include <linux/workqueue.h>
#include <linux/timer.h>
#include <linux/spinlock.h>
#include <linux/atomic.h>

#include <drm/drm_device.h>
#include <drm/drm_gem.h>
#include <drm/drm_gem_shmem_helper.h>
#include <drm/drm_simple_kms_helper.h>
#include <drm/drm_connector.h>
#include <drm/drm_crtc.h>
#include <drm/drm_plane.h>
#include <drm/drm_framebuffer.h>
#include <drm/drm_modes.h>

#include "ax206.h"

/* Rotation modes */
#define AX206_ROTATE_0		0
#define AX206_ROTATE_90		1
#define AX206_ROTATE_180		2
#define AX206_ROTATE_270		3
#define AX206_ROTATE_0_HFLIP	4
#define AX206_ROTATE_90_HFLIP	5
#define AX206_ROTATE_180_HFLIP	6
#define AX206_ROTATE_270_HFLIP	7
#define AX206_ROTATE_COUNT	8

/* Default idle timeout (seconds, 0 = disabled) */
#define AX206_IDLE_TIMEOUT_DEFAULT	0

/* Coalescing delay before USB transfer (microseconds) */
#define AX206_BLIT_COALESCE_US		5000

/*
 * struct ax206_stats - frame statistics
 */
struct ax206_stats {
	spinlock_t	lock;
	u64		frames_received;
	u64		frames_drawn;
	u64		frames_skipped;
	u64		frames_error;
	u32		fps_ewma;          /* EWMA fps * 1000 */
	ktime_t		last_frame_time;
};

/*
 * struct ax206_drm_dev - DRM child device context
 */
struct ax206_drm_dev {
	struct drm_device		drm;
	struct ax206_dev			*dpf;

	struct drm_simple_display_pipe	pipe;
	struct drm_connector		connector;

	/* Fixed display mode */
	struct drm_display_mode		mode;

	/* Shadow buffer - big-endian RGB565, physical dimensions */
	u8				*shadow_buf;
	size_t				shadow_size;

	/* Logical dimensions (may be swapped relative to physical by rotation) */
	u16				lwidth;
	u16				lheight;

	/* Current rotation mode */
	int				rotation;

	/* noblank: keep last image when pipe closes */
	bool				noblank;

	/*
	 * Coalescing blit engine.
	 *
	 * blit_work is a single persistent work item.  When new damage
	 * arrives, it expands the pending dirty rectangle and sets
	 * blit_dirty.  If blit_active is false the work item is queued;
	 * otherwise the running work item will pick up the new data when
	 * it loops after completing the current USB transfer.
	 *
	 * A AX206_BLIT_COALESCE_US delay at the start of each loop iteration
	 * lets multiple rapid commits accumulate into one USB transfer.
	 */
	struct work_struct		blit_work;
	spinlock_t			blit_lock;  /* protects fields below */
	bool				blit_dirty;
	bool				blit_active;
	/* Physical dirty rectangle (x1/y1 are exclusive) */
	u16				dirty_x0, dirty_y0;
	u16				dirty_x1, dirty_y1;

	/* Virtual power-save / idle timer */
	struct timer_list		idle_timer;
	unsigned int			idle_timeout_s;
	bool				idle_active;
	struct work_struct		idle_work;
	struct work_struct		resume_work;

	/* Frame statistics */
	struct ax206_stats		stats;
};

static inline struct ax206_drm_dev *to_ax206_drm(struct drm_device *drm)
{
	return container_of(drm, struct ax206_drm_dev, drm);
}

/* ----- Sub-module prototypes ----- */

/* dpf-drm-plane.c */
void ax206_drm_pipe_update(struct drm_simple_display_pipe *pipe,
			 struct drm_plane_state *old_state);
void ax206_drm_pipe_enable(struct drm_simple_display_pipe *pipe,
			 struct drm_crtc_state *crtc_state,
			 struct drm_plane_state *plane_state);
void ax206_drm_pipe_disable(struct drm_simple_display_pipe *pipe);
void ax206_drm_blit_work_fn(struct work_struct *work);

/* dpf-drm-rotation.c */
void ax206_rotate_pixel(const struct ax206_drm_dev *d,
		      int lx, int ly, int *px, int *py);
void ax206_drm_set_rotation(struct ax206_drm_dev *d, int rot);
const char *ax206_rotation_name(int rot);

/* dpf-drm-pm.c */
void ax206_idle_timer_reset(struct ax206_drm_dev *d);
void ax206_idle_enter(struct ax206_drm_dev *d);
void ax206_idle_leave(struct ax206_drm_dev *d);
void ax206_idle_timer_fn(struct timer_list *t);
void ax206_idle_work_fn(struct work_struct *work);
void ax206_resume_work_fn(struct work_struct *work);

/* dpf-drm-stats.c */
void ax206_stats_init(struct ax206_stats *s);
void ax206_stats_frame_received(struct ax206_stats *s);
void ax206_stats_frame_drawn(struct ax206_stats *s);
void ax206_stats_frame_skipped(struct ax206_stats *s);
void ax206_stats_frame_error(struct ax206_stats *s);
void ax206_stats_reset(struct ax206_stats *s);
ssize_t ax206_stats_show(struct ax206_stats *s, char *buf);

/* dpf-drm-sysfs.c */
int ax206_drm_sysfs_init(struct ax206_drm_dev *d);
void ax206_drm_sysfs_fini(struct ax206_drm_dev *d);

#endif /* _AX206_DRM_PRIV_H */
