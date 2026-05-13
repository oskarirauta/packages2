// SPDX-License-Identifier: GPL-2.0-only
/*
 * ax206-drm-stats.c - AX206 DRM frame statistics
 *
 * Tracks per-device frame counters and an EWMA frames-per-second
 * estimate.  All updates use the stats spinlock for consistency;
 * readers (sysfs) acquire the same lock.
 *
 * fps_ewma is stored as integer * 1000 (milliframes-per-second).
 *
 * Copyright (C) 2025 Oskari Rauta <oskari.rauta@gmail.com>
 */

#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/ktime.h>
#include <linux/time64.h>

#include "ax206-drm-priv.h"

/* EWMA weight for new samples: alpha = 1/8 */
#define EWMA_SHIFT	3

void ax206_stats_init(struct ax206_stats *s)
{
	spin_lock_init(&s->lock);
	s->frames_received  = 0;
	s->frames_drawn     = 0;
	s->frames_skipped   = 0;
	s->frames_error     = 0;
	s->fps_ewma         = 0;
	s->last_frame_time  = 0;
}

void ax206_stats_reset(struct ax206_stats *s)
{
	unsigned long flags;

	spin_lock_irqsave(&s->lock, flags);
	s->frames_received  = 0;
	s->frames_drawn     = 0;
	s->frames_skipped   = 0;
	s->frames_error     = 0;
	s->fps_ewma         = 0;
	s->last_frame_time  = 0;
	spin_unlock_irqrestore(&s->lock, flags);
}

static void _update_fps(struct ax206_stats *s)
{
	ktime_t now = ktime_get();
	ktime_t delta;
	u32 fps_now;

	if (s->last_frame_time == 0) {
		s->last_frame_time = now;
		return;
	}

	delta = ktime_sub(now, s->last_frame_time);
	s->last_frame_time = now;

	if (ktime_to_ns(delta) == 0)
		return;

	/* fps_now in milliframes-per-second */
	fps_now = (u32)div64_u64(1000ULL * NSEC_PER_SEC,
				 (u64)ktime_to_ns(delta));

	if (s->fps_ewma == 0)
		s->fps_ewma = fps_now;
	else
		s->fps_ewma = s->fps_ewma -
			(s->fps_ewma >> EWMA_SHIFT) +
			(fps_now     >> EWMA_SHIFT);
}

void ax206_stats_frame_received(struct ax206_stats *s)
{
	unsigned long flags;

	spin_lock_irqsave(&s->lock, flags);
	s->frames_received++;
	_update_fps(s);
	spin_unlock_irqrestore(&s->lock, flags);
}

void ax206_stats_frame_drawn(struct ax206_stats *s)
{
	unsigned long flags;

	spin_lock_irqsave(&s->lock, flags);
	s->frames_drawn++;
	spin_unlock_irqrestore(&s->lock, flags);
}

void ax206_stats_frame_skipped(struct ax206_stats *s)
{
	unsigned long flags;

	spin_lock_irqsave(&s->lock, flags);
	s->frames_skipped++;
	spin_unlock_irqrestore(&s->lock, flags);
}

void ax206_stats_frame_error(struct ax206_stats *s)
{
	unsigned long flags;

	spin_lock_irqsave(&s->lock, flags);
	s->frames_error++;
	spin_unlock_irqrestore(&s->lock, flags);
}

/*
 * ax206_stats_show() - format stats into a sysfs buffer
 *
 * Returns bytes written.
 */
ssize_t ax206_stats_show(struct ax206_stats *s, char *buf)
{
	unsigned long flags;
	u64 recv, drawn, skip, err;
	u32 fps;

	spin_lock_irqsave(&s->lock, flags);
	recv  = s->frames_received;
	drawn = s->frames_drawn;
	skip  = s->frames_skipped;
	err   = s->frames_error;
	fps   = s->fps_ewma;
	spin_unlock_irqrestore(&s->lock, flags);

	return sysfs_emit(buf,
		"received:  %llu\n"
		"drawn:     %llu\n"
		"skipped:   %llu\n"
		"errors:    %llu\n"
		"fps_ewma:  %u.%03u\n",
		recv, drawn, skip, err,
		fps / 1000, fps % 1000);
}
