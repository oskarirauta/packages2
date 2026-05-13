// SPDX-License-Identifier: GPL-2.0
/*
 * mpro_backlight.c — MPro USB display backlight driver.
 *
 * Registers a standard Linux backlight class device and tracks the
 * DRM-side pipe enable/disable state through a screen state listener.
 * Includes a software gamma curve that makes the perceived brightness
 * change more linearly across the 0..255 range.
 *
 * The device has no readback for the current backlight level, so a
 * default value is sent on probe. The default can be configured with
 * the default_brightness module parameter; gamma likewise via
 * default_gamma.
 */

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/string.h>
#include <linux/stringify.h>

#include "mpro_backlight.h"

/* ------------------------------------------------------------------ */
/* Module parameters                                                  */
/* ------------------------------------------------------------------ */

static int mpro_bl_default_brightness = MPRO_BL_DEFAULT;
module_param_named(default_brightness, mpro_bl_default_brightness, int, 0444);
MODULE_PARM_DESC(default_brightness,
	"Initial backlight brightness on probe (0..255, default "
	__stringify(MPRO_BL_DEFAULT) "). The device has no readback "
	"for the current brightness, so this value is sent on each probe. "
	"Can be set on the kernel command line: "
	"mpro_backlight.default_brightness=N");

static int mpro_bl_default_gamma = MPRO_BL_GAMMA_DEFAULT;
module_param_named(default_gamma, mpro_bl_default_gamma, int, 0444);
MODULE_PARM_DESC(default_gamma,
	"Initial gamma curve, multiplied by 100 (50..400, default "
	__stringify(MPRO_BL_GAMMA_DEFAULT) " = 1.00 linear). "
	"Higher values make low brightness levels feel smoother. "
	"Can be set on the kernel command line: "
	"mpro_backlight.default_gamma=N");

/* ------------------------------------------------------------------ */
/* Gamma curve                                                        */
/* ------------------------------------------------------------------ */

/*
 * Apply the gamma curve to a raw value before sending it to the device.
 *   raw 0..255 -> device 0..255
 *   device = 255 * (raw/255)^(gamma/100)
 *
 * Human perception of brightness is roughly logarithmic, so a linear
 * 0..255 ramp feels like nothing happens at low values and then
 * brightness jumps suddenly. gamma > 1 produces a smoother curve at
 * the dark end.
 */
static u8 mpro_bl__curve(struct mpro_backlight *mb, u8 raw)
{
	u32 g = READ_ONCE(mb->gamma_x100);

	if (g == 100 || raw == 0 || raw == 255)
		return raw;

	return mpro_pow_lut(raw, g);
}

/* ------------------------------------------------------------------ */
/* Hardware command                                                   */
/* ------------------------------------------------------------------ */

static int mpro_bl__send_value(struct mpro_backlight *mb, u8 raw)
{
	/*
	 * Backlight command layout (8 bytes, fits a USB control transfer):
	 *   [0]    = 0x00 (reserved)
	 *   [1]    = 0x51 (command byte)
	 *   [2]    = 0x02 (sub-command: backlight)
	 *   [3..5] = 0
	 *   [6]    = brightness 0..255 (gamma-corrected)
	 *   [7]    = 0
	 */
	u8 cmd[8] = { 0x00, 0x51, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00 };
	u8 device_value = mpro_bl__curve(mb, raw);

	cmd[6] = device_value;
	return mpro_send_command(mb->mpro, cmd, sizeof(cmd));
}

/* ------------------------------------------------------------------ */
/* Backlight class callback                                           */
/* ------------------------------------------------------------------ */

static int mpro_bl__update(struct backlight_device *bl)
{
	struct mpro_backlight *mb = bl_get_data(bl);
	int requested = backlight_get_brightness(bl);
	u8 value = (u8)clamp(requested, 0, MPRO_BL_MAX);
	int ret = 0;

	mutex_lock(&mb->lock);
	mb->stored_value = value;
	if (mb->screen_on)
		ret = mpro_bl__send_value(mb, value);
	mutex_unlock(&mb->lock);

	return ret;
}

static const struct backlight_ops mpro_bl__ops = {
	.update_status = mpro_bl__update,
};

/* ------------------------------------------------------------------ */
/* Screen state listener                                              */
/* ------------------------------------------------------------------ */

static void mpro_bl__screen_off(void *priv)
{
	struct mpro_backlight *mb = priv;

	mutex_lock(&mb->lock);
	mb->screen_on = false;
	mpro_bl__send_value(mb, 0);
	mutex_unlock(&mb->lock);
}

static void mpro_bl__screen_on(void *priv)
{
	struct mpro_backlight *mb = priv;

	mutex_lock(&mb->lock);
	mb->screen_on = true;
	mpro_bl__send_value(mb, mb->stored_value);
	mutex_unlock(&mb->lock);
}

/* ------------------------------------------------------------------ */
/* Sysfs: gamma                                                       */
/* ------------------------------------------------------------------ */

static ssize_t gamma_show(struct device *dev,
			  struct device_attribute *attr, char *buf)
{
	struct backlight_device *bl = to_backlight_device(dev);
	struct mpro_backlight *mb = bl_get_data(bl);

	return sysfs_emit(buf, "%u.%02u\n",
			  mb->gamma_x100 / 100, mb->gamma_x100 % 100);
}

static ssize_t gamma_store(struct device *dev,
			   struct device_attribute *attr,
			   const char *buf, size_t count)
{
	struct backlight_device *bl = to_backlight_device(dev);
	struct mpro_backlight *mb = bl_get_data(bl);
	u32 new_gamma;
	int ret;

	ret = mpro_parse_gamma_x100(buf, &new_gamma);
	if (ret)
		return ret;

	if (new_gamma < MPRO_BL_GAMMA_MIN || new_gamma > MPRO_BL_GAMMA_MAX)
		return -EINVAL;

	WRITE_ONCE(mb->gamma_x100, new_gamma);

	mutex_lock(&mb->lock);
	if (mb->screen_on)
		mpro_bl__send_value(mb, mb->stored_value);
	mutex_unlock(&mb->lock);

	return count;
}
static DEVICE_ATTR_RW(gamma);

static struct attribute *mpro_bl__attrs[] = {
	&dev_attr_gamma.attr,
	NULL,
};

static const struct attribute_group mpro_bl__attr_group = {
	.attrs = mpro_bl__attrs,
};

/* ------------------------------------------------------------------ */
/* Probe / remove                                                     */
/* ------------------------------------------------------------------ */

static int mpro_bl__probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct mpro_device *mpro = dev_get_drvdata(dev->parent);
	struct backlight_properties props = {
		.type		= BACKLIGHT_RAW,
		.scale		= BACKLIGHT_SCALE_LINEAR,
		.max_brightness	= MPRO_BL_MAX,
	};
	struct mpro_backlight *mb;
	char name[64];
	u8  initial_brightness;
	u32 initial_gamma;
	int ret;

	if (!mpro)
		return -ENODEV;

	/* Validate the module parameters; clamp to safe ranges */
	if (mpro_bl_default_brightness < 0 ||
	    mpro_bl_default_brightness > MPRO_BL_MAX) {
		dev_warn(dev,
			 "default_brightness=%d invalid, clamping to 0..%u\n",
			 mpro_bl_default_brightness, MPRO_BL_MAX);
		mpro_bl_default_brightness =
			clamp(mpro_bl_default_brightness, 0, MPRO_BL_MAX);
	}
	initial_brightness = (u8)mpro_bl_default_brightness;

	if (mpro_bl_default_gamma < MPRO_BL_GAMMA_MIN ||
	    mpro_bl_default_gamma > MPRO_BL_GAMMA_MAX) {
		dev_warn(dev,
			 "default_gamma=%d invalid (must be %d..%d), "
			 "using %d\n",
			 mpro_bl_default_gamma,
			 MPRO_BL_GAMMA_MIN, MPRO_BL_GAMMA_MAX,
			 MPRO_BL_GAMMA_DEFAULT);
		mpro_bl_default_gamma = MPRO_BL_GAMMA_DEFAULT;
	}
	initial_gamma = (u32)mpro_bl_default_gamma;

	props.brightness = initial_brightness;

	mb = devm_kzalloc(dev, sizeof(*mb), GFP_KERNEL);
	if (!mb)
		return -ENOMEM;

	mb->mpro		= mpro;
	mb->screen_on		= true;
	mb->stored_value	= initial_brightness;
	mb->gamma_x100		= initial_gamma;
	mutex_init(&mb->lock);

	snprintf(name, sizeof(name), "mpro-%s", dev_name(dev->parent));

	mb->bl = devm_backlight_device_register(dev, name, dev, mb,
						&mpro_bl__ops, &props);
	if (IS_ERR(mb->bl))
		return dev_err_probe(dev, PTR_ERR(mb->bl),
				     "backlight register failed\n");

	platform_set_drvdata(pdev, mb);

	mb->listener.screen_off	= mpro_bl__screen_off;
	mb->listener.screen_on	= mpro_bl__screen_on;
	mb->listener.priv	= mb;

	ret = mpro_screen_listener_register(mpro, &mb->listener);
	if (ret)
		return dev_err_probe(dev, ret,
				     "screen listener register failed\n");

	/* Push the initial value — the device has no readback */
	mpro_bl__send_value(mb, initial_brightness);

	ret = sysfs_create_group(&pdev->dev.kobj, &mpro_bl__attr_group);
	if (ret) {
		mpro_screen_listener_unregister(mpro, &mb->listener);
		return dev_err_probe(dev, ret, "sysfs create failed\n");
	}

	dev_info(dev,
		 "backlight registered: %s "
		 "(brightness %u/%u, gamma %u.%02u)\n",
		 name, initial_brightness, MPRO_BL_MAX,
		 initial_gamma / 100, initial_gamma % 100);
	return 0;
}

static void mpro_bl__remove(struct platform_device *pdev)
{
	struct mpro_backlight *mb = platform_get_drvdata(pdev);

	if (!mb)
		return;

	sysfs_remove_group(&pdev->dev.kobj, &mpro_bl__attr_group);
	mpro_screen_listener_unregister(mb->mpro, &mb->listener);
	/* devm releases backlight_device_unregister() */
}

static struct platform_driver mpro_bl__driver = {
	.probe	= mpro_bl__probe,
	.remove	= mpro_bl__remove,
	.driver	= { .name = "mpro_backlight" },
};
module_platform_driver(mpro_bl__driver);

MODULE_AUTHOR("Oskari Rauta");
MODULE_DESCRIPTION("MPro USB display backlight driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:mpro_backlight");
