// SPDX-License-Identifier: GPL-2.0-only
/*
 * beada-drm-pm.c – BeadaPanel DRM child: resume handler
 *
 * Single entry point: beada_drm_resume_handler() is registered with the
 * MFD parent at probe and called from beada_usb_resume() after the bus
 * is back up and Panel-Link has been re-armed.
 *
 * What we do here
 * ───────────────
 * Re-push the shadow buffer (or a blank frame, depending on state) to
 * hardware so the user sees the previous image immediately after the
 * device wakes up — without having to wait for the next compositor
 * commit, which on a frozen userspace can take an arbitrary amount of
 * time after resume.
 *
 *   shadow_valid       → re-send shadow_buf (last fully rendered frame)
 *   !shadow_valid &&
 *      !bdev->noblank  → send blank_buf (clear stale clock screen)
 *   !shadow_valid &&
 *      bdev->noblank   → do nothing (caller wants whatever's there)
 *
 * Why a separate file
 * ───────────────────
 * Keeps platform_driver / drm_driver / module boilerplate in
 * beada-drm-drv.c, the blit and pipe callbacks in beada-drm-plane.c,
 * and the (very small) PM bridge here.  If the resume path ever grows
 * more responsibilities (multi-stage restore, telemetry, etc.), this
 * file is the obvious home for them.
 */

#include <linux/device.h>
#include <linux/mutex.h>

#include "../beada.h"
#include "beada-drm.h"

void beada_drm_resume_handler(void *priv)
{
	struct beada_drm_dev *bdev = priv;
	const void *src;
	int ret;

	/*
	 * Decide what to push BEFORE calling beada_send_frame() so we
	 * never hold the shadow lock across the (asynchronous) submit
	 * call.  send_frame just queues the work and returns; the
	 * worker reads shadow_buf later — by which time the shadow
	 * lock is free again.
	 */
	mutex_lock(&bdev->shadow_lock);
	if (bdev->shadow_valid)
		src = bdev->shadow_buf;
	else if (!bdev->noblank)
		src = bdev->blank_buf;
	else
		src = NULL;
	mutex_unlock(&bdev->shadow_lock);

	if (!src)
		return;

	ret = beada_send_frame(bdev->beada, src, bdev->shadow_size,
			       bdev->panel->width, bdev->panel->height);
	if (ret)
		dev_warn(bdev->drm.dev,
			 "resume frame restore failed: %d\n", ret);
}
