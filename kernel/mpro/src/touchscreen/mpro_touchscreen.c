// SPDX-License-Identifier: GPL-2.0
/*
 * mpro_touchscreen.c — MPro USB touchscreen driver.
 *
 * Reads 14-byte touch packets from interrupt endpoint 0x81. Supports
 * two simultaneous touch points. Tracks the DRM-side rotation for
 * coordinate transformation, and applies per-model calibration flags
 * (touch_invert_x/y, touch_swap_xy) at the raw stage before rotation.
 *
 * The URB only runs while userspace has the input device open AND the
 * DRM pipe is active — releasing either of those conditions stops it.
 */

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/input/mt.h>
#include <linux/timer.h>
#include <linux/slab.h>

#include <drm/drm_blend.h>

#include "../drm/mpro_drm.h"
#include "mpro_touchscreen.h"

/* ------------------------------------------------------------------ */
/* Coordinate transformation                                          */
/* ------------------------------------------------------------------ */

static u16 mpro_ts__get_rotation(struct mpro_touch *mt)
{
	struct mpro_drm *mdrm = READ_ONCE(mt->mpro->drm);

	if (!mdrm)
		return DRM_MODE_ROTATE_0;

	/*
	 * Touch coordinates must follow the same effective rotation as
	 * the display: the composition of the device's physical
	 * orientation (native_rotation, set via sysfs) and the
	 * compositor's content rotation (plane_rotation, set via the
	 * DRM plane property).
	 */
	return mpro_drm__compose_rotation(
		READ_ONCE(mdrm->native_rotation),
		READ_ONCE(mdrm->plane_rotation));
}

/*
 * Convert device coordinates to DRM coordinates according to the
 * current rotation. The device's native resolution is
 * (model->width, model->height); the DRM-visible resolution may be
 * swapped for ROTATE_90 / ROTATE_270.
 */
static void mpro_ts__rotate(struct mpro_touch *mt, u16 *x, u16 *y)
{
	const u16 dw = mt->mpro->model->width;
	const u16 dh = mt->mpro->model->height;
	u16 r = mpro_ts__get_rotation(mt);
	bool refl_x = !!(r & DRM_MODE_REFLECT_X);
	bool refl_y = !!(r & DRM_MODE_REFLECT_Y);
	u16 nx = *x, ny = *y;

	if (refl_x)
		nx = dw - 1 - nx;
	if (refl_y)
		ny = dh - 1 - ny;

	switch (r & ~(DRM_MODE_REFLECT_X | DRM_MODE_REFLECT_Y)) {
	case DRM_MODE_ROTATE_90:
		*x = ny;
		*y = dw - 1 - nx;
		break;
	case DRM_MODE_ROTATE_180:
		*x = dw - 1 - nx;
		*y = dh - 1 - ny;
		break;
	case DRM_MODE_ROTATE_270:
		*x = dh - 1 - ny;
		*y = nx;
		break;
	default:
		*x = nx;
		*y = ny;
		break;
	}
}

/* ------------------------------------------------------------------ */
/* Touch release                                                      */
/* ------------------------------------------------------------------ */

static void mpro_ts__release_timeout(struct timer_list *t)
{
	struct mpro_touch *mt = timer_container_of(mt, t, release_timer);
	struct input_dev *input = mt->input;
	int i;

	dev_dbg(&mt->mpro->intf->dev,
		"release timeout — releasing all fingers\n");

	for (i = 0; i < MPRO_TOUCH_MAX_SLOTS; i++) {
		input_mt_slot(input, i);
		input_mt_report_slot_state(input, MT_TOOL_FINGER, false);
	}
	input_mt_sync_frame(input);
	input_sync(input);

	mt->any_active = false;
}

/* ------------------------------------------------------------------ */
/* Packet handling                                                    */
/* ------------------------------------------------------------------ */

static void mpro_ts__handle_packet(struct mpro_touch *mt,
				   const struct mpro_touch_packet *pkt)
{
	const struct mpro_model_info *model = mt->mpro->model;
	struct input_dev *input = mt->input;
	bool any_active = false;
	int i;

	for (i = 0; i < MPRO_TOUCH_MAX_SLOTS; i++) {
		const struct mpro_touch_point *p = &pkt->p[i];
		u16 x, y;
		u8 slot, state;
		bool active;

		slot  = p->yh.y.id;
		state = p->xh.x.f;

		/* Slot >= MPRO_TOUCH_MAX_SLOTS means "unused" */
		if (slot >= MPRO_TOUCH_MAX_SLOTS)
			continue;

		x = ((u16)p->xh.x.h << 8) | p->xl;
		y = ((u16)p->yh.y.h << 8) | p->yl;

		/* Touch panel orientation compensation, raw stage */
		if (model->touch_swap_xy) {
			u16 t = x;

			x = y;
			y = t;
		}
		if (model->touch_invert_x)
			x = model->width  - 1 - x;
		if (model->touch_invert_y)
			y = model->height - 1 - y;

		/*
		 * State codes on firmware v0.25 (verified by USB-protocol
		 * analysis; differs from the original userspace assumption):
		 *
		 *   0 = touch start marker
		 *       (coordinates may be stale)
		 *   1 = release (finger lifted, coordinates discarded)
		 *   2 = active touch / drag (coordinates valid)
		 *   3 = not observed in use
		 *
		 * Only state=2 reports an actual touch. state=1 triggers
		 * the automatic release through input_mt_sync_frame()
		 * because active=false.
		 */
		active = (state == 2);

		input_mt_slot(input, slot);
		input_mt_report_slot_state(input, MT_TOOL_FINGER, active);

		if (active) {
			mpro_ts__rotate(mt, &x, &y);
			input_report_abs(input, ABS_MT_POSITION_X, x);
			input_report_abs(input, ABS_MT_POSITION_Y, y);
			any_active = true;
		}

		dev_dbg(&mt->mpro->intf->dev,
			"slot=%u state=%u active=%d x=%u y=%u%s\n",
			slot, state, active, x, y,
			(state == 1) ? " [release]" : "");
	}

	input_mt_sync_frame(input);
	input_sync(input);

	/*
	 * Wake the display on touch only if something is listening to
	 * the input device. If nobody has it open, the touch is
	 * undeliverable anyway and waking the display would be surprising
	 * — consistent with how mice behave in Linux.
	 */
	if (any_active &&
	    READ_ONCE(mt->mpro->touch_wake_enabled) &&
	    READ_ONCE(mt->opened)) {

		/*
		 * Reset the idle countdown and wake the display if it
		 * went dark. mpro_pm_touch_activity() reschedules
		 * idle_work; mpro_pm_wake_async() is still needed for
		 * the is_idle → active transition.
		 */

		mpro_pm_touch_activity(mt->mpro);
		if (READ_ONCE(mt->mpro->is_idle))
			mpro_pm_wake_async(mt->mpro);
	}

	/*
	 * Release watchdog: the firmware sends a state=1 packet when a
	 * finger is lifted, which releases the touch immediately above.
	 * The watchdog timer is only a safety net for the case where a
	 * firmware bug or a dropped USB packet causes state=1 to never
	 * arrive — in that case the finger is released after
	 * MPRO_TOUCH_RELEASE_WATCHDOG_MS.
	 */
	if (any_active) {
		mt->any_active = true;
		mod_timer(&mt->release_timer,
			  jiffies +
			  msecs_to_jiffies(MPRO_TOUCH_RELEASE_WATCHDOG_MS));
	} else if (mt->any_active) {
		/* Packet itself confirmed the release — no timer needed */
		timer_delete(&mt->release_timer);
		mt->any_active = false;
	}
}

/* ------------------------------------------------------------------ */
/* Pipeline state management                                          */
/* ------------------------------------------------------------------ */

/*
 * Called from USB completion context (softirq). The parent has already
 * verified actual_length >= MPRO_TOUCH_PKT_SIZE, so the cast is safe.
 * mpro_ts__handle_packet() and everything it calls must not sleep.
 */
static void mpro_ts__touch_packet(void *priv, const void *data, size_t len)
{
	struct mpro_touch *mt = priv;

	mpro_ts__handle_packet(mt, (const struct mpro_touch_packet *)data);
}

/*
 * Start/stop gate: the URB runs while (opened && screen_on), and stops
 * otherwise. Called whenever either condition changes.
 *
 * When touch-wake is enabled, the URB also runs while the display is
 * idle (screen_on=false) as long as someone has the input device open.
 * This allows a touch to reach mpro_ts__handle_packet() which calls
 * mpro_pm_wake_async() to bring the display back. Without this, opening
 * the input node after idle is reached would find screen_on=false and
 * never start the URB, making touch-wake impossible.
 *
 * Always invoked from process context — safe to call mpro_touch_stop(),
 * which may block in usb_kill_urb().
 */
static void mpro_ts__resync(struct mpro_touch *mt)
{
	bool should_run;

	mutex_lock(&mt->lock);
	should_run = mt->opened &&
		     (mt->screen_on ||
		      READ_ONCE(mt->mpro->touch_wake_enabled));
	mutex_unlock(&mt->lock);

	if (should_run)
		mpro_touch_start(mt->mpro);
	else
		mpro_touch_stop(mt->mpro);
}

/* ------------------------------------------------------------------ */
/* input_dev callbacks                                                */
/* ------------------------------------------------------------------ */

static int mpro_ts__input_open(struct input_dev *input)
{
	struct mpro_touch *mt = input_get_drvdata(input);

	mutex_lock(&mt->lock);
	mt->opened = true;
	mutex_unlock(&mt->lock);

	mpro_ts__resync(mt);
	return 0;
}

static void mpro_ts__input_close(struct input_dev *input)
{
	struct mpro_touch *mt = input_get_drvdata(input);
	int i;

	mutex_lock(&mt->lock);
	mt->opened = false;
	mutex_unlock(&mt->lock);

	mpro_ts__resync(mt);

	/*
	 * Stop the release timer and clear MT state. This makes sure the
	 * next input_open() starts from a clean slate, even if the
	 * previous client (e.g. Weston) closed mid-touch without
	 * releasing first.
	 */
	timer_delete_sync(&mt->release_timer);

	for (i = 0; i < MPRO_TOUCH_MAX_SLOTS; i++) {
		input_mt_slot(input, i);
		input_mt_report_slot_state(input, MT_TOOL_FINGER, false);
	}

	input_mt_sync_frame(input);
	input_sync(input);
	mt->any_active = false;

	/*
	 * If the display was woken by a previous touch and no DRM pipe
	 * is active, restart the idle countdown now. Without this,
	 * closing the input node leaves the device permanently active
	 * because nothing else would schedule idle_work.
	 */
	if (!READ_ONCE(mt->mpro->is_idle))
		mpro_pm_touch_activity(mt->mpro);
}

/* ------------------------------------------------------------------ */
/* Screen state listener                                              */
/* ------------------------------------------------------------------ */

static void mpro_ts__screen_off(void *priv)
{
	struct mpro_touch *mt = priv;

	/*
	 * Keep the URB running during idle only if touch-wake is enabled
	 * AND userspace currently has the input device open. This mirrors
	 * standard Linux display-wake behaviour: a mouse does not wake
	 * a display unless something is listening to its events. evtest
	 * or similar tools that open the input node but don't drive the
	 * display should not prevent idle or trigger a wake.
	 */
	if (READ_ONCE(mt->mpro->touch_wake_enabled) &&
	    READ_ONCE(mt->opened))
		return;

	mutex_lock(&mt->lock);
	mt->screen_on = false;
	mutex_unlock(&mt->lock);

	mpro_ts__resync(mt);
}

static void mpro_ts__screen_on(void *priv)
{
	struct mpro_touch *mt = priv;

	mutex_lock(&mt->lock);
	mt->screen_on = true;
	mutex_unlock(&mt->lock);

	mpro_ts__resync(mt);
}

/* ------------------------------------------------------------------ */
/* Probe / remove                                                     */
/* ------------------------------------------------------------------ */

static int mpro_ts__probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct mpro_device *mpro = dev_get_drvdata(dev->parent);
	struct mpro_touch *mt;
	struct input_dev *input;
	int ret;

	if (!mpro || !mpro->model || !mpro->touch_urb)
		return -ENODEV;

	mt = devm_kzalloc(dev, sizeof(*mt), GFP_KERNEL);
	if (!mt)
		return -ENOMEM;

	mt->mpro      = mpro;
	mt->screen_on = true;
	mutex_init(&mt->lock);
	timer_setup(&mt->release_timer, mpro_ts__release_timeout, 0);

	input = devm_input_allocate_device(dev);
	if (!input)
		return -ENOMEM;

	mt->input = input;
	input_set_drvdata(input, mt);

	input->name		= "MPro touchscreen";
	input->id.bustype	= BUS_USB;
	input->id.vendor	= 0xc872;
	input->id.product	= 0x1004;
	input->open		= mpro_ts__input_open;
	input->close		= mpro_ts__input_close;
	input->dev.parent	= dev;

	input_set_capability(input, EV_KEY, BTN_TOUCH);
	input_set_abs_params(input, ABS_MT_POSITION_X,
			     0, mpro->model->width  - 1, 0, 0);
	input_set_abs_params(input, ABS_MT_POSITION_Y,
			     0, mpro->model->height - 1, 0, 0);

	/*
	 * INPUT_MT_DIRECT — touchscreen (direct touch, not a trackpad).
	 *
	 * INPUT_MT_DROP_UNUSED is deliberately omitted: the firmware does
	 * not always report both slots in every packet, and DROP_UNUSED
	 * could release an active slot by accident. Releases are handled
	 * explicitly through the state=1 packet and the watchdog timer.
	 */
	ret = input_mt_init_slots(input, MPRO_TOUCH_MAX_SLOTS, INPUT_MT_DIRECT);
	if (ret)
		return ret;

	ret = input_register_device(input);
	if (ret)
		return ret;

	mt->screen_listener.screen_off	= mpro_ts__screen_off;
	mt->screen_listener.screen_on	= mpro_ts__screen_on;
	mt->screen_listener.priv	= mt;
	ret = mpro_screen_listener_register(mpro, &mt->screen_listener);
	if (ret)
		return ret;

	mt->touch_listener.touch_packet	= mpro_ts__touch_packet;
	mt->touch_listener.priv		= mt;
	ret = mpro_touch_listener_register(mpro, &mt->touch_listener);
	if (ret) {
		mpro_screen_listener_unregister(mpro, &mt->screen_listener);
		return ret;
	}

	platform_set_drvdata(pdev, mt);

	dev_info(dev, "touchscreen registered (%ux%u, %d-finger MT)\n",
		 mpro->model->width, mpro->model->height,
		 MPRO_TOUCH_MAX_SLOTS);
	return 0;
}

static void mpro_ts__remove(struct platform_device *pdev)
{
	struct mpro_touch *mt = platform_get_drvdata(pdev);

	if (!mt)
		return;

	/*
	 * Unregister listeners before stopping the URB. This order
	 * ensures no touch_packet or screen_on/off callback fires after
	 * the URB is killed and the mt state is torn down.
	 */
	mpro_touch_listener_unregister(mt->mpro, &mt->touch_listener);
	mpro_screen_listener_unregister(mt->mpro, &mt->screen_listener);

	timer_delete_sync(&mt->release_timer);
	mpro_touch_stop(mt->mpro);
}

static struct platform_driver mpro_ts__driver = {
	.probe	= mpro_ts__probe,
	.remove	= mpro_ts__remove,
	.driver	= { .name = "mpro_touchscreen" },
};

module_platform_driver(mpro_ts__driver);

MODULE_AUTHOR("Oskari Rauta");
MODULE_DESCRIPTION("MPro USB touchscreen driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:mpro_touchscreen");
