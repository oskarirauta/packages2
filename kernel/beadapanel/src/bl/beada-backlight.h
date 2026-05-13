/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * beada-backlight.h – BeadaPanel backlight child driver: private definitions
 *
 * Only included by the backlight child's own .c files.
 * The MFD parent and the DRM child never see this file.
 *
 * Architecture
 * ────────────
 * beada-backlight-drv.c    platform_driver, probe/remove, module info
 * beada-backlight-bl.c     Linux backlight class ops, hardware send,
 *                          software gamma curve
 * beada-backlight-sysfs.c  "gamma" sysfs attribute (per-device)
 * beada-backlight-pm.c     Resume handler registered with the MFD parent
 *
 * Power management
 * ────────────────
 * No platform_driver .pm callbacks.  The MFD parent's USB resume
 * callback dispatches our registered resume handler (beada-backlight-pm.c)
 * after the bus is back up, and the handler re-pushes the user's last
 * brightness to hardware.
 *
 * Brightness model
 * ────────────────
 * The device reports its accepted maximum in panel->max_brightness; we
 * expose that range to userspace 1:1 (no software rescaling at the class
 * level).  An optional software gamma curve, configurable via sysfs,
 * applies a perceptual remapping before the value is sent to hardware
 * (the curve is computed in a normalised 0..max_brightness space; see
 * beada-backlight-bl.c for the arithmetic).
 *
 * Initial value
 * ─────────────
 * The MFD parent reports panel->current_brightness, which is either the
 * value read at GET_INFO time or the value forced by the MFD's
 * initial_brightness module parameter.  Our probe pushes that value to
 * hardware so the device's actual state matches what userspace will
 * read back via /sys/class/backlight/<name>/brightness.
 */

#ifndef __BEADA_BACKLIGHT_H
#define __BEADA_BACKLIGHT_H

#include <linux/backlight.h>
#include <linux/mutex.h>
#include <linux/types.h>

#include "../beada.h"

/* ------------------------------------------------------------------ */
/* Gamma curve range                                                    */
/* ------------------------------------------------------------------ */

/*
 * Software gamma applied to the user-visible brightness value before it
 * is sent to the device.  Human perception of brightness is roughly
 * logarithmic, so a linear 0..max ramp feels like "nothing happens" at
 * the low end and then the panel suddenly jumps to full bright.  A gamma
 * curve > 1 redistributes the steps so the perceived change is more
 * uniform.
 *
 * gamma_x100 is stored as the exponent × 100 (avoids floating point):
 *   100 → linear (no correction)
 *   220 → typical "perceptual" curve
 *   400 → very aggressive emphasis on low values
 */
#define BEADA_BL_GAMMA_DEFAULT	100	/* 1.00, linear */
#define BEADA_BL_GAMMA_MIN	 50	/* 0.50 */
#define BEADA_BL_GAMMA_MAX	400	/* 4.00 */

/* ------------------------------------------------------------------ */
/* Per-device state                                                     */
/* ------------------------------------------------------------------ */

/**
 * struct beada_backlight - Backlight child per-device state
 *
 * @beada:        MFD parent handle, used for beada_set_brightness().
 * @bl:           Linux backlight class device exposed to userspace.
 * @max_value:    Cached @panel->max_brightness; the device's accepted
 *                upper bound for SL_SET_BL.  Also reported to userspace
 *                via backlight_properties.max_brightness.
 * @gamma_x100:   Software gamma curve exponent × 100.  Accessed under
 *                READ_ONCE / WRITE_ONCE so the sysfs writer and the
 *                update path stay consistent without taking @lock.
 *
 * @lock:         Serialises @stored_value updates against the userspace
 *                backlight_class callbacks and the resume handler.  Also
 *                held while sending a value to the device so two
 *                concurrent writes from userspace can't reorder on the
 *                wire.
 * @stored_value: The user's most recent request, in the device's native
 *                0..@max_value scale (gamma is applied on the way out,
 *                not on the way in).  Re-sent by the resume handler so
 *                the user-visible brightness doesn't change across a
 *                suspend cycle.
 *
 * @resume:       Resume handler struct registered with the MFD parent at
 *                probe; unregistered at remove.
 */
struct beada_backlight {
	struct beada_mfd_dev		*beada;
	struct backlight_device		*bl;
	u8				 max_value;
	u32				 gamma_x100;

	struct mutex			 lock;
	u8				 stored_value;

	struct beada_resume_handler	 resume;
};

/* ------------------------------------------------------------------ */
/* Cross-file prototypes, grouped by source file                        */
/* ------------------------------------------------------------------ */

/* beada-backlight-bl.c */

/**
 * beada_bl_send() - Push a value to the device through the gamma curve
 * @mb:  Per-device state
 * @raw: User-requested level in the device's native 0..@max_value range
 *
 * Caller MUST hold @mb->lock.  Applies the software gamma curve, scales
 * the result back into 0..@max_value, and forwards to the MFD parent's
 * beada_set_brightness().  Errors are returned to the caller but only
 * informational — userspace has no useful action on a USB failure of a
 * backlight write.
 */
int beada_bl_send(struct beada_backlight *mb, u8 raw);

/** backlight class ops table — defined in beada-backlight-bl.c */
extern const struct backlight_ops beada_bl_ops;

/* beada-backlight-sysfs.c */

/**
 * beada_bl_sysfs_init() / beada_bl_sysfs_fini() - "gamma" sysfs attribute
 *
 * Creates / removes the per-device "gamma" attribute group on the
 * backlight class device.  Decoupled from sysfs_create_group() in
 * probe() so the drv.c stays free of attribute table boilerplate.
 */
int  beada_bl_sysfs_init(struct beada_backlight *mb);
void beada_bl_sysfs_fini(struct beada_backlight *mb);

/* beada-backlight-pm.c */

/**
 * beada_bl_resume_handler() - Re-push the stored brightness after USB resume
 *
 * Registered with the MFD parent at probe via
 * beada_register_resume_handler().  The parent invokes it from
 * beada_usb_resume() after Panel-Link has been re-armed.  Just re-runs
 * the most recent user request through the gamma curve and sends it.
 */
void beada_bl_resume_handler(void *priv);

#endif /* __BEADA_BACKLIGHT_H */
