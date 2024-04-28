#ifndef _MPRO_H_
#define _MPRO_H_

#include <drm/drm_connector.h>
#include <drm/drm_drv.h>
#include <drm/drm_simple_kms_helper.h>

#include <linux/usb.h>
#include <linux/input.h>
#include <linux/usb/input.h>
#include <linux/iosys-map.h>

#if IS_ENABLED(CONFIG_BACKLIGHT_CLASS_DEVICE) || IS_ENABLED(CONFIG_FB_BACKLIGHT)
#include <linux/backlight.h>
#endif

#define MPRO_BPP		16
#define MPRO_MAX_DELAY		100

#define to_mpro(__dev) container_of(__dev, struct mpro_device, dev)

struct mpro_device {
	struct drm_device		dev;
	struct device			*dmadev;
	struct drm_simple_display_pipe	pipe;
	struct drm_connector		conn;
	unsigned int			screen;
	unsigned int			version;
	unsigned char			id[8];
	char				*model;
	unsigned int			width;
	unsigned int			height;
	unsigned int			width_mm;
	unsigned int			height_mm;
	unsigned int			margin;
	unsigned char			cmd[64];
	unsigned char			*data;
	unsigned int			data_size;

	struct {
		struct input_dev	*input;
		char			name[64];
		char			phys[64];
		struct urb		*irq;
		unsigned char		*data;
		dma_addr_t		dma;
		int			data_size;
	} touch;

#if IS_ENABLED(CONFIG_BACKLIGHT_CLASS_DEVICE) || IS_ENABLED(CONFIG_FB_BACKLIGHT)
	int				bl_power;
#endif
	char				partial;
};

extern struct usb_device *mpro_to_usb_device(struct mpro_device *mpro);

extern void mpro_edid_setup(struct mpro_device *mpro);
extern int mpro_conn_init(struct mpro_device *mpro);
extern int mpro_pipe_init(struct mpro_device *mpro);

extern int mpro_get_screen(struct mpro_device *mpro);
extern int mpro_get_version(struct mpro_device *mpro);
extern int mpro_get_id(struct mpro_device *mpro);
extern void mpro_mode_config_setup(struct mpro_device *mpro);

extern void mpro_touch_init(struct mpro_device *mpro);
extern void mpro_touch_exit(struct mpro_device *mpro);

extern void mpro_bl_init(struct mpro_device* mpro);

extern int mpro_sysfs_init(struct mpro_device* mpro);

extern int mpro_send_command(struct mpro_device *mpro, void* cmd, unsigned int len);
extern void mpro_fb_mark_dirty(struct iosys_map *src,
				struct drm_framebuffer *fb,
				struct drm_rect *rect);

#endif
