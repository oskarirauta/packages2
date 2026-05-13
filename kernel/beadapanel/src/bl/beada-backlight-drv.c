// SPDX-License-Identifier: GPL-2.0-only
/*
 * beada-backlight-drv.c – BeadaPanel MFD child: backlight driver entry
 *
 * Owns the platform_driver registration and the probe/remove wiring.
 * All the actual work lives in the companion files:
 *
 *   beada-backlight-bl.c     Linux backlight class ops, hardware send,
 *                            software gamma curve
 *   beada-backlight-sysfs.c  "gamma" sysfs attribute
 *   beada-backlight-pm.c     Resume handler
 *
 * Probe sequence
 * ──────────────
 *   1.  Retrieve the MFD parent handle (set before our cell device is
 *       registered; the MFD parent guarantees this).
 *   2.  Read panel info; bail out if the device has no backlight
 *       control (max_brightness == 0).
 *   3.  Allocate per-device state.
 *   4.  Register the backlight class device.
 *   5.  Push the initial value (panel->current_brightness, possibly
 *       overridden by the MFD's initial_brightness module parameter)
 *       to hardware so the device's state matches what userspace reads.
 *   6.  Register the resume handler with the MFD parent.
 *   7.  Create the "gamma" sysfs attribute group.
 */

#include <linux/backlight.h>
#include <linux/device.h>
#include <linux/mod_devicetable.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/platform_device.h>
#include <linux/slab.h>

#include "../beada.h"
#include "beada-backlight.h"

/* ------------------------------------------------------------------ */
/* Probe / remove                                                       */
/* ------------------------------------------------------------------ */

static int beada_backlight_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct beada_mfd_dev *beada = dev_get_drvdata(dev->parent);
	const struct beada_panel_info *panel;
	struct backlight_properties props = {
		.type  = BACKLIGHT_RAW,
		.scale = BACKLIGHT_SCALE_LINEAR,
	};
	struct beada_backlight *mb;
	char name[64];
	int ret;

	if (!beada)
		return -ENODEV;

	panel = beada_get_panel_info(beada);
	if (!panel->max_brightness) {
		dev_info(dev,
			 "device reports no backlight control; skipping\n");
		return -ENODEV;
	}

	mb = devm_kzalloc(dev, sizeof(*mb), GFP_KERNEL);
	if (!mb)
		return -ENOMEM;

	mb->beada        = beada;
	mb->max_value    = panel->max_brightness;
	mb->stored_value = min(panel->current_brightness,
			       panel->max_brightness);
	mb->gamma_x100   = BEADA_BL_GAMMA_DEFAULT;
	mutex_init(&mb->lock);

	props.max_brightness = mb->max_value;
	props.brightness     = mb->stored_value;

	scnprintf(name, sizeof(name), "beada-%s", dev_name(dev->parent));

	mb->bl = devm_backlight_device_register(dev, name, dev, mb,
						&beada_bl_ops, &props);
	if (IS_ERR(mb->bl))
		return dev_err_probe(dev, PTR_ERR(mb->bl),
				     "backlight register failed\n");

	platform_set_drvdata(pdev, mb);

	/*
	 * Don't push the initial value to hardware here.  The device
	 * firmware refuses SL_SET_BL until Panel-Link has received at
	 * least one frame after a PL_RESET, and no frame has been sent
	 * yet at probe time.  The hardware will get our value via one
	 * of the two natural paths:
	 *
	 *   - The user explicitly writes /sys/class/backlight/.../brightness
	 *     after opening a DRM client (frames are flowing, firmware
	 *     accepts SL_SET_BL).
	 *
	 *   - On USB resume, the MFD parent's resume callback re-sends
	 *     the most recent rendered frame via the DRM resume handler,
	 *     then calls our resume handler which pushes stored_value via
	 *     beada_bl_send().  The frame flush between handlers ensures
	 *     the firmware has accepted Panel-Link before the SL_SET_BL.
	 *
	 * Until either of those happens, the device runs at its firmware
	 * default brightness (typically full).  This matches what users
	 * see anyway — the panel is showing its idle clock at full
	 * brightness until something opens it.
	 */

	/*
	 * Register the resume handler BEFORE the sysfs attribute group:
	 * if sysfs creation fails we're going to bail out, and the unwind
	 * needs to unregister the handler.  Doing them in this order keeps
	 * the failure path obvious.
	 */
	mb->resume.handler = beada_bl_resume_handler;
	mb->resume.priv    = mb;
	beada_register_resume_handler(beada, &mb->resume);

	ret = beada_bl_sysfs_init(mb);
	if (ret) {
		beada_unregister_resume_handler(beada, &mb->resume);
		return dev_err_probe(dev, ret,
				     "sysfs gamma group create failed\n");
	}

	dev_info(dev, "backlight ready: %s (0..%u, initial %u)\n",
		 name, mb->max_value, mb->stored_value);
	return 0;
}

static void beada_backlight_remove(struct platform_device *pdev)
{
	struct beada_backlight *mb = platform_get_drvdata(pdev);

	if (!mb)
		return;

	/*
	 * Stop receiving resume notifications BEFORE tearing anything
	 * else down, so a concurrent MFD resume can't fire a handler
	 * into half-released state.
	 */
	beada_unregister_resume_handler(mb->beada, &mb->resume);

	beada_bl_sysfs_fini(mb);

	/* devm releases backlight_device_unregister() */
}

/* ------------------------------------------------------------------ */
/* Platform driver registration                                         */
/* ------------------------------------------------------------------ */

static const struct platform_device_id beada_backlight_id_table[] = {
	{ BEADA_CELL_BACKLIGHT, 0 },
	{ }
};
MODULE_DEVICE_TABLE(platform, beada_backlight_id_table);

static struct platform_driver beada_backlight_driver = {
	.probe    = beada_backlight_probe,
	.remove   = beada_backlight_remove,
	.id_table = beada_backlight_id_table,
	.driver   = {
		.name = "beada-bl",
		/*
		 * No .pm — see beada-backlight.h header comment.
		 * Suspend/resume runs through the MFD parent's resume
		 * handler dispatch instead.
		 */
	},
};

module_platform_driver(beada_backlight_driver);

MODULE_AUTHOR("Oskari Rauta <oskari.rauta@gmail.com>");
MODULE_DESCRIPTION("BeadaPanel USB display – backlight child driver");
MODULE_LICENSE("GPL");
