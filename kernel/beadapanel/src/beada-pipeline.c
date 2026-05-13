// SPDX-License-Identifier: GPL-2.0-only
/*
 * beada-pipeline.c – BeadaPanel high-level data pipeline
 *
 * Implements the public API exported to child drivers and the internal
 * helpers called from beada-main.c, beada-sysfs.c and beada-pm.c.
 *
 * Frame transfer model
 * ────────────────────
 * beada_send_frame() enqueues a job onto an ordered workqueue and returns
 * immediately.  The worker (beada_xfer_worker) holds xfer_lock for the
 * duration of one bulk transfer (~8–20 ms on USB 2.0 HS).
 * beada_flush_frame_transfer() blocks until the worker is idle; the DRM
 * child calls it at the top of every pipe_update so the next blit and
 * the previous USB transfer overlap.
 *
 * Single-buffer model: the caller's @buf pointer is stored in xfer_src
 * and remains the worker's source until the next flush.  Callers must
 * not modify @buf between send_frame() and flush_frame_transfer().
 *
 * Status-Link traffic (brightness, time sync, GET_INFO, PL_RESET) shares
 * the same xfer_lock so it cannot overlap with an in-flight Panel-Link
 * transfer.
 *
 * Autopm
 * ──────
 * The Panel-Link pixel pipeline does NOT touch usb_autopm_*: the DRM
 * child holds an autopm reference for the lifetime of an enabled pipe
 * via beada_pipe_active() / beada_pipe_idle() (see beada-pm.c).  So
 * when a frame reaches the worker the bus is guaranteed to be up.
 *
 * Status-Link transactions can arrive from sysfs at any time, including
 * while the device is autosuspended.  Those public functions
 * (beada_set_brightness, beada_sync_time) take their own short-lived
 * autopm reference around the transfer.
 */

#include <linux/delay.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/time.h>
#include <linux/timekeeping.h>

#include "beada-priv.h"

/* ------------------------------------------------------------------ */
/* Module parameters                                                    */
/* ------------------------------------------------------------------ */

static int tz_offset;
module_param(tz_offset, int, 0644);
MODULE_PARM_DESC(tz_offset,
	"Local timezone offset from UTC in minutes (e.g. 120 for EET, "
	"-300 for EST).  Default 0 = UTC.  Used by SET_TIME sent to the "
	"device.");

static bool tz_dst;
module_param(tz_dst, bool, 0644);
MODULE_PARM_DESC(tz_dst,
	"Add 60 minutes for daylight saving time.  Default 0.  Toggle "
	"twice a year (kernel does not handle DST rules).");

static int initial_brightness = -1;
module_param(initial_brightness, int, 0444);
MODULE_PARM_DESC(initial_brightness,
	"Override the backlight value reported to the backlight child "
	"driver.  Default -1 = use the device's current value read at "
	"GET_INFO time.  Any value 0..max_brightness sets a different "
	"initial level — useful to start the panel at a known brightness "
	"regardless of the previous session.  Note: takes effect on the "
	"next backlight-child probe; the MFD itself never writes SL_SET_BL "
	"on its own.");

/* ------------------------------------------------------------------ */
/* USB liveness check                                                   */
/* ------------------------------------------------------------------ */

/**
 * beada_check_alive() - Verify the device can accept a USB transfer
 *
 * Caller must hold @xfer_lock.  Returns:
 *   0           — device attached, not suspended; proceed
 *   -ENODEV     — disconnect() ran; do not touch USB
 *   -EHOSTDOWN  — system suspend is in progress; bus is down
 *
 * Used at the top of every public function that performs a USB
 * transfer.  Defense-in-depth: the autopm paths in set_brightness and
 * sync_time already resume the bus before touching USB, so @suspended
 * should never be true on that path; but if a transfer ever sneaks in
 * during the brief window between usb_driver->suspend setting the flag
 * and the bus actually going down, we catch it cleanly.
 */
static inline int beada_check_alive(struct beada_mfd_dev *beada)
{
	if (beada->disconnected)
		return -ENODEV;
	if (beada->suspended)
		return -EHOSTDOWN;
	return 0;
}

/* ------------------------------------------------------------------ */
/* sl_transact() — Status-Link send + (optional) receive               */
/* ------------------------------------------------------------------ */

/*
 * Caller holds xfer_lock and has already verified the device is alive.
 * Public Status-Link wrappers (set_brightness, sync_time) do that for
 * us; the probe-time GET_INFO does its own raw bulk I/O instead.
 */
static int sl_transact(struct beada_mfd_dev *beada,
		       void *cmd, size_t cmd_len,
		       void *reply, size_t reply_len)
{
	int ret;

	ret = beada_bulk_out(beada, beada->ep_sl_out,
			     cmd, cmd_len, SL_CMD_TIMEOUT_MS);
	if (ret)
		return ret;

	if (reply && reply_len)
		ret = beada_bulk_in(beada, beada->ep_sl_in,
				    reply, reply_len, SL_DATA_TIMEOUT_MS);
	return ret;
}

/* ------------------------------------------------------------------ */
/* pl_send() — Panel-Link frame transfer                                */
/* ------------------------------------------------------------------ */

/*
 * Caller holds xfer_lock.
 *
 * The device firmware latches the frame dimensions on the FIRST
 * Panel-Link START tag it receives, and refuses to accept any further
 * START tag until the next PL_RESET — a "new" tag would be consumed
 * as 270 bytes of pixel data, shifting the display by ~135 pixels.
 *
 * Consequence: we send START only when @cached_frame_w/h are both
 * zero, which is true after devm_kzalloc() (probe) and after the
 * resume path zeroes them.  Any later mismatch is rejected with
 * -EINVAL to turn a silent corruption into a clean error.
 */
static int pl_send(struct beada_mfd_dev *beada,
		   const u8 *buf, size_t len, u16 frame_w, u16 frame_h)
{
	struct panellink_tag *tag = (struct panellink_tag *)beada->pl_tag_buf;
	char fmtstr[96];
	int ret;

	if (!beada->cached_frame_w && !beada->cached_frame_h) {
		snprintf(fmtstr, sizeof(fmtstr),
			 "image/x-raw, format=BGR16, height=%u, width=%u, framerate=0/1",
			 frame_h, frame_w);

		pl_fill_tag(tag, PL_TYPE_START, fmtstr);
		ret = beada_bulk_out(beada, beada->ep_pl_out,
				     tag, sizeof(*tag), PL_TAG_TIMEOUT_MS);
		if (ret) {
			dev_warn(beada->dev,
				 "Panel-Link START failed: %d\n", ret);
			return ret;
		}

		beada->cached_frame_w = frame_w;
		beada->cached_frame_h = frame_h;
	} else if (frame_w != beada->cached_frame_w ||
		   frame_h != beada->cached_frame_h) {
		dev_err_ratelimited(beada->dev,
			"frame dimensions changed mid-session (%ux%u → %ux%u); "
			"firmware locks dimensions until PL_RESET\n",
			beada->cached_frame_w, beada->cached_frame_h,
			frame_w, frame_h);
		return -EINVAL;
	}

	if (len > PANELLINK_MAX_FRAME_BYTES) {
		dev_err(beada->dev,
			"frame size %zu exceeds buffer %u\n",
			len, PANELLINK_MAX_FRAME_BYTES);
		return -EINVAL;
	}

	memcpy(beada->pl_frame_buf, buf, len);
	return beada_bulk_out(beada, beada->ep_pl_out,
			      beada->pl_frame_buf, len, PL_DATA_TIMEOUT_MS);
}

/* ------------------------------------------------------------------ */
/* xfer worker — runs one frame's USB transfer                          */
/* ------------------------------------------------------------------ */

static void beada_xfer_worker(struct work_struct *work)
{
	struct beada_mfd_dev *beada =
		container_of(work, struct beada_mfd_dev, xfer_work);
	int ret;

	mutex_lock(&beada->xfer_lock);
	ret = beada_check_alive(beada);
	if (!ret)
		ret = pl_send(beada,
			      beada->xfer_src, beada->xfer_len,
			      beada->xfer_w,   beada->xfer_h);
	mutex_unlock(&beada->xfer_lock);

	if (ret && ret != -ENODEV && ret != -EHOSTDOWN)
		dev_warn_ratelimited(beada->dev,
				     "frame transfer failed: %d\n", ret);

	if (ret)
		atomic_inc(&beada->stats_dropped);
	else
		atomic_inc(&beada->stats_displayed);
}

/* ------------------------------------------------------------------ */
/* beada_panellink_reset() — Status-Link PL_RESET                       */
/* ------------------------------------------------------------------ */

int beada_panellink_reset(struct beada_mfd_dev *beada)
{
	struct statuslink_tag *cmd;
	int ret;

	if (!beada->has_statuslink)
		return -EOPNOTSUPP;

	mutex_lock(&beada->xfer_lock);
	ret = beada_check_alive(beada);
	if (ret)
		goto out;

	cmd = (struct statuslink_tag *)beada->sl_buf;
	sl_fill_tag(cmd, SL_TYPE_PL_RESET, sizeof(*cmd));

	ret = beada_bulk_out(beada, beada->ep_sl_out,
			     cmd, sizeof(*cmd), SL_CMD_TIMEOUT_MS);
out:
	mutex_unlock(&beada->xfer_lock);
	return ret;
}

/* ------------------------------------------------------------------ */
/* beada_query_device_info() — probe-time GET_INFO                      */
/* ------------------------------------------------------------------ */

static int beada_try_iproduct(struct beada_mfd_dev *beada)
{
	const struct beada_model_entry *model;
	char buf[128];
	char model_id[8];
	unsigned int w, h;
	int ret;

	ret = usb_string(beada->udev, beada->udev->descriptor.iProduct,
			 buf, sizeof(buf));
	if (ret < 0) {
		dev_dbg(beada->dev, "iProduct read failed: %d\n", ret);
		return ret;
	}

	dev_dbg(beada->dev, "iProduct: \"%s\"\n", buf);

	if (sscanf(buf, "BeadaPanel %s, %ux%u", &model_id[0], &w, &h) != 2)
		return -ENODATA;

	if (!w || !h || w > 4096 || h > 4096) {
		dev_warn(beada->dev,
			 "iProduct implausible resolution %ux%u\n", w, h);
		return -ERANGE;
	}

	model = beada_lookup_model_by_modelid(&model_id[0]);

	if (!model)
		model = beada_lookup_model_by_res((u16)w, (u16)h);

	if (model) {
		beada->panel.width     = model->width;
		beada->panel.height    = model->height;
		beada->panel.width_mm  = model->width_mm;
		beada->panel.height_mm = model->height_mm;
		strscpy(beada->panel.model, model->name,
			sizeof(beada->panel.model));
		dev_info(beada->dev,
			 "panel: BeadaPanel %s detected from iProduct "
			 "(%ux%u px, %ux%u mm)\n",
			 beada->panel.model,
			 beada->panel.width,  beada->panel.height,
			 beada->panel.width_mm, beada->panel.height_mm);
		return 0;
	}

	dev_warn(beada->dev,
		 "%ux%u not in model table; add entry to beada-model.c\n",
		 w, h);
	beada->panel.width     = (u16)w;
	beada->panel.height    = (u16)h;
	beada->panel.width_mm  = 0;
	beada->panel.height_mm = 0;
	strscpy(beada->panel.model, "?", sizeof(beada->panel.model));
	dev_info(beada->dev, "panel: %ux%u px (model unknown)\n", w, h);
	return 0;
}

int beada_query_device_info(struct beada_mfd_dev *beada)
{
	const struct beada_model_entry *model;

	if (beada->has_statuslink) {
		struct statuslink_tag       *cmd = (void *)beada->sl_buf;
		struct statuslink_info_pack *rsp = (void *)beada->sl_buf;
		struct statuslink_info       info;
		int ret;

		sl_fill_tag(cmd, SL_TYPE_GET_INFO, sizeof(*cmd));

		ret = beada_bulk_out(beada, beada->ep_sl_out,
				     cmd, sizeof(*cmd), SL_CMD_TIMEOUT_MS);
		if (ret) {
			dev_warn(beada->dev,
				 "GET_INFO send failed (%d); disabling Status-Link\n",
				 ret);
			beada->has_statuslink = false;
			goto try_iproduct;
		}

		ret = beada_bulk_in(beada, beada->ep_sl_in,
				    rsp, sizeof(*rsp), SL_DATA_TIMEOUT_MS);
		if (ret) {
			dev_warn(beada->dev,
				 "GET_INFO recv failed (%d); disabling Status-Link\n",
				 ret);
			beada->has_statuslink = false;
			goto try_iproduct;
		}

		info = rsp->value;

		beada->firmware_version         = le16_to_cpu(info.firmware_version);
		beada->panellink_version        = info.panellink_version;
		beada->statuslink_version       = info.statuslink_version;
		beada->platform                 = info.hardware_platform;
		beada->panel.max_brightness     = info.max_brightness;
		beada->panel.current_brightness = info.current_brightness;
		beada->storage_size_kb          = le32_to_cpu(info.storage_size);
		memcpy(beada->sn, info.sn, sizeof(info.sn));
		beada->sn[sizeof(info.sn)] = '\0';

		model = beada_lookup_model(info.os_version);
		if (model->os_version == 0xFF)
			dev_warn(beada->dev,
				 "unknown model code 0x%02x; add to beada-model.c\n",
				 info.os_version);

		/*
		 * Optional override of the reported current brightness.
		 * Lets the user pin the initial value the backlight child
		 * driver sees, regardless of what the device happened to
		 * have latched from a previous session.  We touch only
		 * the reported value — the device itself is not changed
		 * here; the backlight child pushes the value to hardware
		 * in its own probe.
		 */
		if (initial_brightness >= 0) {
			if (initial_brightness > info.max_brightness) {
				dev_warn(beada->dev,
					 "initial_brightness=%d exceeds max %u; clamping\n",
					 initial_brightness, info.max_brightness);
				initial_brightness = info.max_brightness;
			}
			beada->panel.current_brightness = (u8)initial_brightness;
		}

		goto apply_model;
	}

try_iproduct:
	if (beada_try_iproduct(beada) == 0)
		return 0;

	model = beada_lookup_model(0xFF);
	dev_warn(beada->dev,
		 "all detection methods failed; defaulting to %ux%u\n",
		 model->width, model->height);

apply_model:
	beada->panel.width     = model->width;
	beada->panel.height    = model->height;
	beada->panel.width_mm  = model->width_mm;
	beada->panel.height_mm = model->height_mm;
	strscpy(beada->panel.model, model->name, sizeof(beada->panel.model));

	dev_info(beada->dev, "panel: BeadaPanel %s, %ux%u px\n",
		 beada->panel.model,
		 beada->panel.width, beada->panel.height);

	if (beada->panel.width_mm && beada->panel.height_mm)
		dev_info(beada->dev,
			 "panel: %ux%u mm, brightness_max=%u\n",
			 beada->panel.width_mm, beada->panel.height_mm,
			 beada->panel.max_brightness);

	return 0;
}

/* ------------------------------------------------------------------ */
/* Public API: panel info                                               */
/* ------------------------------------------------------------------ */

const struct beada_panel_info *beada_get_panel_info(struct beada_mfd_dev *beada)
{
	return &beada->panel;
}
EXPORT_SYMBOL_GPL(beada_get_panel_info);

bool beada_uses_bgr_format(struct beada_mfd_dev *beada)
{
	return beada->pl_bgr_format;
}
EXPORT_SYMBOL_GPL(beada_uses_bgr_format);

/* ------------------------------------------------------------------ */
/* Public API: async frame submission                                   */
/* ------------------------------------------------------------------ */

int beada_send_frame(struct beada_mfd_dev *beada,
		     const void *buf, size_t len,
		     u16 frame_w, u16 frame_h)
{
	if (!frame_w || !frame_h || len != (size_t)frame_w * frame_h * 2) {
		atomic_inc(&beada->stats_dropped);
		return -EINVAL;
	}

	atomic_inc(&beada->stats_submitted);

	/*
	 * The DRM child holds an autopm reference for the lifetime of the
	 * enabled pipe (beada_pipe_active / _idle), so the bus is up when
	 * we reach here.  The caller has also flushed the previous frame's
	 * worker before calling us, so the worker is idle and xfer_src is
	 * free to overwrite.
	 */
	beada->xfer_src = buf;
	beada->xfer_len = len;
	beada->xfer_w   = frame_w;
	beada->xfer_h   = frame_h;

	queue_work(beada->xfer_wq, &beada->xfer_work);
	return 0;
}
EXPORT_SYMBOL_GPL(beada_send_frame);

void beada_flush_frame_transfer(struct beada_mfd_dev *beada)
{
	flush_work(&beada->xfer_work);
}
EXPORT_SYMBOL_GPL(beada_flush_frame_transfer);

/* ------------------------------------------------------------------ */
/* Public API: brightness                                               */
/* ------------------------------------------------------------------ */

int beada_set_brightness(struct beada_mfd_dev *beada, u8 level)
{
	struct statuslink_bl_pack *pkt;
	bool need_autopm;
	int ret;

	if (!beada->has_statuslink)
		return -EOPNOTSUPP;

	/*
	 * Don't wake the device from autosuspend just for a brightness
	 * write.  Rationale:
	 *
	 *   1. The user-visible effect would be invisible anyway — when
	 *      the device is autosuspended, our DRM pipe is closed and
	 *      the panel is showing its internal idle clock; nothing the
	 *      user can see depends on the backlight register value until
	 *      the pipe re-opens.
	 *
	 *   2. The backlight child has already cached the requested value
	 *      in its stored_value before calling us.  When the device
	 *      next wakes (because the pipe re-opens, or because a
	 *      different code path triggers resume), the registered
	 *      resume handler pushes stored_value to hardware.  So the
	 *      write isn't lost — it's just deferred.
	 *
	 *   3. Triggering a resume here would deadlock: the backlight
	 *      class layer holds mb->lock around update_status, the
	 *      autopm_get would synchronously enter our resume callback,
	 *      resume_handlers iteration would invoke the backlight
	 *      handler which tries to acquire mb->lock again — recursive
	 *      mutex acquisition = wedge.
	 *
	 * The check is intentionally lock-free: a torn read can at worst
	 * cause one extra resume (if we read 'false' while suspend is in
	 * flight) or one extra deferred write (if we read 'true' while
	 * resume just completed).  Neither is a correctness problem; the
	 * next brightness write or pipe_enable corrects course.
	 */
	if (READ_ONCE(beada->suspended))
		return 0;

	/*
	 * Skip the autopm pair when recursively called from inside the
	 * resume callback — the bus is already up and a real autopm_get
	 * would deadlock waiting for resume to complete.
	 */
	need_autopm = !READ_ONCE(beada->in_resume);

	if (need_autopm) {
		ret = usb_autopm_get_interface(beada->intf);
		if (ret)
			return ret;
	}

	mutex_lock(&beada->xfer_lock);
	ret = beada_check_alive(beada);
	if (ret)
		goto out;

	pkt = (struct statuslink_bl_pack *)beada->sl_buf;
	pkt->level = level;
	sl_fill_tag(&pkt->header, SL_TYPE_SET_BL, sizeof(*pkt));
	ret = sl_transact(beada, pkt, sizeof(*pkt), NULL, 0);
out:
	mutex_unlock(&beada->xfer_lock);
	if (need_autopm)
		usb_autopm_put_interface(beada->intf);
	return ret;
}
EXPORT_SYMBOL_GPL(beada_set_brightness);

/* ------------------------------------------------------------------ */
/* Public API: clock sync                                               */
/* ------------------------------------------------------------------ */

/*
 * sl_send_time() — Send one Status-Link SET_TIME packet with the given
 * SYSTEMTIME fields.  Caller holds xfer_lock.
 */
static int sl_send_time(struct beada_mfd_dev *beada,
			u16 year, u16 month, u16 dow, u16 day,
			u16 hour, u16 min, u16 sec, u16 msec)
{
	struct statuslink_time_pack *pkt =
		(struct statuslink_time_pack *)beada->sl_buf;

	pkt->time.wYear         = cpu_to_le16(year);
	pkt->time.wMonth        = cpu_to_le16(month);
	pkt->time.wDayOfWeek    = cpu_to_le16(dow);
	pkt->time.wDay          = cpu_to_le16(day);
	pkt->time.wHour         = cpu_to_le16(hour);
	pkt->time.wMinute       = cpu_to_le16(min);
	pkt->time.wSecond       = cpu_to_le16(sec);
	pkt->time.wMilliseconds = cpu_to_le16(msec);
	sl_fill_tag(&pkt->header, SL_TYPE_SET_TIME, sizeof(*pkt));

	return sl_transact(beada, pkt, sizeof(*pkt), NULL, 0);
}

int beada_sync_time(struct beada_mfd_dev *beada)
{
	struct timespec64 ts;
	struct tm t;
	s64 offset_sec;
	bool need_autopm;
	bool suspended;
	int ret;

	if (!beada->has_statuslink)
		return -EOPNOTSUPP;

	offset_sec = (s64)tz_offset * 60;
	if (tz_dst)
		offset_sec += 3600;

	ktime_get_real_ts64(&ts);
	time64_to_tm(ts.tv_sec, offset_sec, &t);

	suspended = READ_ONCE(beada->suspended);
	need_autopm = !READ_ONCE(beada->in_resume);

	/*
	 * Clock writes are special: the device's standby clock is what
	 * the user is looking at while the device is autosuspended.
	 * Updating it must NOT wake the bus (that would replace the
	 * visible clock with a black frame), but it MUST actually go
	 * out to the device.
	 *
	 * usb_autopm_get_interface_no_resume() bumps the autopm refcount
	 * without triggering a resume.  If the device is suspended the
	 * subsequent bulk transfer will fail with -EHOSTDOWN — and the
	 * suspended check in beada_check_alive() does exactly that.  So
	 * for the suspended-clock case this path is harmless: it bumps
	 * a refcount, fails the transfer, and releases the refcount.
	 *
	 * That fails the transfer.  Which means the on-screen clock
	 * doesn't actually update in the suspended state — exactly the
	 * thing we wanted to avoid.
	 *
	 * The firmware doesn't expose a separate "update clock without
	 * waking" path: SL_SET_TIME always goes through the same USB
	 * bulk-OUT endpoint that suspend has taken down.  So there's no
	 * way around it — to write the clock we have to have the bus up.
	 *
	 * Resolution: return -EHOSTDOWN to userspace.  Sysfs users can
	 * retry after triggering a brief activity (opening a DRM client,
	 * adjusting brightness when the pipe is open, etc.).  This is
	 * the least surprising behaviour given the hardware constraint.
	 */
	if (suspended)
		return -EHOSTDOWN;

	if (need_autopm) {
		ret = usb_autopm_get_interface(beada->intf);
		if (ret)
			return ret;
	}

	mutex_lock(&beada->xfer_lock);
	ret = beada_check_alive(beada);
	if (ret)
		goto out;

	ret = sl_send_time(beada,
			   t.tm_year + 1900,
			   t.tm_mon + 1,
			   t.tm_wday,
			   t.tm_mday,
			   t.tm_hour,
			   t.tm_min,
			   t.tm_sec,
			   ts.tv_nsec / 1000000);

	if (!ret)
		dev_dbg(beada->dev,
			"clock synced: %04u-%02u-%02u %02u:%02u:%02u "
			"(UTC%+d min%s)\n",
			(u32)(t.tm_year + 1900), t.tm_mon + 1, t.tm_mday,
			t.tm_hour, t.tm_min, t.tm_sec,
			tz_offset + (tz_dst ? 60 : 0),
			tz_dst ? ", DST" : "");
out:
	mutex_unlock(&beada->xfer_lock);
	if (need_autopm)
		usb_autopm_put_interface(beada->intf);
	return ret;
}

/* ------------------------------------------------------------------ */
/* Pipeline init                                                        */
/* ------------------------------------------------------------------ */

static void beada_destroy_workqueue(void *data)
{
	struct workqueue_struct *wq = data;

	destroy_workqueue(wq);
}

int beada_pipeline_init(struct beada_mfd_dev *beada)
{
	int ret;

	beada->xfer_wq = alloc_ordered_workqueue("beada-mfd/%s",
						 WQ_MEM_RECLAIM,
						 dev_name(beada->dev));
	if (!beada->xfer_wq)
		return -ENOMEM;

	ret = devm_add_action_or_reset(beada->dev,
				       beada_destroy_workqueue,
				       beada->xfer_wq);
	if (ret)
		return ret;

	INIT_WORK(&beada->xfer_work, beada_xfer_worker);
	INIT_DELAYED_WORK(&beada->resume_work, beada_resume_handlers_work);
	return 0;
}
