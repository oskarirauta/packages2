// SPDX-License-Identifier: GPL-2.0-only
/*
 * ax206-bl-drv.c - AX206 digital photo frame backlight child driver
 *
 * Implements the Linux backlight class device for the DPF-AX display.
 * All USB writes are submitted through the MFD parent workqueue so
 * they never block the caller and never race with DRM blits.
 *
 * Brightness range: 0 (off) .. 7 (maximum).
 *
 * During the virtual power-save period the brightness value is stored
 * but not applied to hardware; the DRM resume path will call
 * backlight_get_brightness() and restore it.
 *
 * Copyright (C) 2025 Oskari Rauta <oskari.rauta@gmail.com>
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/backlight.h>
#include <linux/slab.h>
#include <linux/workqueue.h>

#include "ax206.h"
#include "ax206-bl.h"

/* ----- workqueue item for backlight USB writes ----- */

struct ax206_bl_work {
	struct work_struct	work;
	struct ax206_dev		*dpf;
	int			value;
};

static void ax206_bl_work_fn(struct work_struct *work)
{
	struct ax206_bl_work *w = container_of(work, struct ax206_bl_work, work);

	if (w->dpf->connected && !w->dpf->blanked)
		ax206_usb_set_backlight(w->dpf, w->value);

	kfree(w);
}

/* ----- backlight class ops ----- */

static int ax206_bl_update_status(struct backlight_device *bd)
{
	struct ax206_dev *dpf = bl_get_data(bd);
	int brightness = backlight_get_brightness(bd);
	struct ax206_bl_work *w;

	/* Always store the requested level in the parent so resume can use it */
	dpf->backlight = brightness;

	if (!dpf->connected)
		return 0;

	w = kmalloc(sizeof(*w), GFP_KERNEL);
	if (!w)
		return -ENOMEM;

	INIT_WORK(&w->work, ax206_bl_work_fn);
	w->dpf   = dpf;
	w->value = brightness;

	queue_work(dpf->wq, &w->work);
	return 0;
}

int ax206_bl_get_brightness(struct backlight_device *bd)
{
	struct ax206_dev *dpf = bl_get_data(bd);

	return dpf->backlight;
}
EXPORT_SYMBOL_GPL(ax206_bl_get_brightness);

static const struct backlight_ops ax206_bl_ops = {
	.update_status	= ax206_bl_update_status,
	.get_brightness	= ax206_bl_get_brightness,
};

/* ----- platform driver ----- */

static int ax206_bl_probe(struct platform_device *pdev)
{
	struct ax206_dev *dpf = dev_get_drvdata(pdev->dev.parent);
	struct backlight_properties props = {};
	struct backlight_device *bd;

	props.type		= BACKLIGHT_RAW;
	props.max_brightness	= AX206_BL_MAX;
	props.brightness	= dpf->backlight;
	props.power		= BACKLIGHT_POWER_ON;

	bd = devm_backlight_device_register(&pdev->dev, "ax206-bl",
					    &pdev->dev, dpf,
					    &ax206_bl_ops, &props);
	if (IS_ERR(bd))
		return PTR_ERR(bd);

	platform_set_drvdata(pdev, bd);

	dev_info(&pdev->dev, "ax206-bl: backlight registered (max=%d)\n",
		 AX206_BL_MAX);
	return 0;
}

static struct platform_driver ax206_bl_driver = {
	.probe  = ax206_bl_probe,
	.driver = {
		.name = "ax206-bl",
	},
};

module_platform_driver(ax206_bl_driver);

MODULE_AUTHOR("Oskari Rauta <oskari.rauta@gmail.com>");
MODULE_DESCRIPTION("AX206 (DPF) digital photo frame backlight driver");
MODULE_LICENSE("GPL");
