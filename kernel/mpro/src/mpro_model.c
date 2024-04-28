#include <linux/module.h>
#include <linux/usb.h>
#include <drm/drm_atomic_helper.h>
#include <drm/drm_gem_framebuffer_helper.h>
#include "mpro.h"

#define MODEL_DEFAULT		"MPRO\n"
#define MODEL_5IN		"MPRO-5\n"
#define MODEL_5IN_OLED		"MPRO-5H\n"
#define MODEL_4IN3		"MPRO-4IN3\n"
#define MODEL_4IN		"MPRO-4\n"
#define MODEL_6IN8		"MPRO-6IN8\n"
#define MODEL_3IN4		"MPRO-3IN4\n"

static const char cmd_get_screen[5] = {
	0x51, 0x02, 0x04, 0x1f, 0xfc
};

static const char cmd_get_version[5] = {
	0x51, 0x02, 0x04, 0x1f, 0xf8
};

static const char cmd_get_id[5] = {
	0x51, 0x02, 0x08, 0x1f, 0xf0
};

int mpro_get_screen(struct mpro_device *mpro) {

	struct usb_device *udev = mpro_to_usb_device(mpro);
	void *cmd = mpro->cmd;
	int ret;

	ret = usb_control_msg(udev, usb_sndctrlpipe(udev, 0),
				0xb5, 0x40, 0, 0,
				(void*)cmd_get_screen, 5,
				MPRO_MAX_DELAY);

	if ( ret < 5 )
		return -EIO;

	ret = usb_control_msg(udev, usb_rcvctrlpipe(udev, 0),
				0xb6, 0xc0, 0, 0,
				cmd, 1, MPRO_MAX_DELAY);

	if ( ret < 1 )
		return -EIO;

	ret = usb_control_msg(udev, usb_rcvctrlpipe(udev, 0),
				0xb7, 0xc0, 0, 0,
				cmd, 5, MPRO_MAX_DELAY);

	if ( ret < 5 )
		return -EIO;

	mpro -> screen = ((unsigned int*)(cmd + 1))[0];
	return 0;
}

int mpro_get_version(struct mpro_device *mpro) {

	struct usb_device *udev = mpro_to_usb_device(mpro);
	void *cmd = mpro -> cmd;
	int ret;

	ret = usb_control_msg(udev, usb_sndctrlpipe(udev, 0),
				0xb5, 0x40, 0, 0,
				(void*)cmd_get_version, 5,
				MPRO_MAX_DELAY);

	if ( ret < 5 )
		return -EIO;

	ret = usb_control_msg(udev, usb_rcvctrlpipe(udev, 0),
				0xb6, 0xc0, 0, 0,
				cmd, 1, MPRO_MAX_DELAY);

	if ( ret < 1 )
		return -EIO;

	ret = usb_control_msg(udev, usb_rcvctrlpipe(udev, 0),
				0xb7, 0xc0, 0, 0,
				cmd, 5, MPRO_MAX_DELAY);

	if ( ret < 5 )
		return -EIO;

	mpro -> version = ((unsigned int *)(cmd + 1))[0];
	return 0;
}

int mpro_get_id(struct mpro_device *mpro) {

	struct usb_device *udev = mpro_to_usb_device(mpro);
	void *cmd = mpro -> cmd;
	int ret;

	ret = usb_control_msg(udev, usb_sndctrlpipe(udev, 0),
				0xb5, 0x40, 0, 0,
				(void*)cmd_get_id, 5,
				MPRO_MAX_DELAY);

	if ( ret < 5 )
		return -EIO;

	ret = usb_control_msg(udev, usb_rcvctrlpipe(udev, 0),
				0xb6, 0xc0, 0, 0,
				cmd, 1, MPRO_MAX_DELAY);

	if ( ret < 1 )
		return -EIO;

	ret = usb_control_msg(udev, usb_rcvctrlpipe(udev, 0),
				0xb7, 0xc0, 0, 0,
				cmd, 9, MPRO_MAX_DELAY);

	if ( ret < 5 )
		return -EIO;

	memcpy(mpro -> id, cmd + 1, 8);
	return 0;
}

static const struct drm_mode_config_funcs mpro_mode_config_funcs = {
	.fb_create = drm_gem_fb_create_with_dirty,
	.atomic_check = drm_atomic_helper_check,
	.atomic_commit = drm_atomic_helper_commit,
};

void mpro_mode_config_setup(struct mpro_device *mpro) {

	int width 	= 480;
	int height	= 800;
	int margin	= 0;
	int width_mm	= 0;
	int height_mm	= 0;
	char *model = MODEL_DEFAULT;

	struct drm_device *dev = &mpro->dev;

	switch ( mpro -> screen ) {
	case 0x00000005:
		model = MODEL_5IN;
		height = 854;
		width_mm = 62;
		height_mm = 110;
		if ( mpro -> version != 0x00000003)
			margin = 320;
		break;

	case 0x00001005:
		model = MODEL_5IN_OLED;
		width = 720;
		height = 1280;
		width_mm = 62;
		height_mm = 110;
		break;

	case 0x00000304:
		model = MODEL_4IN3;
		width_mm = 56;
		height_mm = 94;
		break;

	case 0x00000004:
	case 0x00000b04:
	case 0x00000104:
		model = MODEL_4IN;
		width_mm = 53;
		height_mm = 86;
		break;

	case 0x00000007:
		model = MODEL_6IN8;
		width = 800;
		height = 480;
		width_mm = 89;
		height_mm = 148;
		break;

	case 0x00000403:
		model = MODEL_3IN4;
		width = 800;
		height = 800;
		width_mm = 88;
		height_mm = 88;
		break;
	}

	mpro->width = width;
	mpro->height = height;
	mpro->margin = margin;
	mpro->model = model;
	mpro->width_mm = width_mm;
	mpro->height_mm = height_mm;

	dev->mode_config.funcs = &mpro_mode_config_funcs;
	dev->mode_config.min_width = width;
	dev->mode_config.max_width = width;
	dev->mode_config.min_height = height;
	dev->mode_config.max_height = height;
}
