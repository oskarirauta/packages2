// SPDX-License-Identifier: GPL-2.0-only
/*
 * beada-backlight-sysfs.c – Per-device "gamma" sysfs attribute
 *
 * Exposes one writable attribute on the backlight class device:
 *
 *   /sys/class/backlight/<name>/gamma
 *
 * Reads return the current gamma as "X.YZ" (e.g. "2.20").  Writes
 * accept the same form ("1.5", "1.50", "2", "0.50") and update the
 * curve, then immediately re-send the current brightness through the
 * new curve so the change is visible without a brightness write.
 *
 * Why not module-parameter-only
 * ─────────────────────────────
 * The same userspace toolchain that controls brightness (compositors,
 * desktop sliders, OSDs) tends to want gamma as a runtime tunable.  A
 * load-time module parameter would force a rmmod / modprobe cycle for
 * every preference change.  A per-device sysfs attribute is also the
 * right place to put it when multiple BeadaPanels are attached: each
 * one gets its own gamma setting independently.
 */

#include <linux/backlight.h>
#include <linux/device.h>
#include <linux/kstrtox.h>
#include <linux/mutex.h>
#include <linux/string.h>
#include <linux/sysfs.h>

#include "beada-backlight.h"

/* ------------------------------------------------------------------ */
/* Parser                                                               */
/* ------------------------------------------------------------------ */

/**
 * parse_gamma_x100() - Parse "X.YZ" or "X" into gamma × 100
 * @buf: Input string from sysfs write (may include trailing newline)
 * @out: Parsed value, on success
 *
 * Accepted forms:
 *   "1"     →  100
 *   "1.5"   →  150     (single fractional digit padded as tenths)
 *   "1.50"  →  150
 *   "0.50"  →   50
 *   "2.20"  →  220
 *
 * Rejects negative values, more than two fractional digits, and empty
 * input.  The caller validates the resulting value against
 * BEADA_BL_GAMMA_MIN / MAX.
 */
static int parse_gamma_x100(const char *buf, u32 *out)
{
	char tmp[16];
	const char *dot;
	u32 whole = 0, frac = 0;
	size_t len;
	int ret;

	len = strnlen(buf, sizeof(tmp));
	if (len == 0 || len >= sizeof(tmp))
		return -EINVAL;

	memcpy(tmp, buf, len);
	tmp[len] = '\0';

	while (len > 0 && (tmp[len - 1] == '\n' || tmp[len - 1] == ' '))
		tmp[--len] = '\0';

	dot = strchr(tmp, '.');
	if (dot) {
		char w[8], f[4];
		size_t wlen = dot - tmp;
		size_t flen = len - wlen - 1;

		if (wlen >= sizeof(w) || flen == 0 || flen > 2)
			return -EINVAL;

		memcpy(w, tmp, wlen); w[wlen] = '\0';
		memcpy(f, dot + 1, flen); f[flen] = '\0';

		ret = kstrtou32(w, 10, &whole);
		if (ret)
			return ret;
		ret = kstrtou32(f, 10, &frac);
		if (ret)
			return ret;

		/* "1.5" should mean 1.50, not 1.05 — pad single digit */
		if (flen == 1)
			frac *= 10;
	} else {
		ret = kstrtou32(tmp, 10, &whole);
		if (ret)
			return ret;
	}

	*out = whole * 100 + frac;
	return 0;
}

/* ------------------------------------------------------------------ */
/* Attribute show / store                                               */
/* ------------------------------------------------------------------ */

static ssize_t gamma_show(struct device *dev,
			  struct device_attribute *attr, char *buf)
{
	struct backlight_device *bl = to_backlight_device(dev);
	struct beada_backlight  *mb = bl_get_data(bl);
	u32 g = READ_ONCE(mb->gamma_x100);

	return sysfs_emit(buf, "%u.%02u\n", g / 100, g % 100);
}

static ssize_t gamma_store(struct device *dev,
			   struct device_attribute *attr,
			   const char *buf, size_t count)
{
	struct backlight_device *bl = to_backlight_device(dev);
	struct beada_backlight  *mb = bl_get_data(bl);
	u32 new_gamma;
	int ret;

	ret = parse_gamma_x100(buf, &new_gamma);
	if (ret)
		return ret;

	if (new_gamma < BEADA_BL_GAMMA_MIN || new_gamma > BEADA_BL_GAMMA_MAX)
		return -EINVAL;

	WRITE_ONCE(mb->gamma_x100, new_gamma);

	/*
	 * Re-apply through the new curve so the user sees the change
	 * immediately.  beada_bl_send() requires @lock; the call can
	 * fail on a transient USB error, but we don't propagate that —
	 * the curve change has already been recorded and the next
	 * brightness write will land on the new value.
	 */
	mutex_lock(&mb->lock);
	beada_bl_send(mb, mb->stored_value);
	mutex_unlock(&mb->lock);

	return count;
}
static DEVICE_ATTR_RW(gamma);

/* ------------------------------------------------------------------ */
/* Attribute group                                                      */
/* ------------------------------------------------------------------ */

static struct attribute *beada_bl_attrs[] = {
	&dev_attr_gamma.attr,
	NULL,
};

static const struct attribute_group beada_bl_attr_group = {
	.attrs = beada_bl_attrs,
};

/* ------------------------------------------------------------------ */
/* Public init / fini                                                   */
/* ------------------------------------------------------------------ */

int beada_bl_sysfs_init(struct beada_backlight *mb)
{
	return sysfs_create_group(&mb->bl->dev.kobj, &beada_bl_attr_group);
}

void beada_bl_sysfs_fini(struct beada_backlight *mb)
{
	sysfs_remove_group(&mb->bl->dev.kobj, &beada_bl_attr_group);
}
