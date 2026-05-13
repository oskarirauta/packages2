// SPDX-License-Identifier: GPL-2.0
/*
 * mpro_pm.c — virtual idle management for the MPro parent driver.
 *
 * The device firmware does not support a working USB-bus resume:
 * once the USB host suspends the link, the device fails to respond
 * to control transfers and the USB stack escalates to a port reset.
 * Real USB autosuspend is therefore not usable.
 *
 * Instead we implement a logical idle state. The USB bus stays
 * active, but after a configurable idle delay (default 30 s) the
 * backlight is switched off and the touchscreen URB pipeline is
 * stopped. This protects the device's backlight, which has a
 * limited lifetime when left lit through long idle periods.
 *
 * Children take and release "active references" via mpro_active_get()
 * and mpro_active_put(). DRM holds one across pipe_enable/pipe_disable;
 * the touchscreen holds one across input_open/input_close. While at
 * least one reference is held the device is considered active. When
 * the last reference is dropped, the idle work is scheduled. The
 * next get cancels the work, or wakes the device if it had already
 * gone idle.
 *
 * USB suspend/resume callbacks remain in place for system-level PM
 * (S3, hibernate) only — runtime autosuspend is disabled at the
 * usb_driver level by leaving out supports_autosuspend.
 */

#include <linux/module.h>
#include <linux/workqueue.h>
#include <linux/ktime.h>
#include <linux/usb.h>

#include "mpro_internal.h"
#include "mpro.h"

/* ------------------------------------------------------------------ */
/* PM statistics                                                      */
/* ------------------------------------------------------------------ */

/*
 * Record a state transition. Caller must hold listeners_lock and must
 * NOT have changed mpro->is_idle yet — this helper inspects the
 * current state to decide which counter to bump.
 *
 * @new_idle:   target state after the transition
 * @touch_wake: true if this is a wake (idle -> active) caused by touch
 */
static void mpro_pm_record_transition(struct mpro_device *mpro,
				      bool new_idle, bool touch_wake)
{
	u64 now_ns = ktime_get_ns();
	u64 elapsed = now_ns - mpro->pm_state_changed_ns;

	/* Account elapsed time to the state we're leaving */
	if (mpro->is_idle)
		mpro->pm_idle_total_ns += elapsed;
	else
		mpro->pm_active_total_ns += elapsed;

	mpro->pm_state_changed_ns = now_ns;

	/* Count the transition (only if we're really changing state) */
	if (mpro->is_idle != new_idle) {
		if (new_idle) {
			mpro->pm_idle_count++;
		} else {
			mpro->pm_wake_count++;
			if (touch_wake)
				mpro->pm_touch_wake_count++;
		}
	}
}

/* ------------------------------------------------------------------ */
/* Idle work                                                          */
/* ------------------------------------------------------------------ */

static void mpro_idle_work(struct work_struct *work)
{
	struct mpro_device *mpro = container_of(to_delayed_work(work),
						struct mpro_device, idle_work);

	mutex_lock(&mpro->listeners_lock);

	if (mpro->is_idle) {
		mutex_unlock(&mpro->listeners_lock);
		return;
	}

	mpro_pm_record_transition(mpro, true, false);
	mpro->is_idle = true;
	mutex_unlock(&mpro->listeners_lock);

	dev_dbg(&mpro->intf->dev, "entering idle\n");

	/*
	 * Push a black frame before turning off the backlight. This is
	 * necessary when idle fires while the DRM pipe is still enabled
	 * (e.g. the compositor keeps the pipe active because it is
	 * receiving touch events). Subsequent frame submissions are
	 * silently dropped in mpro_send_xfer() until the display wakes.
	 */
	if (mpro->model) {
		size_t sz = (size_t)mpro->model->width *
			    mpro->model->height * 2;
		void *black = kzalloc(sz, GFP_KERNEL);

		if (black) {
			mpro_send_full_frame(mpro, black, sz);
			kfree(black);
		}
	}

	mpro_screen_notify_off(mpro);
}

/* ------------------------------------------------------------------ */
/* Lifecycle                                                          */
/* ------------------------------------------------------------------ */

static void mpro_wake_work(struct work_struct *work); /* forward */

void mpro_pm_init(struct mpro_device *mpro)
{
	INIT_DELAYED_WORK(&mpro->idle_work, mpro_idle_work);
	INIT_WORK(&mpro->wake_work, mpro_wake_work);
	atomic_set(&mpro->active_refs, 0);
	mpro->is_idle = false;

	/* Counters are zero-initialised by devm_kzalloc in probe */
	mpro->pm_state_changed_ns = ktime_get_ns();
}
EXPORT_SYMBOL_GPL(mpro_pm_init);

void mpro_pm_shutdown(struct mpro_device *mpro)
{
	cancel_delayed_work_sync(&mpro->idle_work);
	cancel_work_sync(&mpro->wake_work);
}
EXPORT_SYMBOL_GPL(mpro_pm_shutdown);

/* ------------------------------------------------------------------ */
/* Active reference management                                        */
/* ------------------------------------------------------------------ */

void mpro_active_get(struct mpro_device *mpro, bool *held)
{
	bool was_idle;

	if (mpro->fbdev_enabled) {
		/*
		 * fbdev console keeps the pipe permanently active. We
		 * still track refs (so sysfs reflects the state) but
		 * don't schedule idle.
		 */
		atomic_inc(&mpro->active_refs);
		*held = true;
		return;
	}

	atomic_inc(&mpro->active_refs);
	cancel_delayed_work_sync(&mpro->idle_work);

	mutex_lock(&mpro->listeners_lock);
	was_idle = mpro->is_idle;
	if (was_idle)
		mpro_pm_record_transition(mpro, false, false);
	mpro->is_idle = false;
	mutex_unlock(&mpro->listeners_lock);

	if (was_idle) {
		dev_dbg(&mpro->intf->dev, "leaving idle: backlight on\n");
		mpro_screen_notify_on(mpro);
	}

	*held = true;
}
EXPORT_SYMBOL_GPL(mpro_active_get);

void mpro_active_put(struct mpro_device *mpro, bool *held)
{
	if (!*held)
		return;

	*held = false;

	if (!atomic_dec_and_test(&mpro->active_refs))
		return;

	/* Last reference released */
	if (mpro->fbdev_enabled || mpro->idle_delay_ms == 0)
		return;

	schedule_delayed_work(&mpro->idle_work,
			      msecs_to_jiffies(mpro->idle_delay_ms));
}
EXPORT_SYMBOL_GPL(mpro_active_put);

/* ------------------------------------------------------------------ */
/* sysfs control — manual force idle / active                         */
/* ------------------------------------------------------------------ */

void mpro_pm_force_idle(struct mpro_device *mpro)
{
	bool changed = false;

	cancel_delayed_work_sync(&mpro->idle_work);

	mutex_lock(&mpro->listeners_lock);
	if (!mpro->is_idle) {
		mpro_pm_record_transition(mpro, true, false);
		mpro->is_idle = true;
		changed = true;
	}
	mutex_unlock(&mpro->listeners_lock);

	if (changed)
		mpro_screen_notify_off(mpro);
}
EXPORT_SYMBOL_GPL(mpro_pm_force_idle);

void mpro_pm_force_active(struct mpro_device *mpro)
{
	bool was_idle;

	cancel_delayed_work_sync(&mpro->idle_work);

	mutex_lock(&mpro->listeners_lock);
	was_idle = mpro->is_idle;
	if (was_idle)
		mpro_pm_record_transition(mpro, false, false);
	mpro->is_idle = false;
	mutex_unlock(&mpro->listeners_lock);

	if (was_idle)
		mpro_screen_notify_on(mpro);
}
EXPORT_SYMBOL_GPL(mpro_pm_force_active);

/* ------------------------------------------------------------------ */
/* Wake-on-touch                                                      */
/* ------------------------------------------------------------------ */

/*
 * Workqueue handler that performs the actual wake. mpro_pm_wake_async()
 * (called from URB-completion context) just schedules this.
 */
static void mpro_wake_work(struct work_struct *work)
{
	struct mpro_device *mpro = container_of(work, struct mpro_device,
						wake_work);
	bool was_idle;

	mutex_lock(&mpro->listeners_lock);
	was_idle = mpro->is_idle;
	if (was_idle)
		mpro_pm_record_transition(mpro, false, true);
	mpro->is_idle = false;
	mutex_unlock(&mpro->listeners_lock);

	if (!was_idle)
		return;

	mpro_screen_notify_on(mpro);

	/*
	 * Request an immediate repaint from any child that has content
	 * to display. This is needed when the DRM pipe stayed enabled
	 * while the display was idle — without this the screen would
	 * remain black until the compositor's next repaint cycle.
	 */
	mpro_screen_notify_wake(mpro);

	if (!mpro->fbdev_enabled && mpro->idle_delay_ms > 0 &&
	    atomic_read(&mpro->active_refs) == 0)
		schedule_delayed_work(&mpro->idle_work,
				      msecs_to_jiffies(mpro->idle_delay_ms));
}

void mpro_pm_touch_activity(struct mpro_device *mpro)
{
	if (mpro->fbdev_enabled || mpro->idle_delay_ms == 0)
		return;

	/*
	 * When the DRM pipe or another child holds an active reference,
	 * idle is managed by active_put() — don't interfere with that
	 * path by scheduling a parallel idle_work countdown.
	 */
	if (atomic_read(&mpro->active_refs) > 0)
		return;

	cancel_delayed_work(&mpro->idle_work);
	schedule_delayed_work(&mpro->idle_work,
			      msecs_to_jiffies(mpro->idle_delay_ms));
}
EXPORT_SYMBOL_GPL(mpro_pm_touch_activity);

/*
 * Schedule a wake from any context. Safe to call from IRQ / atomic
 * context — the actual wake work runs on the parent's ordered
 * workqueue.
 */
void mpro_pm_wake_async(struct mpro_device *mpro)
{
	if (!READ_ONCE(mpro->is_idle))
		return;

	queue_work(mpro->wq, &mpro->wake_work);
}
EXPORT_SYMBOL_GPL(mpro_pm_wake_async);
