// SPDX-License-Identifier: GPL-2.0-only
/*
 * beada-backlight-pm.c – Resume handler for the backlight child
 *
 * Single entry point: beada_bl_resume_handler() is registered with the
 * MFD parent at probe and called from beada_usb_resume() after the bus
 * is back up and Panel-Link has been re-armed.
 *
 * What we do here
 * ───────────────
 * Re-send the user's last brightness through the gamma curve.  The
 * device's backlight state is lost across the USB suspend cycle (the
 * peripheral controller may be power-cycled), so without this the
 * panel would come back at whatever level the firmware happens to
 * default to — usually full brightness, which is a jarring flash.
 *
 * Since the resume handler runs on the MFD parent's resume callback
 * (process context, may sleep), we can call straight into the regular
 * beada_bl_send() path: it acquires the autopm reference internally
 * via beada_set_brightness() on the MFD side and so works even if some
 * other code path is racing the resume.
 *
 * Why a separate file
 * ───────────────────
 * Symmetry with beada-drm-pm.c and the MFD parent's beada-pm.c.  The
 * resume hook is one of those things that's easy to overlook if it's
 * tucked into probe/remove boilerplate — giving it its own file makes
 * the suspend / resume picture clearer when reading the driver.
 */

#include <linux/mutex.h>

#include "beada-backlight.h"
/*
void beada_bl_resume_handler(void *priv)
{
	struct beada_backlight *mb = priv;

	mutex_lock(&mb->lock);
	beada_bl_send(mb, mb->stored_value);
	mutex_unlock(&mb->lock);
}
*/
void beada_bl_resume_handler(void *priv)
{
	struct beada_backlight *mb = priv;
	int ret;

	mutex_lock(&mb->lock);
	ret = beada_bl_send(mb, mb->stored_value);
	mutex_unlock(&mb->lock);
}
