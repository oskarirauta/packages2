// SPDX-License-Identifier: GPL-2.0
/*
 * mpro_drm_sysfs.c — DRM-side sysfs attributes.
 *
 * Per-device attributes:
 *   rotation         — display orientation (0..7, see mpro_drm__rotation_map)
 *   brightness       — software brightness (0..100)
 *   gamma            — gamma correction (0.50..4.00)
 *   disable_partial  — force full-frame updates
 *   gamma_lut        — binary attribute, raw 768-byte LUT (3*256)
 *
 * mpro_drm__apply_rotation() also lives here, since rotation changes
 * are triggered through this sysfs attribute.
 */

#include <linux/module.h>
#include <linux/sysfs.h>
#include <linux/string.h>

#include <drm/drm_blend.h>
#include <drm/drm_probe_helper.h>

#include "mpro_drm.h"

/* ------------------------------------------------------------------ */
/* Rotation map                                                       */
/* ------------------------------------------------------------------ */

static const u16 mpro_drm__rotation_map[] = {
	DRM_MODE_ROTATE_0,
	DRM_MODE_ROTATE_90,
	DRM_MODE_ROTATE_180,
	DRM_MODE_ROTATE_270,
	DRM_MODE_ROTATE_0   | DRM_MODE_REFLECT_X,
	DRM_MODE_ROTATE_90  | DRM_MODE_REFLECT_X,
	DRM_MODE_ROTATE_180 | DRM_MODE_REFLECT_X,
	DRM_MODE_ROTATE_270 | DRM_MODE_REFLECT_X,
};

void mpro_drm__apply_rotation(struct mpro_drm *mdrm, u16 new_rotation)
{
	struct drm_device *drm = &mdrm->drm;
	bool swap = mpro_drm__rotation_swaps_dims(new_rotation);
	u32 base_w = mdrm->mpro->model->width;
	u32 base_h = mdrm->mpro->model->height;

	if (READ_ONCE(mdrm->native_rotation) == new_rotation)
		return;

	WRITE_ONCE(mdrm->native_rotation, new_rotation);
	mdrm->width    = swap ? base_h : base_w;
	mdrm->height   = swap ? base_w : base_h;

	mutex_lock(&drm->mode_config.mutex);
	drm->mode_config.min_width  = mdrm->width;
	drm->mode_config.max_width  = mdrm->width;
	drm->mode_config.min_height = mdrm->height;
	drm->mode_config.max_height = mdrm->height;
	mutex_unlock(&drm->mode_config.mutex);

	if (mdrm->data)
		memset(mdrm->data, 0, mdrm->data_size);
	mpro_drm__request_update(mdrm, 0, 0, mdrm->width, mdrm->height, false);

	drm_kms_helper_hotplug_event(drm);
}

/* ------------------------------------------------------------------ */
/* gamma_lut binary attribute                                         */
/* ------------------------------------------------------------------ */

static ssize_t gamma_lut_write(struct file *f, struct kobject *kobj,
			       const struct bin_attribute *a, char *buf,
			       loff_t off, size_t count)
{
	struct mpro_drm *mdrm = dev_get_drvdata(kobj_to_dev(kobj));

	if (off != 0 || count != MPRO_DRM_LUT_SIZE)
		return -EINVAL;

	mutex_lock(&mdrm->lut_lock);
	memcpy(mdrm->lut[0], buf,       256);
	memcpy(mdrm->lut[1], buf + 256, 256);
	memcpy(mdrm->lut[2], buf + 512, 256);
	mdrm->gamma_valid = true;
	mpro_drm__rebuild_combined_lut(mdrm);
	mutex_unlock(&mdrm->lut_lock);

	return count;
}
static const BIN_ATTR_WO(gamma_lut, MPRO_DRM_LUT_SIZE);

static const struct bin_attribute *mpro_drm__bin_attrs[] = {
	&bin_attr_gamma_lut,
	NULL,
};

/* ------------------------------------------------------------------ */
/* Text attributes                                                    */
/* ------------------------------------------------------------------ */

static ssize_t rotation_show(struct device *dev,
			     struct device_attribute *attr, char *buf)
{
	struct mpro_drm *mdrm = dev_get_drvdata(dev);
	u16 effective = READ_ONCE(mdrm->native_rotation);
	u32 i;

	for (i = 0; i < ARRAY_SIZE(mpro_drm__rotation_map); i++)
		if (effective == mpro_drm__rotation_map[i])
			return sysfs_emit(buf, "%u\n", i);
	return sysfs_emit(buf, "0\n");
}

static ssize_t rotation_store(struct device *dev,
			      struct device_attribute *attr,
			      const char *buf, size_t count)
{
	struct mpro_drm *mdrm = dev_get_drvdata(dev);
	u32 val;

	if (kstrtou32(buf, 10, &val) ||
	    val >= ARRAY_SIZE(mpro_drm__rotation_map))
		return -EINVAL;

	mpro_drm__apply_rotation(mdrm, mpro_drm__rotation_map[val]);
	return count;
}
static DEVICE_ATTR_RW(rotation);

static ssize_t brightness_show(struct device *dev,
			       struct device_attribute *attr, char *buf)
{
	struct mpro_drm *mdrm = dev_get_drvdata(dev);

	return sysfs_emit(buf, "%u\n", mdrm->brightness);
}

static ssize_t brightness_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{
	struct mpro_drm *mdrm = dev_get_drvdata(dev);
	u32 val;

	if (kstrtou32(buf, 10, &val))
		return -EINVAL;
	if (val > MPRO_BRIGHTNESS_MAX)
		return -EINVAL;

	mutex_lock(&mdrm->lut_lock);
	mdrm->brightness = val;
	mpro_drm__rebuild_combined_lut(mdrm);
	mutex_unlock(&mdrm->lut_lock);

	return count;
}
static DEVICE_ATTR_RW(brightness);

static ssize_t gamma_show(struct device *dev,
			  struct device_attribute *attr, char *buf)
{
	struct mpro_drm *mdrm = dev_get_drvdata(dev);

	return sysfs_emit(buf, "%u.%02u\n",
			  mdrm->gamma_x100 / 100, mdrm->gamma_x100 % 100);
}

static ssize_t gamma_store(struct device *dev,
			   struct device_attribute *attr,
			   const char *buf, size_t count)
{
	struct mpro_drm *mdrm = dev_get_drvdata(dev);
	u32 new_gamma;
	int ret;

	ret = mpro_parse_gamma_x100(buf, &new_gamma);
	if (ret)
		return ret;

	if (new_gamma < 50 || new_gamma > 400)
		return -EINVAL;

	mdrm->gamma_x100 = new_gamma;
	mpro_drm__build_power_lut(mdrm, new_gamma);
	return count;
}
static DEVICE_ATTR_RW(gamma);

static ssize_t disable_partial_show(struct device *dev,
				    struct device_attribute *attr, char *buf)
{
	struct mpro_drm *mdrm = dev_get_drvdata(dev);

	return sysfs_emit(buf, "%u\n", mdrm->disable_partial ? 1 : 0);
}

static ssize_t disable_partial_store(struct device *dev,
				     struct device_attribute *attr,
				     const char *buf, size_t count)
{
	struct mpro_drm *mdrm = dev_get_drvdata(dev);
	bool val;

	if (kstrtobool(buf, &val))
		return -EINVAL;

	mdrm->disable_partial = val;
	return count;
}
static DEVICE_ATTR_RW(disable_partial);

static struct attribute *mpro_drm__attrs[] = {
	&dev_attr_rotation.attr,
	&dev_attr_brightness.attr,
	&dev_attr_gamma.attr,
	&dev_attr_disable_partial.attr,
	NULL,
};

static const struct attribute_group mpro_drm__attr_group = {
	.attrs     = mpro_drm__attrs,
	.bin_attrs = mpro_drm__bin_attrs,
};

/* ------------------------------------------------------------------ */
/* Public API                                                         */
/* ------------------------------------------------------------------ */

int mpro_drm__sysfs_create(struct mpro_drm *mdrm)
{
	return sysfs_create_group(&mdrm->pdev->dev.kobj, &mpro_drm__attr_group);
}

void mpro_drm__sysfs_remove(struct mpro_drm *mdrm)
{
	sysfs_remove_group(&mdrm->pdev->dev.kobj, &mpro_drm__attr_group);
}
