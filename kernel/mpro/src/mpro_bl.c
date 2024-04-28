#include "mpro.h"

#if IS_ENABLED(CONFIG_BACKLIGHT_CLASS_DEVICE) || IS_ENABLED(CONFIG_FB_BACKLIGHT)

#define BACKLIGHT_NAME		"mpro_bl"

#define MIN_BL_LEVEL		0
#define MAX_BL_LEVEL		255

static char cmd_set_brightness[8] = {
        0x00, 0x51, 0x02, 0x00, 0x00, 0x00, 0xff, 0x00
};

int mpro_bl_get_brightness(struct backlight_device *bd) {

	int level = bd -> props.brightness;

	if ( level > bd -> props.max_brightness )
		level = bd -> props.max_brightness;
	else if ( level < MIN_BL_LEVEL )
		level = MIN_BL_LEVEL;

	return level;
}

int mpro_bl_update_status(struct backlight_device *bd) {

	struct mpro_device *mpro = bl_get_data(bd);
	struct usb_device *udev = mpro_to_usb_device(mpro);
	int level = bd -> props.brightness;

	if ( bd -> props.power != mpro -> bl_power ) {

		if ( bd -> props.power == FB_BLANK_UNBLANK )
			bd -> props.max_brightness = MAX_BL_LEVEL;
		else if (bd -> props.power == FB_BLANK_NORMAL )
			bd -> props.max_brightness = (MAX_BL_LEVEL * 0.2) * 3;
		else if (bd -> props.power == FB_BLANK_VSYNC_SUSPEND)
			bd -> props.max_brightness = (MAX_BL_LEVEL * 0.2) * 2;
		else if (bd -> props.power == FB_BLANK_HSYNC_SUSPEND)
			bd -> props.max_brightness = MAX_BL_LEVEL * 0.2;
		else if (bd -> props.power == FB_BLANK_POWERDOWN)
			bd -> props.max_brightness = 0;
		else if (bd -> props.power > FB_BLANK_POWERDOWN) {
			bd -> props.power = FB_BLANK_POWERDOWN;
			bd -> props.max_brightness = 0;
		}

		level = bd -> props.max_brightness;
		mpro -> bl_power = bd -> props.power;
	}

	if ( level < MIN_BL_LEVEL )
		level = MIN_BL_LEVEL;
	else if ( level > bd -> props.max_brightness )
		return -EINVAL;
	else if ( level > MAX_BL_LEVEL )
		level = MAX_BL_LEVEL;

	cmd_set_brightness[6] = level;

	int ret = usb_control_msg(udev, usb_sndctrlpipe(udev, 0),
				0xb0, 0x40, 0, 0,
				(void*)cmd_set_brightness, 8,
				MPRO_MAX_DELAY);
	if ( ret < 5 )
		return -EIO;

	bd -> props.brightness = level;
	return 0;
}

static const struct backlight_ops mpro_bl_ops = {
	.update_status = mpro_bl_update_status,
	.get_brightness = mpro_bl_get_brightness,
};

static const struct backlight_properties mpro_bl_props = {
	.type = BACKLIGHT_RAW,
	.max_brightness = MAX_BL_LEVEL,
	.brightness = MAX_BL_LEVEL,
	.fb_blank = FB_BLANK_UNBLANK,
	.power = FB_BLANK_UNBLANK,
};

void mpro_bl_init(struct mpro_device* mpro) {

	struct backlight_device *bl_dev;

	//bl_dev = devm_backlight_device_register(mpro -> dmadev, BACKLIGHT_NAME, mpro -> dmadev, mpro, &mpro_bl_ops, &mpro_bl_props);
	bl_dev = devm_backlight_device_register(mpro -> dmadev, BACKLIGHT_NAME, mpro -> dev.dev, mpro, &mpro_bl_ops, &mpro_bl_props);

	if ( IS_ERR(bl_dev)) {
		dev_err(mpro -> dmadev, "unable to register backlight device\n");
		return;
	}

	mpro -> bl_power = bl_dev -> props.power;
	dev_info(mpro -> dmadev, "backlight registered\n");
}

#else
void mpro_bl_init(struct mpro_device* mpro) {}
#endif
