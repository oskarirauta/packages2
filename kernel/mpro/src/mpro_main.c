// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2019 Hans de Goede <hdegoede@redhat.com>
 */

#include <linux/module.h>
#include <linux/pm.h>

#include <drm/drm_gem.h>
#include <drm/drm_atomic_helper.h>
#include <drm/drm_atomic_state_helper.h>
#include <drm/drm_gem_shmem_helper.h>
#include <drm/drm_framebuffer.h>
#include <drm/drm_fbdev_generic.h>
#include <drm/drm_fb_helper.h>
#include <drm/drm_format_helper.h>
#include <drm/drm_fourcc.h>
#include <drm/drm_managed.h>
#include <drm/drm_probe_helper.h>
#include <drm/drm_gem_framebuffer_helper.h>
#include "mpro.h"

#define DRIVER_NAME		"MPRO"
#define DRIVER_DESC		"VoCore Screen"
#define DRIVER_DATE		"2024"
#define DRIVER_MAJOR		0
#define DRIVER_MINOR		1

static int partial = 0;
module_param(partial, int, 0660);
MODULE_PARM_DESC(partial, "set partial to 1 to enable partial updates");

static char cmd_draw[12] = {
	0x00, 0x2c, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static int mpro_data_alloc(struct mpro_device *mpro) {

	int block_size;

	block_size = mpro -> height * mpro -> width * MPRO_BPP / 8 + mpro -> margin;

	mpro -> data = drmm_kmalloc(&mpro -> dev, block_size, GFP_KERNEL);
	if ( !mpro -> data )
		return -ENOMEM;

	mpro -> data_size = block_size;

	if ( mpro -> partial != 0 )
		return 0;

	cmd_draw[2] = (char)(block_size >> 0);
	cmd_draw[3] = (char)(block_size >> 8);
	cmd_draw[4] = (char)(block_size >> 16);

	return 0;
}

static int mpro_update_frame(struct mpro_device *mpro) {

	struct usb_device *udev = mpro_to_usb_device(mpro);
	int cmd_len = mpro -> partial == 0 ? 6 : 12;
	int ret;

	/* 0x40 USB_DIR_OUT | USB_TYPE_VENDOR | USB_RECIP_DEVICE */
	ret = usb_control_msg(udev, usb_sndctrlpipe(udev, 0), 0xb0, 0x40,
			      0, 0, cmd_draw, cmd_len,
			      MPRO_MAX_DELAY);
	if ( ret < 0 )
		return ret;

	ret = usb_bulk_msg(udev, usb_sndbulkpipe(udev, 0x02), mpro -> data,
			   mpro -> data_size, NULL, MPRO_MAX_DELAY);
	if ( ret < 0 )
		return ret;

	return 0;
}

static int mpro_buf_copy(void *dst, struct iosys_map *src_map, struct drm_framebuffer *fb,
			 struct drm_rect *clip) {

	int ret;
	struct iosys_map dst_map;

	iosys_map_set_vaddr(&dst_map, dst);

	ret = drm_gem_fb_begin_cpu_access(fb, DMA_FROM_DEVICE);
	if ( ret )
		return ret;

	drm_fb_xrgb8888_to_rgb565(&dst_map, NULL, src_map, fb, clip, false);
	drm_gem_fb_end_cpu_access(fb, DMA_FROM_DEVICE);

	return 0;
}

void mpro_fb_mark_dirty(struct iosys_map *src, struct drm_framebuffer *fb,
				struct drm_rect *rect) {

	struct mpro_device *mpro = to_mpro(fb -> dev);
	struct drm_rect clip;
	int idx, ret;

	if ( !drm_dev_enter(fb -> dev, &idx))
		return;

	if ( mpro -> partial == 0 ) {

		clip.x1 = 0;
		clip.x2 = fb -> width;
		clip.y1 = 0;
		clip.y2 = fb -> height;

		ret = mpro_buf_copy(mpro -> data, src, fb, &clip);
		if ( ret )
			goto err_msg;
	} else {

		ret = mpro_buf_copy(mpro -> data, src, fb, rect);
		if ( ret )
			goto err_msg;

		int len = (rect -> x2 - rect -> x1) * (rect -> y2 - rect -> y1) * MPRO_BPP / 8;
		int width = rect -> x2 - rect -> x1;

		cmd_draw[6] = (char)(rect -> x1 >> 0);
		cmd_draw[7] = (char)(rect -> x1 >> 8);
		cmd_draw[8] = (char)(rect -> y1 >> 0);
		cmd_draw[9] = (char)(rect -> y1 >> 8);
		cmd_draw[10] = (char)(width >> 0);
		cmd_draw[11] = (char)(width >> 8);

		cmd_draw[2] = (char)(len >> 0);
		cmd_draw[3] = (char)(len >> 8);
		cmd_draw[4] = (char)(len >> 16);
	}

	ret = mpro_update_frame(mpro);

err_msg:
	if (ret)
		dev_err_once(fb -> dev -> dev, "Failed to update display %d\n", ret);

	drm_dev_exit(idx);
}

DEFINE_DRM_GEM_FOPS(mpro_fops);

static const struct drm_driver mpro_drm_driver = {
	.driver_features = DRIVER_MODESET | DRIVER_GEM | DRIVER_ATOMIC,

	.name		 = DRIVER_NAME,
	.desc		 = DRIVER_DESC,
	.date		 = DRIVER_DATE,
	.major		 = DRIVER_MAJOR,
	.minor		 = DRIVER_MINOR,

	.fops		 = &mpro_fops,
	DRM_GEM_SHMEM_DRIVER_OPS,
};

static int mpro_usb_probe(struct usb_interface *interface,
			  const struct usb_device_id *id) {

	struct mpro_device *mpro;
	struct drm_device *dev;
	int ret;

	mpro = devm_drm_dev_alloc(&interface -> dev, &mpro_drm_driver,
				      struct mpro_device, dev);
	if ( IS_ERR(mpro))
		return PTR_ERR(mpro);
	dev = &mpro -> dev;
	mpro -> partial = partial == 0 ? 0 : 1;

	mpro -> dmadev = usb_intf_get_dma_device(to_usb_interface(dev -> dev));
	if ( !mpro -> dmadev )
		drm_warn(dev, "buffer sharing not supported"); /* not an error */

	ret = mpro_get_screen(mpro);
	if ( ret ) {
		drm_err(dev, "can't get screen info.\n");
		goto err_put_device;
	}

	ret = mpro_get_version(mpro);
	if ( ret ) {
		drm_err(dev, "can't get screen version.\n");
		goto err_put_device;
	}

	ret = mpro_get_id(mpro);
	if ( ret ) {
		drm_err(dev, "can't get screen id.\n");
		goto err_put_device;
	}

	ret = drmm_mode_config_init(dev);
	if ( ret )
		goto err_put_device;

	mpro_mode_config_setup(mpro);

	mpro_edid_setup(mpro);

	ret = mpro_data_alloc(mpro);
	if ( ret )
		goto err_put_device;

	ret = mpro_conn_init(mpro);
	if ( ret )
		goto err_put_device;

	ret = mpro_pipe_init(mpro);
	if ( ret )
		goto err_put_device;

	drm_mode_config_reset(dev);

	usb_set_intfdata(interface, dev);

	ret = mpro_sysfs_init(mpro);
	if ( ret ) {
		drm_warn(dev, "failed to add partial_updates sysfs entry");
	}

	ret = drm_dev_register(dev, 0);
	if ( ret )
		goto err_put_device;

	drm_fbdev_generic_setup(dev, 0);
	mpro_bl_init(mpro);
	mpro_touch_init(mpro);

	return 0;

err_put_device:
	put_device(mpro -> dmadev);
	return ret;
}

static void mpro_usb_disconnect(struct usb_interface *interface) {

	struct drm_device *dev = usb_get_intfdata(interface);
	struct mpro_device *mpro = to_mpro(dev);

	mpro_touch_exit(mpro);

	put_device(mpro->dmadev);
	mpro->dmadev = NULL;
	drm_dev_unplug(dev);
	drm_atomic_helper_shutdown(dev);
}

static const struct usb_device_id id_table[] = {
	{ USB_DEVICE(0xc872, 0x1004) },
	{},
};
MODULE_DEVICE_TABLE(usb, id_table);

static struct usb_driver mpro_usb_driver = {
	.name = "mpro",
	.probe = mpro_usb_probe,
	.disconnect = mpro_usb_disconnect,
	.id_table = id_table,
};

module_usb_driver(mpro_usb_driver);
MODULE_AUTHOR("ieiao <ieiao@outlook.com>");
MODULE_LICENSE("GPL");
