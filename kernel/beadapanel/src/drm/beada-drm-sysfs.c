// SPDX-License-Identifier: GPL-2.0-only
/*
 * beada-drm-sysfs.c – BeadaPanel DRM: per-device sysfs attributes
 *
 * Exposes per-device behaviour controls under the platform device's
 * sysfs directory:
 *
 *   /sys/devices/platform/.../beada-drm.N/rotation   (RW, 0..7)
 *   /sys/devices/platform/.../beada-drm.N/noblank    (RW, 0..1)
 *
 * Attribute summaries
 * ───────────────────
 *
 *   rotation  Active display orientation.  Values match the mpro driver
 *             convention so settings transfer one-to-one:
 *               0  ROTATE_0
 *               1  ROTATE_90
 *               2  ROTATE_180
 *               3  ROTATE_270
 *               4  ROTATE_0   + REFLECT_X    (horizontal mirror)
 *               5  ROTATE_90  + REFLECT_X
 *               6  ROTATE_180 + REFLECT_X    (== REFLECT_Y)
 *               7  ROTATE_270 + REFLECT_X
 *             Writes invoke beada_apply_rotation() (beada-drm-kms.c),
 *             which updates mode_config, rebuilds the EDID, and fires
 *             a hotplug event so compositors re-probe.
 *
 *   noblank   When 1, pipe_disable leaves the last image on the display
 *             instead of sending an all-zero frame.  Initial value is
 *             copied from the beada_drm noblank module parameter at
 *             probe; per-device writes override it for that device only.
 *             Effective on the next pipe_disable (DRM client exit,
 *             DPMS-off, suspend).
 *
 * Multi-device note
 * ─────────────────
 * Each BeadaPanel that probes gets its own platform device and its own
 * attribute set, so two panels on the same host can be configured
 * independently.
 */

#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/string.h>
#include <linux/sysfs.h>

#include "beada-drm.h"

/* ------------------------------------------------------------------ */
/* Rotation table                                                       */
/* ------------------------------------------------------------------ */

static const u16 beada_rotation_map[] = {
	DRM_MODE_ROTATE_0,
	DRM_MODE_ROTATE_90,
	DRM_MODE_ROTATE_180,
	DRM_MODE_ROTATE_270,
	DRM_MODE_ROTATE_0   | DRM_MODE_REFLECT_X,
	DRM_MODE_ROTATE_90  | DRM_MODE_REFLECT_X,
	DRM_MODE_ROTATE_180 | DRM_MODE_REFLECT_X,
	DRM_MODE_ROTATE_270 | DRM_MODE_REFLECT_X,
};

static inline struct beada_drm_dev *dev_to_beada_drm(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);

	return platform_get_drvdata(pdev);
}

/* ------------------------------------------------------------------ */
/* rotation                                                             */
/* ------------------------------------------------------------------ */

static ssize_t rotation_show(struct device *dev,
			     struct device_attribute *attr, char *buf)
{
	struct beada_drm_dev *bdev = dev_to_beada_drm(dev);
	u16 cur = bdev->rotation;
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(beada_rotation_map); i++)
		if (beada_rotation_map[i] == cur)
			return sysfs_emit(buf, "%u\n", i);

	return sysfs_emit(buf, "0\n");
}

static ssize_t rotation_store(struct device *dev,
			      struct device_attribute *attr,
			      const char *buf, size_t count)
{
	struct beada_drm_dev *bdev = dev_to_beada_drm(dev);
	unsigned int val;
	int ret;

	ret = kstrtouint(buf, 10, &val);
	if (ret)
		return ret;

	if (val >= ARRAY_SIZE(beada_rotation_map))
		return -EINVAL;

	beada_apply_rotation(bdev, beada_rotation_map[val]);
	return count;
}

static DEVICE_ATTR_RW(rotation);

/* ------------------------------------------------------------------ */
/* noblank                                                              */
/* ------------------------------------------------------------------ */

static ssize_t noblank_show(struct device *dev,
			    struct device_attribute *attr, char *buf)
{
	struct beada_drm_dev *bdev = dev_to_beada_drm(dev);

	return sysfs_emit(buf, "%u\n", bdev->noblank ? 1 : 0);
}

static ssize_t noblank_store(struct device *dev,
			     struct device_attribute *attr,
			     const char *buf, size_t count)
{
	struct beada_drm_dev *bdev = dev_to_beada_drm(dev);
	bool val;
	int ret;

	ret = kstrtobool(buf, &val);
	if (ret)
		return ret;

	/*
	 * The store is racy with a concurrent pipe_disable on a different
	 * CPU, but the worst case is that one pipe_disable uses the old
	 * value — which is fine.  No lock needed.
	 */
	bdev->noblank = val;
	return count;
}

static DEVICE_ATTR_RW(noblank);

/* ------------------------------------------------------------------ */
/* Attribute group                                                      */
/* ------------------------------------------------------------------ */

static struct attribute *beada_drm_attrs[] = {
	&dev_attr_rotation.attr,
	&dev_attr_noblank.attr,
	NULL,
};

static const struct attribute_group beada_drm_attr_group = {
	.attrs = beada_drm_attrs,
};

/* ------------------------------------------------------------------ */
/* Public init / fini                                                   */
/* ------------------------------------------------------------------ */

int beada_sysfs_init(struct beada_drm_dev *bdev)
{
	struct platform_device *pdev =
		to_platform_device(bdev->drm.dev->parent);

	return sysfs_create_group(&pdev->dev.kobj, &beada_drm_attr_group);
}

void beada_sysfs_fini(struct beada_drm_dev *bdev)
{
	struct platform_device *pdev =
		to_platform_device(bdev->drm.dev->parent);

	sysfs_remove_group(&pdev->dev.kobj, &beada_drm_attr_group);
}
