// SPDX-License-Identifier: GPL-2.0-only
/*
 * beada-backlight-bl.c – Linux backlight class ops + hardware send
 *
 * The "hot path" of the backlight child: everything that runs on every
 * userspace write to /sys/class/backlight/<name>/brightness.
 *
 * Three layers, top-down:
 *
 *   beada_bl_update_status()  - backlight class callback
 *     │     Clamps the requested value into 0..@max_value, stores it,
 *     │     and hands it down.
 *     ▼
 *   beada_bl_send()           - applies gamma + forwards to MFD
 *     │     Public to the rest of the backlight child (sysfs gamma write
 *     │     re-runs the curve through this; PM resume also calls in).
 *     ▼
 *   beada_pow_lut()           - integer power approximation
 *     │     Fixed-point base^(g/100) on a 0..255 scale.  Internal to
 *     │     this file; only beada_bl_send() uses it.
 *     ▼
 *   beada_set_brightness()    - MFD parent's public Status-Link write
 *
 * Locking
 * ───────
 * Callers of beada_bl_send() must hold @mb->lock.  The function does no
 * locking of its own — the lock guards @stored_value against re-entry
 * from concurrent userspace writes AND keeps the gamma curve coherent
 * with the value being sent (although gamma_x100 itself uses READ_ONCE,
 * so a sysfs gamma write that races a brightness write will land on the
 * next write, not the current one).
 */

#include <linux/backlight.h>
#include <linux/minmax.h>
#include <linux/mutex.h>
#include <linux/types.h>

#include "../beada.h"
#include "beada-backlight.h"

/* ------------------------------------------------------------------ */
/* Software gamma curve                                                 */
/* ------------------------------------------------------------------ */

/**
 * beada_pow_lut() - Integer base^(g/100) on a 0..255 fixed-point scale
 * @raw:    Input value, 0..255
 * @g_x100: Exponent × 100, e.g. 220 means gamma 2.20
 *
 * Returns the curved value in the same 0..255 range.
 *
 * Implementation: repeated multiply-then-rescale for the integer part
 * of the exponent, then linear interpolation between exponent=whole and
 * exponent=whole+1 for the fractional part.  For typical gamma values
 * (1.0–4.0) the loop runs at most 4 iterations of the integer exponent
 * plus one fractional step, so latency is well under a microsecond — no
 * lookup table needed.
 *
 * Signed arithmetic in the fractional step is required because the
 * curve is monotonically decreasing in exponent for base < 255 (so
 * next < result and the difference must not underflow as u32).
 */
static u8 beada_pow_lut(u8 raw, u32 g_x100)
{
	u32 base = (u32)raw;
	u32 result;
	u32 frac;
	int whole;

	if (g_x100 == 100 || raw == 0 || raw == 255)
		return raw;

	whole = g_x100 / 100;
	frac  = g_x100 % 100;

	result = 255;
	while (whole-- > 0)
		result = result * base / 255;

	if (frac) {
		s32 next = (s32)(result * base / 255);
		s32 step = ((next - (s32)result) * (s32)frac) / 100;

		result = (u32)((s32)result + step);
	}

	return (u8)min_t(u32, result, 255);
}

/* ------------------------------------------------------------------ */
/* Hardware send                                                        */
/* ------------------------------------------------------------------ */

int beada_bl_send(struct beada_backlight *mb, u8 raw)
{
	u32 hw;
	u32 g = READ_ONCE(mb->gamma_x100);

	if (g == BEADA_BL_GAMMA_DEFAULT) {
		hw = raw;
	} else {
		/*
		 * Apply gamma in the device's normalised 0..@max_value
		 * space.  pow_lut works on the 0..255 range, so we scale
		 * up, curve, then scale back:
		 *
		 *   normalised = raw / max_value           (in [0,1])
		 *   scaled_255 = normalised × 255
		 *   curved_255 = pow_lut(scaled_255, gamma)
		 *   hw         = curved_255 × max_value / 255
		 *
		 * The 0..255 round-trip is a minor precision trade-off
		 * (sub-LSB on the device side) in exchange for keeping
		 * pow_lut's internal arithmetic happy with 8-bit values.
		 */
		u32 scaled = (u32)raw * 255 / mb->max_value;
		u8  curved = beada_pow_lut(min_t(u32, scaled, 255), g);

		hw = (u32)curved * mb->max_value / 255;
	}

	return beada_set_brightness(mb->beada, (u8)hw);
}

/* ------------------------------------------------------------------ */
/* Backlight class callback                                             */
/* ------------------------------------------------------------------ */

static int beada_bl_update_status(struct backlight_device *bl)
{
	struct beada_backlight *mb = bl_get_data(bl);
	int requested = backlight_get_brightness(bl);
	u8 value = (u8)clamp(requested, 0, (int)mb->max_value);
	int ret;

	mutex_lock(&mb->lock);
	mb->stored_value = value;
	ret = beada_bl_send(mb, value);
	mutex_unlock(&mb->lock);

	if (ret && ret != -ENODEV)
		dev_warn_ratelimited(&bl->dev,
				     "SL_SET_BL %u failed: %d\n", value, ret);

	/*
	 * Don't propagate transient USB errors to the backlight class.
	 * Userspace has no useful action on -ETIMEDOUT and leaving the
	 * class in an error state would prevent further brightness
	 * writes.  @stored_value has been updated in either case, so
	 * the next write will simply try again.  -ENODEV (parent gone)
	 * is the only error we keep, so userspace can detect a removed
	 * device.
	 */
	return (ret == -ENODEV) ? -ENODEV : 0;
}

/* ------------------------------------------------------------------ */
/* Ops table (exported via the header)                                  */
/* ------------------------------------------------------------------ */

const struct backlight_ops beada_bl_ops = {
	.update_status = beada_bl_update_status,
};
