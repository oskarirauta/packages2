// SPDX-License-Identifier: GPL-2.0-only
/*
 * beada-drm-drv.c – BeadaPanel DRM child driver: entry point
 *
 * Owns the platform_driver, the drm_driver, and the module parameters.
 * All other DRM logic lives in companion files (see beada-drm.h for the
 * architecture overview).
 *
 * Power management
 * ────────────────
 * The DRM child does NOT register platform_driver .pm callbacks.
 * Suspend/resume coordination happens at the MFD parent level: the
 * parent's USB suspend/resume callbacks (beada-pm.c) flush the
 * workqueue on the way down and, on the way back up, re-arm the
 * Panel-Link receiver and invoke each child's registered resume
 * handler.  The DRM resume handler (beada-drm-pm.c) re-sends the
 * shadow buffer to hardware so the image is back on screen by the
 * time userspace unfreezes.
 *
 * USB autosuspend is gated by the pipe lifecycle: pipe_enable holds
 * an autopm reference (beada_pipe_active), pipe_disable releases it
 * (beada_pipe_idle).  So an enabled pipe keeps the device powered
 * even when the compositor is sitting on a dim screen with no
 * commits, and a disabled pipe lets the device autosuspend.
 *
 * Probe sequence
 * ──────────────
 *   1.  Retrieve the MFD parent handle (set before our cell device is
 *       registered; the MFD parent guarantees this).
 *   2.  Allocate the drm_device (embedded in beada_drm_dev).
 *   3.  Compute initial logical width/height for ROTATE_0.
 *   4.  Allocate shadow + blank buffers (drmm-managed → auto-freed).
 *   5.  Initialise vblank state (hrtimer, lock, period).
 *   6.  Call beada_kms_init() which sets up mode_config, pipe,
 *       connector, gamma LUT, and the initial EDID.
 *   7.  Register the resume handler with the MFD parent.
 *   8.  Register the DRM device.
 *   9.  Create the sysfs attribute group.
 *  10.  Optionally create an fbdev client (off by default).
 */

#include <linux/mod_devicetable.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/version.h>

#include <drm/drm_atomic_helper.h>
#include <drm/drm_drv.h>
#include <drm/drm_gem_shmem_helper.h>
#include <drm/drm_managed.h>

#include "../beada.h"
#include "beada-drm.h"

/*
 * fbdev API changed between kernels:
 *   < 6.17  drm_fbdev_shmem_setup(dev, preferred_bpp)
 *   ≥ 6.17  drm_client_setup(dev, format)
 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 17, 0)
#include <drm/clients/drm_client_setup.h>
#define beada_fbdev_setup(drm)	drm_client_setup((drm), NULL)
#else
#include <drm/drm_fbdev_shmem.h>
#define beada_fbdev_setup(drm)	drm_fbdev_shmem_setup((drm), 0)
#endif

/* ------------------------------------------------------------------ */
/* Module parameters                                                    */
/* ------------------------------------------------------------------ */

bool beada_noblank;
module_param_named(noblank, beada_noblank, bool, 0644);
MODULE_PARM_DESC(noblank,
	"Skip the all-zero frame sent at pipe_disable.  When set, the last "
	"image stays visible after the DRM client exits (useful for "
	"persistent-display use cases).  Default: 0 (blank on disable).");

static bool enable_fbdev;
module_param(enable_fbdev, bool, 0444);
MODULE_PARM_DESC(enable_fbdev,
	"Create a legacy /dev/fb* framebuffer (default: 0).  fbdev keeps "
	"the display continuously active, preventing power-save and "
	"shortening backlight lifetime.");

/* ------------------------------------------------------------------ */
/* drm_driver                                                           */
/* ------------------------------------------------------------------ */

DEFINE_DRM_GEM_FOPS(beada_drm_fops);

static const struct drm_driver beada_drm_driver = {
	.driver_features = DRIVER_MODESET | DRIVER_GEM | DRIVER_ATOMIC,

	.name  = BEADA_DRM_NAME,
	.desc  = BEADA_DRM_DESC,
	.major = BEADA_DRM_MAJOR,
	.minor = BEADA_DRM_MINOR,

	.fops  = &beada_drm_fops,
	DRM_GEM_SHMEM_DRIVER_OPS,
};

/* ------------------------------------------------------------------ */
/* Probe / remove / shutdown                                            */
/* ------------------------------------------------------------------ */

static int beada_drm_probe(struct platform_device *pdev)
{
	struct beada_mfd_dev *beada;
	const struct beada_panel_info *panel;
	struct beada_drm_dev *bdev;
	size_t native_bytes;
	int ret;

	/* ── 1. MFD parent handle ────────────────────────────────────── */
	beada = dev_get_drvdata(pdev->dev.parent);
	if (!beada) {
		dev_err(&pdev->dev, "no MFD parent data\n");
		return -ENODEV;
	}
	panel = beada_get_panel_info(beada);

	/* ── 2. Allocate DRM device ──────────────────────────────────── */
	bdev = devm_drm_dev_alloc(&pdev->dev, &beada_drm_driver,
				  struct beada_drm_dev, drm);
	if (IS_ERR(bdev))
		return PTR_ERR(bdev);

	bdev->beada = beada;
	bdev->panel = panel;
	platform_set_drvdata(pdev, bdev);

	/* ── 3. Initial logical dimensions: ROTATE_0 = native ────────── */
	bdev->rotation = DRM_MODE_ROTATE_0;
	bdev->width    = panel->width;
	bdev->height   = panel->height;
	bdev->noblank  = beada_noblank;   /* default from module param */

	/* ── 4. Shadow + blank buffers (always native panel size) ────── */
	native_bytes = (size_t)panel->width * panel->height * 2;

	bdev->shadow_buf = drmm_kmalloc(&bdev->drm, native_bytes, GFP_KERNEL);
	bdev->blank_buf  = drmm_kzalloc(&bdev->drm, native_bytes, GFP_KERNEL);
	if (!bdev->shadow_buf || !bdev->blank_buf)
		return -ENOMEM;

	bdev->shadow_size  = native_bytes;
	bdev->shadow_valid = false;
	mutex_init(&bdev->shadow_lock);
	mutex_init(&bdev->lut_lock);

	/* ── 5. Vblank state (timer created, not yet started) ────────── */
	beada_vblank_init(bdev);

	/* ── 6. KMS setup ────────────────────────────────────────────── */
	ret = beada_kms_init(bdev);
	if (ret) {
		dev_err(&pdev->dev, "KMS init failed: %d\n", ret);
		return ret;
	}

	/* ── 7. Register resume handler with MFD parent ──────────────── */
	bdev->resume.handler = beada_drm_resume_handler;
	bdev->resume.priv    = bdev;
	beada_register_resume_handler(beada, &bdev->resume);

	/* ── 8. Register DRM device ──────────────────────────────────── */
	ret = drm_dev_register(&bdev->drm, 0);
	if (ret) {
		dev_err(&pdev->dev, "drm_dev_register failed: %d\n", ret);
		beada_unregister_resume_handler(beada, &bdev->resume);
		return ret;
	}

	/* ── 9. Sysfs ────────────────────────────────────────────────── */
	ret = beada_sysfs_init(bdev);
	if (ret)
		dev_warn(&pdev->dev,
			 "sysfs init failed: %d; continuing\n", ret);

	/* ── 10. Optional fbdev (opt-in) ─────────────────────────────── */
	if (enable_fbdev)
		beada_fbdev_setup(&bdev->drm);

	drm_info(&bdev->drm,
		 "BeadaPanel %s DRM driver ready (%ux%u, fbdev=%s)\n",
		 panel->model, panel->width, panel->height,
		 enable_fbdev ? "on" : "off");

	return 0;
}

static void beada_drm_remove(struct platform_device *pdev)
{
	struct beada_drm_dev *bdev = platform_get_drvdata(pdev);

	/*
	 * Stop receiving resume notifications BEFORE tearing down the
	 * DRM device, so a concurrent MFD resume can't fire a handler
	 * into half-released state.
	 */
	beada_unregister_resume_handler(bdev->beada, &bdev->resume);

	beada_sysfs_fini(bdev);

	drm_dev_unplug(&bdev->drm);
	drm_atomic_helper_shutdown(&bdev->drm);
}

static void beada_drm_shutdown(struct platform_device *pdev)
{
	struct beada_drm_dev *bdev = platform_get_drvdata(pdev);

	drm_atomic_helper_shutdown(&bdev->drm);
}

/* ------------------------------------------------------------------ */
/* Platform driver registration                                         */
/* ------------------------------------------------------------------ */

static const struct platform_device_id beada_drm_id_table[] = {
	{ BEADA_CELL_DRM, 0 },
	{ }
};
MODULE_DEVICE_TABLE(platform, beada_drm_id_table);

static struct platform_driver beada_drm_platform_driver = {
	.probe    = beada_drm_probe,
	.remove   = beada_drm_remove,
	.shutdown = beada_drm_shutdown,
	.id_table = beada_drm_id_table,
	.driver   = {
		.name = "beada-drm",
		/*
		 * No .pm — see file header.  Suspend/resume runs through
		 * the MFD parent's resume handler dispatch instead.
		 */
	},
};

module_platform_driver(beada_drm_platform_driver);

MODULE_AUTHOR("Oskari Rauta <oskari.rauta@gmail.com>");
MODULE_DESCRIPTION("BeadaPanel USB display – DRM child module");
MODULE_LICENSE("GPL");
