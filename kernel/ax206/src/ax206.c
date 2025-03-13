// SPDX-License-Identifier: MIT
/*
 * Copyright 2024 Oskari Rauta <oskari.rauta@gmail.com>
 */

#include <linux/module.h>
#include <linux/pm.h>
#include <linux/usb.h>

#include <drm/drm_atomic_helper.h>
#include <drm/drm_atomic_state_helper.h>
#include <drm/drm_connector.h>
#include <drm/drm_damage_helper.h>
#include <drm/drm_drv.h>
#include <drm/drm_fb_helper.h>
#include <drm/drm_file.h>
#include <drm/drm_format_helper.h>
#include <drm/drm_fourcc.h>
#include <drm/drm_framebuffer.h>
#include <drm/drm_fbdev_generic.h>
#include <drm/drm_gem_atomic_helper.h>
#include <drm/drm_gem_framebuffer_helper.h>
#include <drm/drm_gem_shmem_helper.h>
#include <drm/drm_ioctl.h>
#include <drm/drm_managed.h>
#include <drm/drm_modeset_helper_vtables.h>
#include <drm/drm_probe_helper.h>
#include <drm/drm_simple_kms_helper.h>
#include <drm/drm_modes.h>

#define DRIVER_NAME		"ax206"
#define DRIVER_DESC		"dpf/ax-206 screen"
#define DRIVER_DATE		"2024"
#define DRIVER_MAJOR		0
#define DRIVER_MINOR		1

#define AX206_BPP		16
#define AX206_MAX_DELAY		100

#define MPRO_BPP		16
#define MPRO_MAX_DELAY		100

#define MODEL_DEFAULT		"MPRO\n"
#define MODEL_5IN		"MPRO-5\n"
#define MODEL_5IN_OLED		"MPRO-5H\n"
#define MODEL_4IN3		"MPRO-4IN3\n"
#define MODEL_4IN		"MPRO-4\n"
#define MODEL_6IN8		"MPRO-6IN8\n"
#define MODEL_3IN4		"MPRO-3IN4\n"

enum DIR : unsigned char {
	IN = 0x01,
	OUT = 0x81
};

enum CMD : unsigned char {
	SETPROPERTY = 0x01,
	BLIT = 0x12
};

struct ax206_device {
	struct drm_device		dev;
	struct device			*dmadev;
	struct drm_simple_display_pipe	pipe;
	struct drm_connector		conn;
/*
	unsigned int			screen;
	unsigned int			version;
	unsigned char			id[8];
	char 				*model;
*/
	unsigned int			width;
	unsigned int			height;
/*
	unsigned int			width_mm;
	unsigned int			height_mm;
	unsigned int			margin;
*/
	unsigned char			cmd[64];
	unsigned char			*data;
	unsigned int			data_size;
};

#define to_ax206(__dev) container_of(__dev, struct ax206_device, dev)

static const char cmd_get_screen[5] = {
	0x51, 0x02, 0x04, 0x1f, 0xfc
};

static const char cmd_get_version[5] = {
	0x51, 0x02, 0x04, 0x1f, 0xf8
};

static const char cmd_get_id[5] = {
	0x51, 0x02, 0x08, 0x1f, 0xf0
};

static char cmd_draw[6] = {
	0x00, 0x2c, 0x00, 0x00, 0x00, 0x00
};

//static char cmd_draw_part[12] = {
//	0x00, 0x2c, 0x00, 0x00, 0x00, 0x00,
//	0x00, 0x00, 0x00, 0x00, 0x00, 0x00
//};

static char cmd_quit_sleep[6] = {
	0x00, 0x29, 0x00, 0x00, 0x00, 0x00
};

static inline struct usb_device *ax206_to_usb_device(struct ax206_device *ax206) {
	return interface_to_usbdev(to_usb_interface(ax206 -> dev.dev));
}

static int ax206_data_alloc(struct ax206_device *ax206) {

	int block_size;

	block_size = mpro->height * mpro->width * AX206_BPP / 8;

	ax206 -> data = drmm_kmalloc(&ax206 -> dev, block_size, GFP_KERNEL);
	if ( !ax206 -> data )
		return -ENOMEM;

	ax206 -> data_size = block_size;

	cmd_draw[2] = (char)(block_size >> 0);
	cmd_draw[3] = (char)(block_size >> 8);
	cmd_draw[4] = (char)(block_size >> 16);

	return 0;
}

static int ax206_blit(struct ax206_device *ax206) {

	

}

static int ax206_update_frame(struct mpro_device *mpro)
{
	struct usb_device *udev = mpro_to_usb_device(mpro);
	int ret;

	/* 0x40 USB_DIR_OUT | USB_TYPE_VENDOR | USB_RECIP_DEVICE */
	ret = usb_control_msg(udev, usb_sndctrlpipe(udev, 0), 0xb0, 0x40,
			      0, 0, cmd_draw, sizeof(cmd_draw),
			      MPRO_MAX_DELAY);
	if (ret < 0) {
		return ret;
	}

	ret = usb_bulk_msg(udev, usb_sndbulkpipe(udev, 0x02), mpro->data,
			   mpro->data_size, NULL, MPRO_MAX_DELAY);
	if (ret < 0) {
		return ret;
	}

	return 0;
}

static int mpro_send_command(struct mpro_device *mpro, void* cmd, unsigned int len)
{
	struct usb_device *udev = mpro_to_usb_device(mpro);
	int ret;

	if (len == 0)
		return 0;

	ret = usb_control_msg(udev, usb_sndctrlpipe(udev, 0),
			      0xb0, 0x40, 0, 0, cmd, len,
			      MPRO_MAX_DELAY);

	return ret;
}

static int mpro_buf_copy(void *dst, struct iosys_map *src_map, struct drm_framebuffer *fb,
			 struct drm_rect *clip)
{
	int ret;
	struct iosys_map dst_map;

	iosys_map_set_vaddr(&dst_map, dst);

	ret = drm_gem_fb_begin_cpu_access(fb, DMA_FROM_DEVICE);
	if (ret)
		return ret;

	drm_fb_xrgb8888_to_rgb565(&dst_map, NULL, src_map, fb, clip, false);

	drm_gem_fb_end_cpu_access(fb, DMA_FROM_DEVICE);

	return 0;
}

static void mpro_fb_mark_dirty(struct iosys_map *src, struct drm_framebuffer *fb,
				struct drm_rect *rect)
{
	struct mpro_device *mpro = to_mpro(fb->dev);
	struct drm_rect clip;
	int idx, ret;

	if (!drm_dev_enter(fb->dev, &idx))
		return;

	clip.x1 = 0;
	clip.x2 = fb->width;
	clip.y1 = 0;
	clip.y2 = fb->height;

	ret = mpro_buf_copy(mpro->data, src, fb, &clip);
	if (ret)
		goto err_msg;

	ret = mpro_update_frame(mpro);

err_msg:
	if (ret)
		dev_err_once(fb->dev->dev, "Failed to update display %d\n", ret);

	drm_dev_exit(idx);
}

/* ------------------------------------------------------------------ */
/* mpro connector						      */

static struct drm_display_mode mpro_display_mode = {
	.clock = 23040, /* 800x480x60*0.001 ( /1000 )*/
	.hdisplay = 800, .hsync_start = 840, .hsync_end = 968, .htotal = 1056,
	.vdisplay = 480, .vsync_start = 481, .vsync_end = 485, .vtotal = 508,
	.flags = DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC,
	.width_mm = 148, .height_mm = 89,
	.type = DRM_MODE_TYPE_DRIVER,
	.expose_to_userspace = true,
	.name = "800x480x60",
	.status = MODE_OK,
};

static int mpro_conn_get_modes(struct drm_connector *connector)
{
	//struct mpro_device *mpro = to_mpro(connector -> dev);
	return drm_connector_helper_get_modes_fixed(connector, &mpro_display_mode);
}

static const struct drm_connector_helper_funcs mpro_conn_helper_funcs = {
	.get_modes = mpro_conn_get_modes,
};

static const struct drm_connector_funcs mpro_conn_funcs = {
	.fill_modes = drm_helper_probe_single_connector_modes,
	.destroy = drm_connector_cleanup,
	.reset = drm_atomic_helper_connector_reset,
	.atomic_duplicate_state = drm_atomic_helper_connector_duplicate_state,
	.atomic_destroy_state = drm_atomic_helper_connector_destroy_state,
};

static int mpro_conn_init(struct mpro_device *mpro)
{
	drm_connector_helper_add(&mpro->conn, &mpro_conn_helper_funcs);
	return drm_connector_init(&mpro->dev, &mpro->conn,
				  &mpro_conn_funcs, DRM_MODE_CONNECTOR_USB);
}

static void mpro_pipe_enable(struct drm_simple_display_pipe *pipe,
				 struct drm_crtc_state *crtc_state,
				 struct drm_plane_state *plane_state)
{
	struct mpro_device *mpro = to_mpro(pipe->crtc.dev);
	struct drm_shadow_plane_state *shadow_plane_state = to_drm_shadow_plane_state(plane_state);
	struct drm_framebuffer *fb = plane_state->fb;
	struct drm_rect rect = {
		.x1 = 0,
		.x2 = fb->width,
		.y1 = 0,
		.y2 = fb->height,
	};

	mpro_send_command(mpro, cmd_quit_sleep, sizeof(cmd_quit_sleep));
	mpro_fb_mark_dirty(&shadow_plane_state->data[0], fb, &rect);
}

static void mpro_pipe_disable(struct drm_simple_display_pipe *pipe)
{
	//struct mpro_device *mpro = to_mpro(pipe->crtc.dev);

}

static void mpro_pipe_update(struct drm_simple_display_pipe *pipe,
				 struct drm_plane_state *old_state)
{
	struct drm_plane_state *state = pipe->plane.state;
	struct drm_shadow_plane_state *shadow_plane_state = to_drm_shadow_plane_state(state);
	struct drm_framebuffer *fb = state->fb;
	struct drm_rect rect;

	if (!pipe->crtc.state->active)
		return;

	if (drm_atomic_helper_damage_merged(old_state, state, &rect))
		mpro_fb_mark_dirty(&shadow_plane_state->data[0], fb, &rect);
}

static const struct drm_simple_display_pipe_funcs mpro_pipe_funcs = {
	.enable	    = mpro_pipe_enable,
	.disable    = mpro_pipe_disable,
	.update	    = mpro_pipe_update,
	DRM_GEM_SIMPLE_DISPLAY_PIPE_SHADOW_PLANE_FUNCS,
};

static const uint32_t mpro_pipe_formats[] = {
	DRM_FORMAT_XRGB8888,
};

static const uint64_t mpro_pipe_modifiers[] = {
	DRM_FORMAT_MOD_LINEAR,
	DRM_FORMAT_MOD_INVALID
};

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

static const struct drm_format_info *mpro_format_info(const struct drm_mode_fb_cmd2 *mode_cmd) {
	return drm_format_info(DRM_FORMAT_XRGB8888);
}

static const struct drm_mode_config_funcs mpro_mode_config_funcs = {
	.fb_create = drm_gem_fb_create_with_dirty,
	.get_format_info = mpro_format_info,
	.atomic_check = drm_atomic_helper_check,
	.atomic_commit = drm_atomic_helper_commit,
};

static int mpro_get_screen(struct mpro_device *mpro)
{
	struct usb_device *udev = mpro_to_usb_device(mpro);
	void *cmd = mpro->cmd;
	int ret;

	ret = usb_control_msg(udev, usb_sndctrlpipe(udev, 0),
			      0xb5, 0x40, 0, 0,
			      (void *)cmd_get_screen, 5,
			      MPRO_MAX_DELAY);
	if (ret < 5)
		goto err;

	ret = usb_control_msg(udev, usb_rcvctrlpipe(udev, 0),
			      0xb6, 0xc0, 0, 0,
			      cmd, 1,
			      MPRO_MAX_DELAY);
	if (ret < 1)
		goto err;

	ret = usb_control_msg(udev, usb_rcvctrlpipe(udev, 0),
			      0xb7, 0xc0, 0, 0,
			      cmd, 5,
			      MPRO_MAX_DELAY);
	if (ret < 5)
		goto err;

	mpro->screen = ((unsigned int *)(cmd + 1))[0];
	return 0;
err:
	return -EIO;
}

static int mpro_get_version(struct mpro_device *mpro)
{
	struct usb_device *udev = mpro_to_usb_device(mpro);
	void *cmd = mpro->cmd;
	int ret;

	ret = usb_control_msg(udev, usb_sndctrlpipe(udev, 0),
			      0xb5, 0x40, 0, 0,
			      (void *)cmd_get_version, 5,
			      MPRO_MAX_DELAY);
	if (ret < 5)
		goto err;

	ret = usb_control_msg(udev, usb_rcvctrlpipe(udev, 0),
			      0xb6, 0xc0, 0, 0,
			      cmd, 1,
			      MPRO_MAX_DELAY);
	if (ret < 1)
		goto err;

	ret = usb_control_msg(udev, usb_rcvctrlpipe(udev, 0),
			      0xb7, 0xc0, 0, 0,
			      cmd, 5,
			      MPRO_MAX_DELAY);
	if (ret < 5)
		goto err;

	mpro->version = ((unsigned int *)(cmd + 1))[0];
	return 0;
err:
	return -EIO;
}

static int mpro_get_id(struct mpro_device *mpro)
{
	struct usb_device *udev = mpro_to_usb_device(mpro);
	void *cmd = mpro->cmd;
	int ret;

	ret = usb_control_msg(udev, usb_sndctrlpipe(udev, 0),
			      0xb5, 0x40, 0, 0,
			      (void *)cmd_get_id, 5,
			      MPRO_MAX_DELAY);
	if (ret < 5)
		goto err;

	ret = usb_control_msg(udev, usb_rcvctrlpipe(udev, 0),
			      0xb6, 0xc0, 0, 0,
			      cmd, 1,
			      MPRO_MAX_DELAY);
	if (ret < 1)
		goto err;

	ret = usb_control_msg(udev, usb_rcvctrlpipe(udev, 0),
			      0xb7, 0xc0, 0, 0,
			      cmd, 9,
			      MPRO_MAX_DELAY);
	if (ret < 5)
		goto err;

	memcpy(mpro->id, cmd + 1, 8);

	return 0;
err:
	return -EIO;
}

static void mpro_mode_config_setup(struct mpro_device *mpro)
{
	struct drm_device *dev = &mpro->dev;
	int width, height, margin, width_mm, height_mm;
	char *model = MODEL_DEFAULT;

	width  = 480;
	height = 800;
	margin = 0;
	width_mm = 0;
	height_mm = 0;

	switch (mpro->screen) {
	case 0x00000005:
		model = MODEL_5IN;
		height = 854;
		width_mm = 62;
		height_mm = 110;
		if (mpro->version != 0x00000003)
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
/*
static void mpro_edid_setup(struct mpro_device *mpro)
{
	unsigned int width, height, width_mm, height_mm;
	char buf[16];

	width = mpro->width;
	height = mpro->height;
	width_mm = mpro->width_mm;
	height_mm = mpro->height_mm;

	mpro_edid.detailed_timings[0].data.pixel_data.hactive_lo = width % 256;
	mpro_edid.detailed_timings[0].data.pixel_data.hactive_hblank_hi &= 0x0f;
	mpro_edid.detailed_timings[0].data.pixel_data.hactive_hblank_hi |= \
						((u8)(width / 256) << 4);

	mpro_edid.detailed_timings[0].data.pixel_data.vactive_lo = height % 256;
	mpro_edid.detailed_timings[0].data.pixel_data.vactive_vblank_hi &= 0x0f;
	mpro_edid.detailed_timings[0].data.pixel_data.vactive_vblank_hi |= \
						((u8)(height / 256) << 4);

	mpro_edid.detailed_timings[0].data.pixel_data.width_mm_lo = \
							width_mm % 256;
	mpro_edid.detailed_timings[0].data.pixel_data.height_mm_lo = \
							height_mm % 256;
	mpro_edid.detailed_timings[0].data.pixel_data.width_height_mm_hi = \
					((u8)(width_mm / 256) << 4) | \
					((u8)(height_mm / 256) & 0xf);

	memcpy(&mpro_edid.detailed_timings[2].data.other_data.data.str.str,
		mpro->model, strlen(mpro->model));

	snprintf(buf, 16, "%02X%02X%02X%02X\n",
		mpro->id[4], mpro->id[5], mpro->id[6], mpro->id[7]);

	memcpy(&mpro_edid.detailed_timings[3].data.other_data.data.str.str,
		buf, strlen(buf));

	mpro_edid.checksum = mpro_edid_block_checksum((u8*)&mpro_edid);
}
*/
static int mpro_usb_probe(struct usb_interface *interface,
			  const struct usb_device_id *id)
{
	struct mpro_device *mpro;
	struct drm_device *dev;
	int ret;

	mpro = devm_drm_dev_alloc(&interface->dev, &mpro_drm_driver,
				      struct mpro_device, dev);
	if (IS_ERR(mpro))
		return PTR_ERR(mpro);
	dev = &mpro->dev;

	mpro->dmadev = usb_intf_get_dma_device(to_usb_interface(dev->dev));
	if (!mpro->dmadev)
		drm_warn(dev, "buffer sharing not supported"); /* not an error */

	ret = mpro_get_screen(mpro);
	if (ret) {
		drm_err(dev, "cant't get screen info.\n");
		goto err_put_device;
	}

	ret = mpro_get_version(mpro);
	if (ret) {
		drm_err(dev, "cant't get screen version.\n");
		goto err_put_device;
	}

	ret = mpro_get_id(mpro);
	if (ret) {
		drm_err(dev, "cant't get screen id.\n");
		goto err_put_device;
	}

	ret = drmm_mode_config_init(dev);
	if (ret)
		goto err_put_device;

	mpro_mode_config_setup(mpro);

	ret = mpro_data_alloc(mpro);
	if (ret)
		goto err_put_device;

	ret = mpro_conn_init(mpro);
	if (ret)
		goto err_put_device;

	ret = drm_simple_display_pipe_init(&mpro->dev,
					   &mpro->pipe,
					   &mpro_pipe_funcs,
					   mpro_pipe_formats,
					   ARRAY_SIZE(mpro_pipe_formats),
					   mpro_pipe_modifiers,
					   &mpro->conn);
	if (ret)
		goto err_put_device;

	drm_mode_config_reset(dev);

	usb_set_intfdata(interface, dev);
	ret = drm_dev_register(dev, 0);
	if (ret)
		goto err_put_device;

	drm_fbdev_generic_setup(dev, 0);
	
	return 0;

err_put_device:
	put_device(mpro->dmadev);
	return ret;
}

static void mpro_usb_disconnect(struct usb_interface *interface)
{
	struct drm_device *dev = usb_get_intfdata(interface);
	struct mpro_device *mpro = to_mpro(dev);

	put_device(mpro->dmadev);
	mpro->dmadev = NULL;
	drm_dev_unplug(dev);
	drm_atomic_helper_shutdown(dev);
}

static const struct usb_device_id id_table[] = {
	{ USB_DEVICE(0x1908, 0x0x02) },
	{},
};
MODULE_DEVICE_TABLE(usb, id_table);

static struct usb_driver mpro_usb_driver = {
	.name = "ax206",
	.probe = ax206_usb_probe,
	.disconnect = ax206_usb_disconnect,
	.id_table = id_table,
};

module_usb_driver(ax206_usb_driver);
MODULE_AUTHOR("Oskari Rauta <oskari.rauta@gmail.com>");
MODULE_LICENSE("Dual MIT/GPL");
