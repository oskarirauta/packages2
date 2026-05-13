// SPDX-License-Identifier: GPL-2.0-only
/*
 * ax206-drm-drv.c - AX206 digital photo frame DRM child driver
 *
 * Module parameters (also settable on the kernel command line):
 *   rotation=<0-7>        initial rotation mode (default: 0)
 *   noblank=<0|1>         keep last image when pipe closes (default: 0)
 *   fbcon=<0|1>           enable fbcon (default: 0)
 *   idle_timeout=<sec>    virtual idle timeout in seconds, 0=off (default: 0)
 *
 * fbcon notes:
 *   When fbcon is enabled it keeps the DRM pipe open, which prevents
 *   the noblank/idle logic from triggering on pipe close.  The idle
 *   timer still works normally (fbcon will be blanked along with the
 *   display when idle fires).
 *
 * Copyright (C) 2025 Oskari Rauta <oskari.rauta@gmail.com>
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/timer.h>
#include <linux/workqueue.h>

#include <drm/drm_atomic_helper.h>
#include <drm/drm_damage_helper.h>
#include <drm/drm_drv.h>
#include <drm/drm_fb_helper.h>
#include <drm/drm_format_helper.h>
#include <drm/drm_fourcc.h>
#include <drm/drm_gem_atomic_helper.h>
#include <drm/drm_gem_framebuffer_helper.h>
#include <drm/drm_gem_shmem_helper.h>
#include <drm/drm_managed.h>
#include <drm/drm_modes.h>
#include <drm/drm_probe_helper.h>
#include <drm/drm_simple_kms_helper.h>
#include <drm/clients/drm_client_setup.h>

#include "ax206.h"
#include "ax206-drm-priv.h"

/* ----- Module parameters ----- */

static int rotation_param = AX206_ROTATE_0;
module_param_named(rotation, rotation_param, int, 0444);
MODULE_PARM_DESC(rotation,
		 "Initial rotation: 0=0°, 1=90°, 2=180°, 3=270°, "
		 "4-7=same with horizontal flip (default: 0)");

static bool noblank_param;
module_param_named(noblank, noblank_param, bool, 0444);
MODULE_PARM_DESC(noblank,
		 "Keep last image when DRM pipe closes (default: 0)");

static bool fbcon_param;
module_param_named(fbcon, fbcon_param, bool, 0444);
MODULE_PARM_DESC(fbcon,
		 "Enable fbcon on this display (default: 0). "
		 "fbcon keeps the pipe open so pipe-close blank/idle "
		 "does not trigger on console switch.");

static unsigned int idle_timeout_param = AX206_IDLE_TIMEOUT_DEFAULT;
module_param_named(idle_timeout, idle_timeout_param, uint, 0444);
MODULE_PARM_DESC(idle_timeout,
		 "Virtual idle timeout in seconds before display blanks "
		 "(0 = disabled, default: 0). Can also be changed at "
		 "runtime via sysfs idle_timeout_ms.");

/* ----- DRM driver ----- */

DEFINE_DRM_GEM_FOPS(ax206_drm_fops);

static const struct drm_driver ax206_drm_driver = {
	.driver_features	= DRIVER_GEM | DRIVER_MODESET | DRIVER_ATOMIC,
	.fops			= &ax206_drm_fops,
	DRM_GEM_SHMEM_DRIVER_OPS,
	.name			= "ax206",
	.desc			= "AX206 digital photo frame",
	.major			= 1,
	.minor			= 0,
};

/* ----- Connector ----- */

static int ax206_connector_get_modes(struct drm_connector *connector)
{
	struct ax206_drm_dev *d = container_of(connector, struct ax206_drm_dev,
					     connector);
	struct drm_display_mode *mode;

	mode = drm_mode_duplicate(connector->dev, &d->mode);
	if (!mode)
		return 0;

	drm_mode_set_name(mode);
	mode->type = DRM_MODE_TYPE_DRIVER | DRM_MODE_TYPE_PREFERRED;
	drm_mode_probed_add(connector, mode);
	return 1;
}

static const struct drm_connector_helper_funcs ax206_connector_helper_funcs = {
	.get_modes = ax206_connector_get_modes,
};

static const struct drm_connector_funcs ax206_connector_funcs = {
	.reset			= drm_atomic_helper_connector_reset,
	.fill_modes		= drm_helper_probe_single_connector_modes,
	.destroy		= drm_connector_cleanup,
	.atomic_duplicate_state	= drm_atomic_helper_connector_duplicate_state,
	.atomic_destroy_state	= drm_atomic_helper_connector_destroy_state,
};

/* ----- Mode config ----- */

static const struct drm_mode_config_funcs ax206_mode_config_funcs = {
	.fb_create     = drm_gem_fb_create_with_dirty,
	.atomic_check  = drm_atomic_helper_check,
	.atomic_commit = drm_atomic_helper_commit,
};

/* ----- Simple display pipe ----- */

static const struct drm_simple_display_pipe_funcs ax206_pipe_funcs = {
	.enable		= ax206_drm_pipe_enable,
	.disable	= ax206_drm_pipe_disable,
	.update		= ax206_drm_pipe_update,
	DRM_GEM_SIMPLE_DISPLAY_PIPE_SHADOW_PLANE_FUNCS,
};

static const uint32_t ax206_formats[] = {
	DRM_FORMAT_RGB565,
	DRM_FORMAT_XRGB8888,
};

static const uint64_t ax206_format_modifiers[] = {
	DRM_FORMAT_MOD_LINEAR,
	DRM_FORMAT_MOD_INVALID,
};

static struct drm_display_mode ax206_make_mode(u16 w, u16 h)
{
	struct drm_display_mode mode = {
		DRM_SIMPLE_MODE(w, h, 80, 60),
	};
	return mode;
}

/* ----- Probe / remove ----- */

static int ax206_drm_probe(struct platform_device *pdev)
{
	struct ax206_dev *dpf = dev_get_drvdata(pdev->dev.parent);
	struct ax206_drm_dev *d;
	struct drm_device *drm;
	u16 lw, lh;
	int ret;

	d = devm_drm_dev_alloc(&pdev->dev, &ax206_drm_driver,
				struct ax206_drm_dev, drm);
	if (IS_ERR(d))
		return PTR_ERR(d);

	drm    = &d->drm;
	d->dpf = dpf;

	/* Rotation */
	if (rotation_param < 0 || rotation_param >= AX206_ROTATE_COUNT) {
		dev_warn(&pdev->dev, "ax206-drm: invalid rotation %d, using 0\n",
			 rotation_param);
		rotation_param = AX206_ROTATE_0;
	}
	ax206_drm_set_rotation(d, rotation_param);
	d->noblank = noblank_param;

	lw = d->lwidth;
	lh = d->lheight;

	d->mode = ax206_make_mode(lw, lh);

	/* Shadow buffer (physical dimensions, big-endian RGB565) */
	d->shadow_size = (size_t)dpf->width * dpf->height * AX206_BPP;
	d->shadow_buf  = devm_kzalloc(&pdev->dev, d->shadow_size, GFP_KERNEL);
	if (!d->shadow_buf)
		return -ENOMEM;

	/* Coalescing blit engine */
	spin_lock_init(&d->blit_lock);
	d->blit_dirty  = false;
	d->blit_active = false;
	d->dirty_x0    = dpf->width;
	d->dirty_y0    = dpf->height;
	d->dirty_x1    = 0;
	d->dirty_y1    = 0;
	INIT_WORK(&d->blit_work, ax206_drm_blit_work_fn);

	/* Statistics */
	ax206_stats_init(&d->stats);

	/* Idle timer */
	timer_setup(&d->idle_timer, ax206_idle_timer_fn, 0);
	INIT_WORK(&d->idle_work,   ax206_idle_work_fn);
	INIT_WORK(&d->resume_work, ax206_resume_work_fn);
	d->idle_timeout_s = idle_timeout_param;
	d->idle_active    = false;

	/* DRM mode config */
	ret = drmm_mode_config_init(drm);
	if (ret)
		return ret;

	drm->mode_config.min_width  = lw;
	drm->mode_config.max_width  = lw;
	drm->mode_config.min_height = lh;
	drm->mode_config.max_height = lh;
	drm->mode_config.funcs      = &ax206_mode_config_funcs;

	/* Connector */
	ret = drm_connector_init(drm, &d->connector, &ax206_connector_funcs,
				 DRM_MODE_CONNECTOR_USB);
	if (ret)
		return ret;

	drm_connector_helper_add(&d->connector, &ax206_connector_helper_funcs);
	d->connector.display_info.width_mm  = 80;
	d->connector.display_info.height_mm = 60;

	/* Simple display pipe */
	ret = drm_simple_display_pipe_init(drm, &d->pipe, &ax206_pipe_funcs,
					   ax206_formats, ARRAY_SIZE(ax206_formats),
					   ax206_format_modifiers, &d->connector);
	if (ret)
		return ret;

	drm_plane_enable_fb_damage_clips(&d->pipe.plane);
	drm_mode_config_reset(drm);

	platform_set_drvdata(pdev, d);

	ret = ax206_drm_sysfs_init(d);
	if (ret)
		return ret;

	ret = drm_dev_register(drm, 0);
	if (ret) {
		ax206_drm_sysfs_fini(d);
		return ret;
	}

	/*
	 * fbcon is optional and off by default.  This display is slow and
	 * not well-suited for terminal use, but fbcon can be useful for
	 * testing (fbcon=1 module parameter or kernel cmdline).
	 */
	if (fbcon_param)
		drm_client_setup(drm, NULL);

	dev_info(&pdev->dev,
		 "ax206-drm: %ux%u rotation=%s noblank=%d fbcon=%d idle=%us\n",
		 lw, lh, ax206_rotation_name(d->rotation),
		 d->noblank, fbcon_param, d->idle_timeout_s);

	return 0;
}

static void ax206_drm_remove(struct platform_device *pdev)
{
	struct ax206_drm_dev *d = platform_get_drvdata(pdev);

	timer_delete_sync(&d->idle_timer);
	cancel_work_sync(&d->idle_work);
	cancel_work_sync(&d->resume_work);
	cancel_work_sync(&d->blit_work);

	ax206_drm_sysfs_fini(d);
	drm_dev_unplug(&d->drm);
	drm_atomic_helper_shutdown(&d->drm);
}

static struct platform_driver ax206_drm_platform_driver = {
	.probe  = ax206_drm_probe,
	.remove = ax206_drm_remove,
	.driver = { .name = "ax206-drm" },
};

module_platform_driver(ax206_drm_platform_driver);

MODULE_AUTHOR("Oskari Rauta <oskari.rauta@gmail.com>");
MODULE_DESCRIPTION("AX206 (DPF) digital photo frame DRM driver");
MODULE_LICENSE("GPL");
