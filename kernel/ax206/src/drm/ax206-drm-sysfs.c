// SPDX-License-Identifier: GPL-2.0-only
/*
 * ax206-drm-sysfs.c - AX206 DRM sysfs attribute interface
 *
 * Exposes the following attributes under the DRM device:
 *
 *   rotation          R/W  current rotation mode (0-7)
 *   noblank           R/W  keep last image when pipe closes (0/1)
 *   stats             R    frame statistics (received/drawn/skipped/errors/fps)
 *   reset_stats       W    write any value to reset statistics
 *
 * Virtual power management attributes (idle timer attributes):
 *
 *   idle_control    R/W  "auto" enables idle timer, "on" forces display on
 *   idle_timeout_ms R/W  idle timeout in ms (0 = disable)
 *   idle_status     R    "active" or "suspended"
 *
 * Copyright (C) 2025 Oskari Rauta <oskari.rauta@gmail.com>
 */

#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/sysfs.h>
#include <linux/string.h>
#include <linux/timer.h>

#include "ax206.h"
#include "ax206-drm-priv.h"

/* Helper: retrieve ax206_drm_dev from a drm sub-device */
static struct ax206_drm_dev *sysfs_to_ax206_drm(struct device *dev)
{
	struct drm_device *drm = dev_get_drvdata(dev);

	return to_ax206_drm(drm);
}

/* ----- rotation ----- */

static ssize_t rotation_show(struct device *dev,
			     struct device_attribute *attr, char *buf)
{
	struct ax206_drm_dev *d = sysfs_to_ax206_drm(dev);

	return sysfs_emit(buf, "%d (%s)\n",
			  d->rotation, ax206_rotation_name(d->rotation));
}

static ssize_t rotation_store(struct device *dev,
			      struct device_attribute *attr,
			      const char *buf, size_t count)
{
	struct ax206_drm_dev *d = sysfs_to_ax206_drm(dev);
	int val;
	int ret;

	ret = kstrtoint(buf, 10, &val);
	if (ret)
		return ret;

	if (val < 0 || val >= AX206_ROTATE_COUNT)
		return -EINVAL;

	ax206_drm_set_rotation(d, val);
	/* Note: mode reconfiguration would need a full KMS reset;
	 * rotation change takes effect on the next frame. */
	return count;
}

static DEVICE_ATTR_RW(rotation);

/* ----- noblank ----- */

static ssize_t noblank_show(struct device *dev,
			    struct device_attribute *attr, char *buf)
{
	struct ax206_drm_dev *d = sysfs_to_ax206_drm(dev);

	return sysfs_emit(buf, "%d\n", d->noblank ? 1 : 0);
}

static ssize_t noblank_store(struct device *dev,
			     struct device_attribute *attr,
			     const char *buf, size_t count)
{
	struct ax206_drm_dev *d = sysfs_to_ax206_drm(dev);
	bool val;
	int ret;

	ret = kstrtobool(buf, &val);
	if (ret)
		return ret;

	d->noblank = val;
	return count;
}

static DEVICE_ATTR_RW(noblank);

/* ----- stats ----- */

static ssize_t stats_show(struct device *dev,
			  struct device_attribute *attr, char *buf)
{
	struct ax206_drm_dev *d = sysfs_to_ax206_drm(dev);

	return ax206_stats_show(&d->stats, buf);
}

static DEVICE_ATTR_RO(stats);

/* ----- reset_stats ----- */

static ssize_t reset_stats_store(struct device *dev,
				 struct device_attribute *attr,
				 const char *buf, size_t count)
{
	struct ax206_drm_dev *d = sysfs_to_ax206_drm(dev);

	ax206_stats_reset(&d->stats);
	return count;
}

static DEVICE_ATTR_WO(reset_stats);

/* ----- power/control (virtual) ----- */

static ssize_t power_control_show(struct device *dev,
				  struct device_attribute *attr, char *buf)
{
	struct ax206_drm_dev *d = sysfs_to_ax206_drm(dev);

	return sysfs_emit(buf, "%s\n",
			  d->idle_timeout_s ? "auto" : "on");
}

static ssize_t power_control_store(struct device *dev,
				   struct device_attribute *attr,
				   const char *buf, size_t count)
{
	struct ax206_drm_dev *d = sysfs_to_ax206_drm(dev);
	char val[8];
	size_t len;

	len = min(count, sizeof(val) - 1);
	memcpy(val, buf, len);
	val[len] = '\0';
	/* strip trailing newline */
	if (len && val[len - 1] == '\n')
		val[len - 1] = '\0';

	if (strcmp(val, "on") == 0) {
		/* Force display on, disable idle timer */
		d->idle_timeout_s = 0;
		timer_delete_sync(&d->idle_timer);
		if (d->idle_active)
			queue_work(d->dpf->wq, &d->resume_work);
	} else if (strcmp(val, "auto") == 0) {
		/* Enable idle timer with current timeout */
		if (!d->idle_timeout_s)
			d->idle_timeout_s = 60; /* sane default */
		ax206_idle_timer_reset(d);
	} else {
		return -EINVAL;
	}

	return count;
}

static DEVICE_ATTR_RW(power_control);

/* ----- power/autosuspend_delay_ms ----- */

static ssize_t power_autosuspend_delay_ms_show(struct device *dev,
					       struct device_attribute *attr,
					       char *buf)
{
	struct ax206_drm_dev *d = sysfs_to_ax206_drm(dev);

	return sysfs_emit(buf, "%u\n", d->idle_timeout_s * 1000);
}

static ssize_t power_autosuspend_delay_ms_store(struct device *dev,
						struct device_attribute *attr,
						const char *buf, size_t count)
{
	struct ax206_drm_dev *d = sysfs_to_ax206_drm(dev);
	unsigned int ms;
	int ret;

	ret = kstrtouint(buf, 10, &ms);
	if (ret)
		return ret;

	d->idle_timeout_s = ms / 1000;

	if (d->idle_timeout_s == 0) {
		timer_delete_sync(&d->idle_timer);
	} else {
		if (d->idle_active)
			queue_work(d->dpf->wq, &d->resume_work);
		ax206_idle_timer_reset(d);
	}

	return count;
}

static DEVICE_ATTR_RW(power_autosuspend_delay_ms);

/* ----- power/runtime_status ----- */

static ssize_t power_runtime_status_show(struct device *dev,
					 struct device_attribute *attr,
					 char *buf)
{
	struct ax206_drm_dev *d = sysfs_to_ax206_drm(dev);

	return sysfs_emit(buf, "%s\n",
			  d->idle_active ? "suspended" : "active");
}

static DEVICE_ATTR_RO(power_runtime_status);

/* ----- attribute groups ----- */

static struct attribute *ax206_drm_attrs[] = {
	&dev_attr_rotation.attr,
	&dev_attr_noblank.attr,
	&dev_attr_stats.attr,
	&dev_attr_reset_stats.attr,
	NULL,
};

static struct attribute *ax206_drm_power_attrs[] = {
	&dev_attr_power_control.attr,
	&dev_attr_power_autosuspend_delay_ms.attr,
	&dev_attr_power_runtime_status.attr,
	NULL,
};

static const struct attribute_group ax206_drm_attr_group = {
	.attrs = ax206_drm_attrs,
};

static const struct attribute_group ax206_drm_power_attr_group = {
	.name  = "virtual_power",
	.attrs = ax206_drm_power_attrs,
};

static const struct attribute_group *ax206_drm_attr_groups[] = {
	&ax206_drm_attr_group,
	&ax206_drm_power_attr_group,
	NULL,
};

int ax206_drm_sysfs_init(struct ax206_drm_dev *d)
{
	return sysfs_create_groups(&d->drm.dev->kobj, ax206_drm_attr_groups);
}

void ax206_drm_sysfs_fini(struct ax206_drm_dev *d)
{
	sysfs_remove_groups(&d->drm.dev->kobj, ax206_drm_attr_groups);
}
