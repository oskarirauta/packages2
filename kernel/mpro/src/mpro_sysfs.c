// SPDX-License-Identifier: GPL-2.0
/*
 * mpro_sysfs.c — parent-level sysfs attributes.
 *
 * Exposes device identity (model, screen_id, version, etc.) and
 * pipeline tunables (lz4_level, lz4_threshold, stats) under the
 * parent's sysfs.
 *
 * Per-child attributes (rotation, brightness, gamma) live in the
 * respective child driver's sysfs.
 */

#include <linux/module.h>
#include <linux/lz4.h>

#include "mpro_internal.h"
#include "mpro.h"

static ssize_t model_show(struct device *dev, struct device_attribute *attr,
			  char *buf)
{
	struct mpro_device *mpro = dev_get_drvdata(dev);

	if (!mpro->model)
		return sysfs_emit(buf, "unknown\n");
	return sysfs_emit(buf, "%s\n", mpro->model->name);
}
static DEVICE_ATTR_RO(model);

static ssize_t description_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct mpro_device *mpro = dev_get_drvdata(dev);

	if (!mpro->model)
		return sysfs_emit(buf, "unknown\n");
	return sysfs_emit(buf, "%s\n", mpro->model->description);
}
static DEVICE_ATTR_RO(description);

static ssize_t resolution_show(struct device *dev,
			       struct device_attribute *attr, char *buf)
{
	struct mpro_device *mpro = dev_get_drvdata(dev);

	if (!mpro->model)
		return sysfs_emit(buf, "unknown\n");
	return sysfs_emit(buf, "%u %u\n",
			  mpro->model->width, mpro->model->height);
}
static DEVICE_ATTR_RO(resolution);

static ssize_t physical_size_show(struct device *dev,
				  struct device_attribute *attr, char *buf)
{
	struct mpro_device *mpro = dev_get_drvdata(dev);

	if (!mpro->model)
		return sysfs_emit(buf, "unknown\n");
	return sysfs_emit(buf, "%u %u\n",
			  mpro->model->width_mm, mpro->model->height_mm);
}
static DEVICE_ATTR_RO(physical_size);

static ssize_t width_mm_show(struct device *dev,
			     struct device_attribute *attr, char *buf)
{
	struct mpro_device *mpro = dev_get_drvdata(dev);

	if (!mpro->model)
		return sysfs_emit(buf, "-1\n");
	return sysfs_emit(buf, "%u\n", mpro->model->width_mm);
}
static DEVICE_ATTR_RO(width_mm);

static ssize_t height_mm_show(struct device *dev,
			      struct device_attribute *attr, char *buf)
{
	struct mpro_device *mpro = dev_get_drvdata(dev);

	if (!mpro->model)
		return sysfs_emit(buf, "-1\n");
	return sysfs_emit(buf, "%u\n", mpro->model->height_mm);
}
static DEVICE_ATTR_RO(height_mm);

static ssize_t version_id_show(struct device *dev,
			       struct device_attribute *attr, char *buf)
{
	struct mpro_device *mpro = dev_get_drvdata(dev);

	return sysfs_emit(buf, "0x%08x\n", mpro->version);
}
static DEVICE_ATTR_RO(version_id);

static ssize_t screen_id_show(struct device *dev,
			      struct device_attribute *attr, char *buf)
{
	struct mpro_device *mpro = dev_get_drvdata(dev);

	return sysfs_emit(buf, "0x%08x\n", mpro->screen);
}
static DEVICE_ATTR_RO(screen_id);

static ssize_t margin_show(struct device *dev,
			   struct device_attribute *attr, char *buf)
{
	struct mpro_device *mpro = dev_get_drvdata(dev);

	if (!mpro->model)
		return sysfs_emit(buf, "-1\n");
	return sysfs_emit(buf, "%u\n", mpro->model->margin);
}
static DEVICE_ATTR_RO(margin);

static ssize_t device_id_show(struct device *dev,
			      struct device_attribute *attr, char *buf)
{
	struct mpro_device *mpro = dev_get_drvdata(dev);

	return sysfs_emit(buf, "%*phC\n", (int)sizeof(mpro->id), mpro->id);
}
static DEVICE_ATTR_RO(device_id);

static ssize_t fbdev_enabled_show(struct device *dev,
				  struct device_attribute *attr, char *buf)
{
	struct mpro_device *mpro = dev_get_drvdata(dev);

	return sysfs_emit(buf, "%u\n", mpro->fbdev_enabled ? 1 : 0);
}
static DEVICE_ATTR_RO(fbdev_enabled);

static ssize_t lz4_level_show(struct device *dev,
			      struct device_attribute *attr, char *buf)
{
	struct mpro_device *mpro = dev_get_drvdata(dev);

	if (mpro->model && mpro->model->margin > 0)
		return sysfs_emit(buf, "unsupported (margin device)\n");
	if (!mpro_firmware_supports_lz4(mpro))
		return sysfs_emit(buf, "unsupported (firmware too old)\n");

	return sysfs_emit(buf, "%d\n", READ_ONCE(mpro->lz4_level));
}

static ssize_t lz4_level_store(struct device *dev,
			       struct device_attribute *attr,
			       const char *buf, size_t count)
{
	struct mpro_device *mpro = dev_get_drvdata(dev);
	int val;
	int ret;

	if (kstrtoint(buf, 10, &val) || val < 0 || val > 12)
		return -EINVAL;

	if (val > 0) {
		if (mpro->model && mpro->model->margin > 0) {
			dev_warn(&mpro->intf->dev,
				 "LZ4 is not available on margin device\n");
			return -EOPNOTSUPP;
		}

		if (!mpro_firmware_supports_lz4(mpro)) {
			dev_warn(&mpro->intf->dev,
				 "LZ4 requires firmware >= %d.%d (have %d.%d)\n",
				 MPRO_LZ4_MIN_MAJOR, MPRO_LZ4_MIN_MINOR,
				 mpro->fw_major, mpro->fw_minor);
			return -EOPNOTSUPP;
		}

		ret = mpro_lz4_workmem_alloc(mpro);
		if (ret)
			return ret;
	}

	WRITE_ONCE(mpro->lz4_level, val);
	return count;
}
static DEVICE_ATTR_RW(lz4_level);

static ssize_t lz4_threshold_show(struct device *dev,
				  struct device_attribute *attr, char *buf)
{
	struct mpro_device *mpro = dev_get_drvdata(dev);

	return sysfs_emit(buf, "%d\n", READ_ONCE(mpro->lz4_threshold));
}

static ssize_t lz4_threshold_store(struct device *dev,
				   struct device_attribute *attr,
				   const char *buf, size_t count)
{
	struct mpro_device *mpro = dev_get_drvdata(dev);
	int val;

	if (kstrtoint(buf, 10, &val) || val < 0)
		return -EINVAL;

	WRITE_ONCE(mpro->lz4_threshold, val);
	return count;
}
static DEVICE_ATTR_RW(lz4_threshold);

static ssize_t firmware_show(struct device *dev,
			     struct device_attribute *attr, char *buf)
{
	struct mpro_device *mpro = dev_get_drvdata(dev);

	return sysfs_emit(buf, "%s\n", mpro->fw_string);
}
static DEVICE_ATTR_RO(firmware);

static ssize_t fw_minor_show(struct device *dev,
			     struct device_attribute *attr, char *buf)
{
	struct mpro_device *mpro = dev_get_drvdata(dev);

	return sysfs_emit(buf, "%d\n", mpro->fw_minor);
}
static DEVICE_ATTR_RO(fw_minor);

static ssize_t fw_major_show(struct device *dev,
			     struct device_attribute *attr, char *buf)
{
	struct mpro_device *mpro = dev_get_drvdata(dev);

	return sysfs_emit(buf, "%d\n", mpro->fw_major);
}
static DEVICE_ATTR_RO(fw_major);

/* ------------------------------------------------------------------ */
/* Idle / power-management attributes                                 */
/* ------------------------------------------------------------------ */

static ssize_t idle_delay_ms_show(struct device *dev,
				  struct device_attribute *attr, char *buf)
{
	struct mpro_device *mpro = dev_get_drvdata(dev);

	return sysfs_emit(buf, "%u\n", READ_ONCE(mpro->idle_delay_ms));
}

static ssize_t idle_delay_ms_store(struct device *dev,
				   struct device_attribute *attr,
				   const char *buf, size_t count)
{
	struct mpro_device *mpro = dev_get_drvdata(dev);
	unsigned int val;

	if (kstrtouint(buf, 10, &val))
		return -EINVAL;

	WRITE_ONCE(mpro->idle_delay_ms, val);
	return count;
}
static DEVICE_ATTR_RW(idle_delay_ms);

static ssize_t idle_state_show(struct device *dev,
			       struct device_attribute *attr, char *buf)
{
	struct mpro_device *mpro = dev_get_drvdata(dev);
	bool is_idle;

	mutex_lock(&mpro->listeners_lock);
	is_idle = mpro->is_idle;
	mutex_unlock(&mpro->listeners_lock);

	return sysfs_emit(buf, "%s\n", is_idle ? "idle" : "active");
}

static ssize_t idle_state_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{
	struct mpro_device *mpro = dev_get_drvdata(dev);
	bool val;

	if (sysfs_streq(buf, "idle")) {
		mpro_pm_force_idle(mpro);
	} else if (sysfs_streq(buf, "active")) {
		mpro_pm_force_active(mpro);
	} else if (!kstrtobool(buf, &val)) {
		/*
		 * Numeric form: 1 = active, 0 = idle. Also accepts
		 * "y"/"n", "true"/"false" via kstrtobool.
		 */
		if (val)
			mpro_pm_force_active(mpro);
		else
			mpro_pm_force_idle(mpro);
	} else {
		return -EINVAL;
	}

	return count;
}
static DEVICE_ATTR_RW(idle_state);

static ssize_t active_refs_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct mpro_device *mpro = dev_get_drvdata(dev);

	return sysfs_emit(buf, "%d\n", atomic_read(&mpro->active_refs));
}
static DEVICE_ATTR_RO(active_refs);

static ssize_t touch_wake_show(struct device *dev,
			       struct device_attribute *attr, char *buf)
{
	struct mpro_device *mpro = dev_get_drvdata(dev);

	return sysfs_emit(buf, "%u\n", READ_ONCE(mpro->touch_wake_enabled));
}

static ssize_t touch_wake_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{
	struct mpro_device *mpro = dev_get_drvdata(dev);
	bool val;

	if (kstrtobool(buf, &val))
		return -EINVAL;

	WRITE_ONCE(mpro->touch_wake_enabled, val);

	/*
	 * If the user disables touch-wake while the display is
	 * currently idle, stop the touchscreen URB pipeline so the
	 * device can fully idle. The screen-state listener path
	 * handles this — re-notify off so touchscreen reacts.
	 *
	 * If the user enables touch-wake, do nothing here: the
	 * change takes effect on the next idle cycle. The current
	 * idle session won't restart the URB, but that's fine —
	 * users can echo "active" to idle_state to reset.
	 */
	if (!val) {
		mutex_lock(&mpro->listeners_lock);
		if (mpro->is_idle) {
			mutex_unlock(&mpro->listeners_lock);
			/*
			 * Re-fire screen_off so touchscreen sees the
			 * new touch_wake_enabled value and stops its
			 * URB. Backlight is already off.
			 */
			mpro_screen_notify_off(mpro);
		} else {
			mutex_unlock(&mpro->listeners_lock);
		}
	}

	return count;
}
static DEVICE_ATTR_RW(touch_wake);

static ssize_t pm_stats_show(struct device *dev,
			     struct device_attribute *attr, char *buf)
{
	struct mpro_device *mpro = dev_get_drvdata(dev);
	u64 now_ns, active_ns, idle_ns, current_ns;
	u32 idle_count, wake_count, touch_wake_count;
	bool is_idle;

	mutex_lock(&mpro->listeners_lock);
	now_ns = ktime_get_ns();
	is_idle = mpro->is_idle;
	active_ns = mpro->pm_active_total_ns;
	idle_ns = mpro->pm_idle_total_ns;
	current_ns = now_ns - mpro->pm_state_changed_ns;
	idle_count = mpro->pm_idle_count;
	wake_count = mpro->pm_wake_count;
	touch_wake_count = mpro->pm_touch_wake_count;
	mutex_unlock(&mpro->listeners_lock);

	return sysfs_emit(buf,
		"state=%s\n"
		"active_time_ms=%llu\n"
		"idle_time_ms=%llu\n"
		"current_state_time_ms=%llu\n"
		"idle_transitions=%u\n"
		"wake_transitions=%u\n"
		"touch_wakes=%u\n",
		is_idle ? "idle" : "active",
		div_u64(active_ns, NSEC_PER_MSEC),
		div_u64(idle_ns, NSEC_PER_MSEC),
		div_u64(current_ns, NSEC_PER_MSEC),
		idle_count,
		wake_count,
		touch_wake_count);
}
static DEVICE_ATTR_RO(pm_stats);

static ssize_t reset_pm_stats_store(struct device *dev,
				    struct device_attribute *attr,
				    const char *buf, size_t count)
{
	struct mpro_device *mpro = dev_get_drvdata(dev);
	bool val;

	if (kstrtobool(buf, &val))
		return -EINVAL;

	if (val) {
		mutex_lock(&mpro->listeners_lock);
		mpro->pm_active_total_ns = 0;
		mpro->pm_idle_total_ns = 0;
		mpro->pm_state_changed_ns = ktime_get_ns();
		mpro->pm_idle_count = 0;
		mpro->pm_wake_count = 0;
		mpro->pm_touch_wake_count = 0;
		mutex_unlock(&mpro->listeners_lock);
	}

	return count;
}
static DEVICE_ATTR_WO(reset_pm_stats);

static ssize_t fps_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct mpro_device *mpro = dev_get_drvdata(dev);
	u64 now_ns, last_ns;
	u32 period_ns;
	u64 fps_x100;
	unsigned long flags;

	/*
	 * The EWMA period is only updated when a frame completes. If no
	 * frame has been sent for over 2 seconds the value is stale, so
	 * report 0 instead of a frozen "last seen" rate.
	 */
	last_ns = atomic64_read(&mpro->last_frame_ns);
	if (!last_ns)
		return sysfs_emit(buf, "0.00\n");

	now_ns = ktime_get_ns();
	if (now_ns - last_ns > 2ULL * NSEC_PER_SEC)
		return sysfs_emit(buf, "0.00\n");

	spin_lock_irqsave(&mpro->fps_lock, flags);
	period_ns = mpro->ewma_period_ns;
	spin_unlock_irqrestore(&mpro->fps_lock, flags);

	if (!period_ns)
		return sysfs_emit(buf, "0.00\n");

	fps_x100 = div_u64((u64)NSEC_PER_SEC * 100, period_ns);
	return sysfs_emit(buf, "%llu.%02llu\n",
			  fps_x100 / 100, fps_x100 % 100);
}
static DEVICE_ATTR_RO(fps);

static ssize_t stats_show(struct device *dev,
			  struct device_attribute *attr, char *buf)
{
	struct mpro_device *mpro = dev_get_drvdata(dev);
	u32 submitted, displayed, dropped;
	u32 efficiency = 0;
	u32 period_ns;
	u32 fps_x100 = 0;
	u64 now_ns, last_ns;
	unsigned long flags;

	submitted = atomic_read(&mpro->stats_submitted);
	displayed = atomic_read(&mpro->stats_displayed);
	dropped   = atomic_read(&mpro->stats_dropped);

	if (submitted > 0)
		efficiency = (u32)div_u64((u64)displayed * 10000, submitted);

	/* Same staleness check as fps_show() */
	last_ns = atomic64_read(&mpro->last_frame_ns);
	if (last_ns) {
		now_ns = ktime_get_ns();
		if (now_ns - last_ns <= 2ULL * NSEC_PER_SEC) {
			spin_lock_irqsave(&mpro->fps_lock, flags);
			period_ns = mpro->ewma_period_ns;
			spin_unlock_irqrestore(&mpro->fps_lock, flags);

			if (period_ns)
				fps_x100 = (u32)div_u64((u64)NSEC_PER_SEC * 100,
							period_ns);
		}
	}

	return sysfs_emit(buf,
			  "submitted=%u displayed=%u dropped=%u "
			  "fps=%u.%02u efficiency=%u.%02u%%\n",
			  submitted, displayed, dropped,
			  fps_x100 / 100, fps_x100 % 100,
			  efficiency / 100, efficiency % 100);
}
static DEVICE_ATTR_RO(stats);

static ssize_t reset_stats_store(struct device *dev,
				 struct device_attribute *attr,
				 const char *buf, size_t count)
{
	struct mpro_device *mpro = dev_get_drvdata(dev);
	bool val;

	if (kstrtobool(buf, &val))
		return -EINVAL;

	if (val) {
		atomic_set(&mpro->stats_submitted, 0);
		atomic_set(&mpro->stats_displayed, 0);
		atomic_set(&mpro->stats_dropped, 0);

		/* Clear the EWMA state too — otherwise fps would carry over */
		atomic64_set(&mpro->last_frame_ns, 0);
		spin_lock(&mpro->fps_lock);
		mpro->ewma_period_ns = 0;
		spin_unlock(&mpro->fps_lock);
	}

	return count;
}
static DEVICE_ATTR_WO(reset_stats);

static struct attribute *mpro_attrs[] = {
	&dev_attr_model.attr,
	&dev_attr_description.attr,
	&dev_attr_resolution.attr,
	&dev_attr_physical_size.attr,
	&dev_attr_width_mm.attr,
	&dev_attr_height_mm.attr,
	&dev_attr_version_id.attr,
	&dev_attr_screen_id.attr,
	&dev_attr_device_id.attr,
	&dev_attr_margin.attr,
	&dev_attr_fbdev_enabled.attr,
	&dev_attr_lz4_level.attr,
	&dev_attr_lz4_threshold.attr,
	&dev_attr_firmware.attr,
	&dev_attr_fw_minor.attr,
	&dev_attr_fw_major.attr,
	&dev_attr_idle_delay_ms.attr,
	&dev_attr_idle_state.attr,
	&dev_attr_active_refs.attr,
	&dev_attr_touch_wake.attr,
	&dev_attr_pm_stats.attr,
	&dev_attr_reset_pm_stats.attr,
	&dev_attr_fps.attr,
	&dev_attr_stats.attr,
	&dev_attr_reset_stats.attr,
	NULL,
};

static const struct attribute_group mpro_attr_group = {
	.attrs = mpro_attrs,
};

int mpro_sysfs_add(struct mpro_device *mpro)
{
	return devm_device_add_group(&mpro->intf->dev, &mpro_attr_group);
}
