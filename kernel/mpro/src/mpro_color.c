// SPDX-License-Identifier: GPL-2.0
/*
 * mpro_color.c — shared color helpers used by child drivers.
 *
 * Currently provides the gamma power-curve LUT helper. Both the DRM
 * driver (per-channel software gamma) and the backlight driver
 * (perceived-brightness curve) compute the same y = (x/255)^gamma
 * mapping; this file is the single implementation.
 */

#include <linux/module.h>
#include <linux/math.h>

#include "mpro.h"

/*
 * y = round(255 * (x/255)^(g_x100/100))
 *
 * Split gamma into an integer part n and a fractional part f/100, then
 * interpolate between the two adjacent integer-exponent curves:
 *
 *   y_n     = 255 * (x/255)^n     = x^n / 255^(n-1)   (n >= 1)
 *   y_n     = 255                                      (n == 0)
 *   y_{n+1} = 255 * (x/255)^(n+1) = x^(n+1) / 255^n
 *   y       = (y_n * (100-f) + y_{n+1} * f) / 100
 *
 * Exact for integer gammas, within ±1 between them (gamma 0.5..4.0).
 * int_pow(255, 5) ≈ 1.08e12 fits comfortably in u64.
 */
u8 mpro_pow_lut(u32 x, u32 g_x100)
{
	u32 n = g_x100 / 100;
	u32 f = g_x100 % 100;
	u64 y_n, y_np1, y;

	if (x == 0)
		return 0;
	if (x >= 255)
		return 255;
	if (g_x100 == 100)
		return (u8)x;

	if (n == 0)
		y_n = 255;
	else
		y_n = div64_u64(int_pow(x, n), int_pow(255, n - 1));

	y_np1 = div64_u64(int_pow(x, n + 1), int_pow(255, n));

	y = (y_n * (100 - f) + y_np1 * f) / 100;
	if (y > 255)
		y = 255;
	return (u8)y;
}
EXPORT_SYMBOL_GPL(mpro_pow_lut);
