/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * beada.h – BeadaPanel MFD public interface
 *
 * This is the ONLY header child drivers (beada-drm, beada-backlight)
 * should include from the MFD parent.  Everything else is an internal
 * implementation detail.
 *
 * Child drivers obtain the parent handle with:
 *
 *   #include "beada.h"
 *   struct beada_mfd_dev *beada = dev_get_drvdata(pdev->dev.parent);
 */

#ifndef __BEADA_H
#define __BEADA_H

#include <linux/list.h>
#include <linux/types.h>

/* ------------------------------------------------------------------ */
/* MFD cell names                                                       */
/* ------------------------------------------------------------------ */

/** Child DRM platform device – handled by beada_drm.ko */
#define BEADA_CELL_DRM		"beada-drm"

/** Child backlight platform device – handled by beada_backlight.ko */
#define BEADA_CELL_BACKLIGHT	"beada-backlight"

/* ------------------------------------------------------------------ */
/* Panel information                                                    */
/* ------------------------------------------------------------------ */

/**
 * struct beada_panel_info - Immutable display properties
 *
 * Populated at probe by querying the device via Status-Link GET_INFO
 * and looking up the result in the model table.  Once published the
 * contents are read-only for the lifetime of the device.
 *
 * @width:              Horizontal resolution in pixels
 * @height:             Vertical resolution in pixels
 * @width_mm:           Active display area width in millimetres
 * @height_mm:          Active display area height in millimetres
 * @max_brightness:     Maximum value accepted by beada_set_brightness().
 *                      Zero means the device does not support brightness
 *                      control and the backlight child will skip its
 *                      registration.
 * @current_brightness: Backlight level at probe time, possibly overridden
 *                      by the MFD's initial_brightness module parameter.
 *                      Used by the backlight child to initialise its
 *                      initial value without a redundant GET_INFO read.
 * @model:              NUL-terminated model name, e.g. "5C", "6S", "2W".
 */
struct beada_panel_info {
	u16	width;
	u16	height;
	u16	width_mm;
	u16	height_mm;
	u8	max_brightness;
	u8	current_brightness;
	char	model[8];
};

/* ------------------------------------------------------------------ */
/* Resume hooks                                                         */
/* ------------------------------------------------------------------ */

/**
 * struct beada_resume_handler - Per-child resume hook
 *
 * @handler: Called from the MFD parent's USB resume path after the bus
 *           is back up, Panel-Link has been re-armed with PL_RESET, and
 *           the device clock has been re-synced.  Children use this to
 *           re-push state that the device lost across suspend (DRM:
 *           shadow buffer; backlight: stored value).
 * @priv:    Caller-supplied context passed verbatim to @handler.
 * @list:    Internal — linkage on beada->resume_handlers.  Children must
 *           not touch this field; just leave it zero-initialised before
 *           passing the struct to beada_register_resume_handler().
 *
 * The handler runs in MFD-parent resume context (process context, may
 * sleep).  It is safe to call the public API (beada_send_frame,
 * beada_set_brightness, …) from inside a handler: those functions
 * acquire only their own locks and an autopm reference, both of which
 * are reentrant in this context.
 *
 * Registration is one-shot per child: register during probe(), unregister
 * during remove().  Re-registration is not supported.
 */
struct beada_resume_handler {
	void (*handler)(void *priv);
	void *priv;

	struct list_head list;
};

/* ------------------------------------------------------------------ */
/* Opaque parent handle                                                 */
/* ------------------------------------------------------------------ */

/**
 * struct beada_mfd_dev - Opaque MFD parent handle.
 *
 * The full definition lives in beada-priv.h and is private to the parent
 * driver.  Child drivers must not dereference this pointer themselves.
 */
struct beada_mfd_dev;

/* ------------------------------------------------------------------ */
/* API: panel information                                               */
/* ------------------------------------------------------------------ */

/**
 * beada_get_panel_info() - Return display geometry and capabilities
 * @beada: Handle from dev_get_drvdata(dev->parent) in child probe
 *
 * The returned pointer is valid for the lifetime of the device and its
 * contents are read-only.  Never returns NULL.
 */
const struct beada_panel_info *beada_get_panel_info(struct beada_mfd_dev *beada);

/**
 * beada_uses_bgr_format() - Whether the Panel-Link stream uses BGR565 pixels
 * @beada: Handle from dev_get_drvdata(dev->parent)
 *
 * Currently always returns false: the driver packs pixels as RGB565
 * (matching drm_fb_xrgb8888_to_rgb565 and the JT365 reference) and labels
 * the Panel-Link caps as "format=BGR16"; tested firmware (≥ 5.00) accepts
 * that combination and reorders bytes internally.
 *
 * Reserved as an API hook for future devices or firmware that may
 * require a real BGR565 byte order on the wire.  The DRM child should
 * query this once at pipe-enable time and pick its drm_fb_*_to_*565
 * helper accordingly so a future true return value transparently
 * switches the pack path with no other code change.
 */
bool beada_uses_bgr_format(struct beada_mfd_dev *beada);

/* ------------------------------------------------------------------ */
/* API: async frame submission                                          */
/* ------------------------------------------------------------------ */

/**
 * beada_send_frame() - Submit a frame for asynchronous transfer
 * @beada:   Handle from dev_get_drvdata(dev->parent)
 * @buf:     RGB16 (RGB565) pixel data, row-major, top-left origin
 * @len:     Buffer size in bytes; must equal @frame_w × @frame_h × 2
 * @frame_w: Pixel width of the data
 * @frame_h: Pixel height of the data
 *
 * Enqueues a Panel-Link frame transfer onto the background workqueue
 * and returns immediately.  The actual USB transfer (~8–20 ms on USB
 * 2.0 HS) runs asynchronously; per-frame errors are logged inside the
 * worker.
 *
 * On the first frame of a session — after probe, and after any USB
 * resume — the worker prepends a Panel-Link TYPE_START tag with the
 * GStreamer caps string.  Subsequent frames send only pixel data
 * because the firmware refuses any further START tag until the next
 * Status-Link PL_RESET.  @frame_w and @frame_h must therefore match
 * the dimensions of the first frame in the session; a mismatch causes
 * the worker to log an error and drop the frame.
 *
 * Single-buffer ownership: @buf is referenced by the worker until the
 * next call to beada_flush_frame_transfer(), at which point it is safe
 * to reuse or free.  Callers MUST flush before:
 *   - modifying or freeing @buf
 *   - calling beada_send_frame() again with a different source buffer
 *
 * The caller is responsible for keeping an autopm reference (via
 * beada_pipe_active()) over the lifetime of an enabled pipe so the bus
 * is guaranteed to be up when this function reaches the worker.
 *
 * Returns 0 on successful enqueue, -EINVAL if @len doesn't match
 * @frame_w × @frame_h × 2 or either dimension is zero.  Device errors
 * (-ENODEV on disconnect, -EHOSTDOWN if a transfer slips into a
 * system-suspend window, or USB error codes) are reported
 * asynchronously via the worker and not surfaced by this call.
 */
int beada_send_frame(struct beada_mfd_dev *beada,
		     const void *buf, size_t len,
		     u16 frame_w, u16 frame_h);

/**
 * beada_flush_frame_transfer() - Wait for the latest enqueued frame
 *                                to finish transferring
 *
 * Blocks until the most recent beada_send_frame() submission has been
 * fully transmitted (or has failed).  Returns no error code; check
 * dmesg or the per-device log for transfer status.
 *
 * Safe to call when no transfer is pending — returns immediately.
 */
void beada_flush_frame_transfer(struct beada_mfd_dev *beada);

/* ------------------------------------------------------------------ */
/* API: backlight                                                       */
/* ------------------------------------------------------------------ */

/**
 * beada_set_brightness() - Set the backlight level
 * @beada: Handle from dev_get_drvdata(dev->parent)
 * @level: Desired level, 0..beada_panel_info.max_brightness
 *
 * If the device is currently autosuspended, this function returns 0
 * without touching USB and without waking the device.  The backlight
 * child driver is expected to have already cached the requested value
 * in its own state; the registered resume handler will push that value
 * to hardware when the device next wakes for any other reason.  This
 * matches what the user sees on screen anyway — an autosuspended
 * BeadaPanel shows its internal idle clock with the firmware managing
 * the backlight independently of the SL_SET_BL register.
 *
 * When the device IS active (DRM pipe open, or a resume callback is in
 * progress), the value is sent synchronously over Status-Link.
 *
 * Returns 0 on success, -EOPNOTSUPP if the device has no Status-Link
 * control channel, -ENODEV if disconnected, -EHOSTDOWN if a system
 * suspend is in progress, or a USB error code.
 */
int beada_set_brightness(struct beada_mfd_dev *beada, u8 level);

/* ------------------------------------------------------------------ */
/* API: resume hook registration                                        */
/* ------------------------------------------------------------------ */

/**
 * beada_register_resume_handler() - Subscribe to the resume notification
 * @beada: Handle from dev_get_drvdata(dev->parent)
 * @h:     Caller-allocated handler struct (typically embedded in the
 *         child's per-device state).  Fields @handler and @priv must be
 *         filled in by the caller; @list is internal.
 *
 * Call from the child driver's probe().  The handler will be invoked on
 * every USB resume until beada_unregister_resume_handler() is called.
 *
 * Returns 0 on success.  Currently always succeeds, but child drivers
 * should still check the return value.
 */
int  beada_register_resume_handler(struct beada_mfd_dev *beada,
				   struct beada_resume_handler *h);

/**
 * beada_unregister_resume_handler() - Remove a previously-registered hook
 * @beada: Handle from dev_get_drvdata(dev->parent)
 * @h:     The same struct passed to beada_register_resume_handler()
 *
 * Call from the child driver's remove().  Idempotent if @h was never
 * registered or has already been unregistered (it just unlinks an empty
 * list_head).
 */
void beada_unregister_resume_handler(struct beada_mfd_dev *beada,
				     struct beada_resume_handler *h);

/* ------------------------------------------------------------------ */
/* API: pipe lifecycle (autopm)                                         */
/* ------------------------------------------------------------------ */

/**
 * beada_pipe_active() / beada_pipe_idle() - Pipe lifecycle hooks
 *
 * Called by the DRM child when its pipe is enabled and disabled.
 * Between an _active() and the matching _idle() call the USB-level
 * autosuspend timer is held off; the device stays powered even if no
 * frames are sent (e.g. when the compositor has dimmed the screen but
 * is still running, as Weston does).
 *
 * Pairing: exactly one _active() per _idle().  Calling _active() twice,
 * or _idle() when not active, is a programming error and triggers a
 * usb_autopm refcount imbalance warning.
 *
 * beada_pipe_active() may sleep: if the device was autosuspended, the
 * USB stack resumes it synchronously before this function returns.
 * Returns 0 on success, negative errno on USB resume failure (in which
 * case the pipe should not consider itself active — _idle() must NOT
 * be called).
 */
int  beada_pipe_active(struct beada_mfd_dev *beada);
void beada_pipe_idle(struct beada_mfd_dev *beada);

#endif /* __BEADA_H */
