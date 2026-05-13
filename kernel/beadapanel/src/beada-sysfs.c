// SPDX-License-Identifier: GPL-2.0-only
/*
 * beada-sysfs.c – BeadaPanel MFD parent: sysfs attribute group
 *
 * Creates a "beada_panel" subdirectory under the USB interface device:
 *
 *   /sys/bus/usb/devices/X-Y:1.0/beada_panel/
 *       model                "3"
 *       resolution           "480x320"
 *       width_mm             "62"
 *       height_mm            "40"
 *       physical_size        "2.9 in"
 *       serial_number        "SN1234567890..."
 *       firmware_version     "1815"         (decimal; 0x717 = fw 7.17)
 *       panellink_version    "1"
 *       statuslink_version   "2"
 *       platform             "T113 (5)"
 *       storage_size         "7680 MiB"
 *       max_brightness       "100"
 *       sync_time            (write-only: echo anything to sync kernel UTC→device)
 *
 * brightness and current_brightness are intentionally absent here; they are
 * the responsibility of the beada-backlight child driver.
 *
 * All read attributes are set once at probe time (from Status-Link GET_INFO)
 * and are read-only for the device's lifetime.
 *
 * physical_size is the display diagonal in inches (one decimal place):
 *   √(width_mm² + height_mm²) / 25.4
 */

#include <linux/device.h>
#include <linux/math.h>		/* int_sqrt64() */
#include <linux/string.h>
#include <linux/sysfs.h>

#include "beada-priv.h"

/* ------------------------------------------------------------------ */
/* Helpers                                                              */
/* ------------------------------------------------------------------ */

static inline struct beada_mfd_dev *dev_to_beada(struct device *dev)
{
	return dev_get_drvdata(dev);
}

/**
 * diagonal_tenth_inch() - Display diagonal in tenths of an inch
 * @w_mm: Width in millimetres
 * @h_mm: Height in millimetres
 *
 * Returns e.g. 29 for a 2.9" display.  Uses integer sqrt with rounding.
 */
static unsigned int diagonal_tenth_inch(u16 w_mm, u16 h_mm)
{
	u64 diag_sq  = (u64)w_mm * w_mm + (u64)h_mm * h_mm;
	u64 diag_01mm = int_sqrt64(diag_sq * 100);

	return (unsigned int)((diag_01mm * 10 + 127) / 254);
}

/**
 * beada_platform_name() - Human-readable SoC name for the platform field
 * @platform: Value from GET_INFO hardware_platform byte (Status-Link spec)
 */
static const char *beada_platform_name(u8 platform)
{
	switch (platform) {
	case 1: return "i.MX6UL";
	case 2: return "i.MX6ULL";
	case 4: return "V3S";
	case 5: return "T113";
	default: return "unknown";
	}
}

/* ------------------------------------------------------------------ */
/* Display geometry                                                     */
/* ------------------------------------------------------------------ */

static ssize_t model_show(struct device *dev, struct device_attribute *attr,
			  char *buf)
{
	return sysfs_emit(buf, "%s\n", dev_to_beada(dev)->panel.model);
}
static DEVICE_ATTR_RO(model);

static ssize_t resolution_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	const struct beada_panel_info *p = &dev_to_beada(dev)->panel;

	return sysfs_emit(buf, "%ux%u\n", p->width, p->height);
}
static DEVICE_ATTR_RO(resolution);

static ssize_t width_mm_show(struct device *dev,
			     struct device_attribute *attr, char *buf)
{
	return sysfs_emit(buf, "%u\n", dev_to_beada(dev)->panel.width_mm);
}
static DEVICE_ATTR_RO(width_mm);

static ssize_t height_mm_show(struct device *dev,
			      struct device_attribute *attr, char *buf)
{
	return sysfs_emit(buf, "%u\n", dev_to_beada(dev)->panel.height_mm);
}
static DEVICE_ATTR_RO(height_mm);

static ssize_t physical_size_show(struct device *dev,
				  struct device_attribute *attr, char *buf)
{
	const struct beada_panel_info *p = &dev_to_beada(dev)->panel;
	unsigned int tenths;

	if (!p->width_mm || !p->height_mm)
		return sysfs_emit(buf, "unknown\n");

	tenths = diagonal_tenth_inch(p->width_mm, p->height_mm);
	return sysfs_emit(buf, "%u.%u in\n", tenths / 10, tenths % 10);
}
static DEVICE_ATTR_RO(physical_size);

/* ------------------------------------------------------------------ */
/* Device identification                                                */
/* ------------------------------------------------------------------ */

static ssize_t serial_number_show(struct device *dev,
				  struct device_attribute *attr, char *buf)
{
	return sysfs_emit(buf, "%s\n", dev_to_beada(dev)->sn);
}
static DEVICE_ATTR_RO(serial_number);

/* firmware_version as decimal integer (BCD → use as-is for simplicity) */
static ssize_t firmware_version_show(struct device *dev,
				     struct device_attribute *attr, char *buf)
{
	return sysfs_emit(buf, "0x%04x\n", dev_to_beada(dev)->firmware_version);
}
static DEVICE_ATTR_RO(firmware_version);

static ssize_t panellink_version_show(struct device *dev,
				      struct device_attribute *attr, char *buf)
{
	return sysfs_emit(buf, "%u\n", dev_to_beada(dev)->panellink_version);
}
static DEVICE_ATTR_RO(panellink_version);

static ssize_t statuslink_version_show(struct device *dev,
				       struct device_attribute *attr, char *buf)
{
	return sysfs_emit(buf, "%u\n", dev_to_beada(dev)->statuslink_version);
}
static DEVICE_ATTR_RO(statuslink_version);

/**
 * platform_show - SoC name and numeric code, e.g. "T113 (5)\n"
 */
static ssize_t platform_show(struct device *dev,
			     struct device_attribute *attr, char *buf)
{
	u8 p = dev_to_beada(dev)->platform;

	return sysfs_emit(buf, "%s (%u)\n", beada_platform_name(p), p);
}
static DEVICE_ATTR_RO(platform);

/**
 * storage_size_show - eMMC capacity in MiB
 *
 * GET_INFO returns the size in KiB; we convert to MiB for readability.
 */
static ssize_t storage_size_show(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	u32 kb = dev_to_beada(dev)->storage_size_kb;

	if (!kb)
		return sysfs_emit(buf, "unknown\n");

	return sysfs_emit(buf, "%u MiB\n", kb / 1024);
}
static DEVICE_ATTR_RO(storage_size);

static ssize_t max_brightness_show(struct device *dev,
				   struct device_attribute *attr, char *buf)
{
	return sysfs_emit(buf, "%u\n", dev_to_beada(dev)->panel.max_brightness);
}
static DEVICE_ATTR_RO(max_brightness);

/* ------------------------------------------------------------------ */
/* Clock sync (write-only)                                              */
/* ------------------------------------------------------------------ */

/**
 * sync_time_store - Push current kernel UTC time to the device RTC
 *
 *   echo 1 > /sys/.../beada_panel/sync_time
 *
 * Useful after timezone changes so the device's standby clock shows the
 * correct local time.
 */
static ssize_t sync_time_store(struct device *dev,
			       struct device_attribute *attr,
			       const char *buf, size_t count)
{
	int ret = beada_sync_time(dev_to_beada(dev));

	return ret ? ret : count;
}
static DEVICE_ATTR_WO(sync_time);

/* ------------------------------------------------------------------ */
/* Statistics                                                           */
/* ------------------------------------------------------------------ */

static ssize_t stats_show(struct device *dev,
			  struct device_attribute *attr, char *buf)
{
	struct beada_mfd_dev *bdev = dev_get_drvdata(dev);
	u32 submitted, displayed, dropped;
	u32 efficiency = 0;

	submitted = atomic_read(&bdev->stats_submitted);
	displayed = atomic_read(&bdev->stats_displayed);
	dropped   = atomic_read(&bdev->stats_dropped);

	if (submitted > 0)
		efficiency = (u32)div_u64((u64)displayed * 10000, submitted);

	return sysfs_emit(buf,
			  "submitted=%u displayed=%u dropped=%u "
			  "efficiency=%u.%02u%%\n",
			  submitted, displayed, dropped,
			  efficiency / 100, efficiency % 100);
}
static DEVICE_ATTR_RO(stats);

static ssize_t reset_stats_store(struct device *dev,
				 struct device_attribute *attr,
				 const char *buf, size_t count)
{
	struct beada_mfd_dev *bdev = dev_get_drvdata(dev);
	bool val;

	if (kstrtobool(buf, &val))
		return -EINVAL;

	if (val) {
		atomic_set(&bdev->stats_submitted, 0);
		atomic_set(&bdev->stats_displayed, 0);
		atomic_set(&bdev->stats_dropped, 0);
	}

	return count;
}
static DEVICE_ATTR_WO(reset_stats);

/* ------------------------------------------------------------------ */
/* Attribute group                                                      */
/* ------------------------------------------------------------------ */

static struct attribute *beada_panel_attrs[] = {
	/* Display geometry */
	&dev_attr_model.attr,
	&dev_attr_resolution.attr,
	&dev_attr_width_mm.attr,
	&dev_attr_height_mm.attr,
	&dev_attr_physical_size.attr,
	/* Device identification */
	&dev_attr_serial_number.attr,
	&dev_attr_firmware_version.attr,
	&dev_attr_panellink_version.attr,
	&dev_attr_statuslink_version.attr,
	&dev_attr_platform.attr,
	&dev_attr_storage_size.attr,
	&dev_attr_max_brightness.attr,
	/* Pipeline statistics */
	&dev_attr_stats.attr,
	/* Actions */
	&dev_attr_sync_time.attr,
	&dev_attr_reset_stats.attr,
	NULL,
};

/*
 * Creates /sys/.../beada_panel/ as a named subdirectory so all attributes
 * are grouped away from the raw USB device attributes.
 */
static const struct attribute_group beada_panel_attr_group = {
	.name  = "beadapanel",
	.attrs = beada_panel_attrs,
};

/* ------------------------------------------------------------------ */
/* Init (called from beada-main.c probe)                                */
/* ------------------------------------------------------------------ */

int beada_sysfs_init(struct beada_mfd_dev *beada)
{
	return devm_device_add_group(beada->dev, &beada_panel_attr_group);
}
