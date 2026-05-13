// SPDX-License-Identifier: GPL-2.0-only
/*
 * beada-pm.c – BeadaPanel MFD parent: USB power management
 *
 * Three concerns live here:
 *
 *   1. System suspend / resume callbacks (.suspend, .resume,
 *      .reset_resume on struct usb_driver)
 *   2. USB autosuspend lifecycle hooks called by the DRM child
 *      (beada_pipe_active / _idle)
 *   3. Resume handler list — child drivers subscribe to be notified
 *      after the bus is back so they can re-push state the device
 *      lost across the suspend cycle
 *
 * Why not standard platform_driver .pm callbacks on the children?
 * ───────────────────────────────────────────────────────────────
 * Because the work the children need to do on resume depends on the
 * MFD parent having ALREADY completed its own resume (PL_RESET, clock
 * re-sync, zeroed dimension cache).  PM-core's parent-first / child-
 * last ordering is the right shape but the parent's resume is async
 * relative to children — easier and more obvious to invoke children
 * explicitly from inside the parent's resume callback at the exact
 * moment we're ready for them.
 *
 * Suspend / resume flow
 * ─────────────────────
 *   suspend:
 *     - PM core (or USB autopm) calls usb_driver->suspend
 *     - We flush the workqueue (any in-flight frame finishes)
 *     - We flag @suspended so any rogue USB caller short-circuits
 *
 *   resume:
 *     - USB stack brings the bus back up
 *     - PM core calls usb_driver->resume (or .reset_resume on a
 *       cold-reset wakeup)
 *     - We clear @suspended
 *     - We send PL_RESET — see "PL_RESET on resume" below
 *     - We sleep 50 ms for the firmware re-init
 *     - We zero @cached_frame_w/h so the next frame re-sends START
 *     - We re-sync the device clock (PL_RESET wiped it)
 *     - We call every registered resume handler so children can
 *       re-push their state (DRM: shadow buffer; backlight: stored
 *       brightness value)
 *
 * PL_RESET on resume
 * ──────────────────
 * Across a suspend cycle the device's USB peripheral controller may
 * be power-cycled, which loses the Panel-Link dimension latch.  Our
 * @cached_frame_w/h would still be non-zero, so without zeroing the
 * cache and sending PL_RESET our next frame would skip the START tag
 * and the firmware would consume 270 bytes of pixel data as a stale
 * tag — the same misalignment we recover from at probe.
 *
 * If the device DID retain the latch (depends on bus power state),
 * the PL_RESET is harmless: it costs ~50 ms of firmware re-init and
 * a brief flash of the device's default standby clock before DRM
 * resume overwrites the display with the shadow buffer.
 *
 * Autopm (beada_pipe_active / _idle)
 * ──────────────────────────────────
 * The DRM child holds an autopm reference for the lifetime of an
 * enabled pipe.  This keeps the device powered even when the pipe
 * is "idle but enabled" (Weston dim + backlight off, etc.).  When
 * the pipe closes, the reference is released and the autosuspend
 * timer starts.  Status-Link sysfs writes take their own short-lived
 * autopm reference; see beada-pipeline.c.
 *
 * Future work
 * ───────────
 * If hardware autosuspend turns out to be unreliable (the mpro
 * project had this problem and ended up with a software-only
 * "virtual idle" instead), this file is where the timer-based
 * fallback would live.  beada-pipeline.c would stay focused on
 * transport and beada-pm.c would grow a tick handler that flushes
 * a blank frame and SET_BL 0 without going through usb_driver
 * suspend.
 */

#include <linux/delay.h>
#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/usb.h>

#include "beada-priv.h"

/* ------------------------------------------------------------------ */
/* Resume handler list                                                  */
/* ------------------------------------------------------------------ */

int beada_register_resume_handler(struct beada_mfd_dev *beada,
				  struct beada_resume_handler *h)
{
	mutex_lock(&beada->resume_handlers_lock);
	list_add_tail(&h->list, &beada->resume_handlers);
	mutex_unlock(&beada->resume_handlers_lock);
	return 0;
}
EXPORT_SYMBOL_GPL(beada_register_resume_handler);

void beada_unregister_resume_handler(struct beada_mfd_dev *beada,
				     struct beada_resume_handler *h)
{
	mutex_lock(&beada->resume_handlers_lock);
	list_del(&h->list);
	mutex_unlock(&beada->resume_handlers_lock);
}
EXPORT_SYMBOL_GPL(beada_unregister_resume_handler);

/* ------------------------------------------------------------------ */
/* System suspend / resume                                              */
/* ------------------------------------------------------------------ */

int beada_usb_suspend(struct usb_interface *intf, pm_message_t message)
{
	struct beada_mfd_dev *beada = usb_get_intfdata(intf);

	if (!beada)
		return 0;

	/*
	 * Cancel a pending resume-handler dispatch.  Either an earlier
	 * resume just finished and we never got around to running its
	 * handlers, or we're racing a resume/suspend cycle.  Either way
	 * the work would touch USB during the suspend window, which the
	 * bus has already taken down.  Cancellation is the right answer:
	 * a subsequent resume re-queues a fresh dispatch.
	 *
	 * Use _sync variant since suspend must be in a quiescent state
	 * when it returns.
	 */
	cancel_delayed_work_sync(&beada->resume_work);

	/*
	 * Drain any in-flight async frame transfer.  Children's PM
	 * callbacks have already run (PM core invokes children before
	 * parent on the way down).
	 */
	flush_work(&beada->xfer_work);

	mutex_lock(&beada->xfer_lock);
	beada->suspended = true;
	mutex_unlock(&beada->xfer_lock);

	dev_dbg(beada->dev, "USB suspended\n");
	return 0;
}

void beada_resume_handlers_work(struct work_struct *work)
{
	struct beada_mfd_dev *beada = container_of(to_delayed_work(work),
						   struct beada_mfd_dev,
						   resume_work);
	struct beada_resume_handler *h;
	int ret;

	/*
	 * Bail out cleanly if the device went away or re-suspended while
	 * we were waiting on the timer.  Reading the flags lock-free is
	 * fine: a stale "false" read here just means we proceed and the
	 * autopm_get below either succeeds (device is up after all) or
	 * fails (device really is gone, and we return without harm).
	 */
	if (READ_ONCE(beada->disconnected) || READ_ONCE(beada->suspended)) {
		dev_dbg(beada->dev,
			"resume handlers skipped (disconnected=%d suspended=%d)\n",
			READ_ONCE(beada->disconnected),
			READ_ONCE(beada->suspended));
		return;
	}

	/*
	 * Re-take an autopm reference for the duration of handler
	 * dispatch.  The original resume-trigger's reference was released
	 * by its caller long ago; the device may have re-suspended in the
	 * meantime (especially with the long settling delay we use to
	 * give the firmware time to accept SL_SET_BL after PL_RESET).
	 *
	 * If the device IS suspended, this call synchronously resumes it
	 * — which re-enters beada_usb_resume() and queues a fresh delayed
	 * work.  That's fine: the fresh dispatch supersedes us; we still
	 * run handlers once here for our own queued instance, but the
	 * fresh resume's handlers will run again 1.2 s later with
	 * up-to-date state.  In practice this corner case is benign.
	 */
	ret = usb_autopm_get_interface(beada->intf);
	if (ret) {
		dev_dbg(beada->dev,
			"resume handlers skipped: autopm_get failed (%d)\n",
			ret);
		return;
	}

	mutex_lock(&beada->resume_handlers_lock);
	list_for_each_entry(h, &beada->resume_handlers, list)
		h->handler(h->priv);
	mutex_unlock(&beada->resume_handlers_lock);

	usb_autopm_put_interface(beada->intf);
}

int beada_usb_resume(struct usb_interface *intf)
{
	struct beada_mfd_dev *beada = usb_get_intfdata(intf);
	int ret;

	if (!beada)
		return 0;

	mutex_lock(&beada->xfer_lock);
	beada->suspended = false;
	mutex_unlock(&beada->xfer_lock);

	WRITE_ONCE(beada->in_resume, true);

	ret = beada_panellink_reset(beada);
	if (ret && ret != -EOPNOTSUPP)
		dev_warn(beada->dev,
			 "PL_RESET on resume failed: %d\n", ret);

	msleep(50);

	mutex_lock(&beada->xfer_lock);
	beada->cached_frame_w = 0;
	beada->cached_frame_h = 0;
	mutex_unlock(&beada->xfer_lock);

	if (beada->has_statuslink) {
		ret = beada_sync_time(beada);
		if (ret && ret != -EOPNOTSUPP && ret != -EHOSTDOWN)
			dev_warn(beada->dev,
				 "clock re-sync on resume failed: %d\n", ret);
	}

	WRITE_ONCE(beada->in_resume, false);

	/*
	 * Defer handler dispatch to a delayed work.  We need to return
	 * quickly so the autopm_get that started this resume can finish,
	 * AND we need to give the device firmware a moment to settle
	 * before further Status-Link traffic — running handlers here
	 * directly has been observed to make SL_SET_BL time out.
	 *
	 * 200 ms is plenty in practice; the user-visible effect is that
	 * the last-known brightness reappears a fraction of a second
	 * after the image does, which is invisible during a kmscube
	 * relaunch.
	 */
	queue_delayed_work(beada->xfer_wq, &beada->resume_work,
			   msecs_to_jiffies(1200));

	dev_dbg(beada->dev, "USB resumed (handlers deferred)\n");
	return 0;
}

/* ------------------------------------------------------------------ */
/* Pipe lifecycle (autopm)                                              */
/* ------------------------------------------------------------------ */

int beada_pipe_active(struct beada_mfd_dev *beada)
{
	return usb_autopm_get_interface(beada->intf);
}
EXPORT_SYMBOL_GPL(beada_pipe_active);

void beada_pipe_idle(struct beada_mfd_dev *beada)
{
	usb_autopm_put_interface(beada->intf);
}
EXPORT_SYMBOL_GPL(beada_pipe_idle);
