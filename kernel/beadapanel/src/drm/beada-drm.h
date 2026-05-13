/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * beada-drm.h – BeadaPanel DRM child driver: private definitions
 *
 * Only included by the DRM child's own .c files.
 * Userspace and the MFD parent see only beada.h.
 *
 * Architecture
 * ────────────
 * beada-drm-drv.c    platform_driver, drm_driver, module params, probe/remove
 * beada-drm-kms.c    mode_config, beada_kms_init(), apply_rotation()
 * beada-drm-conn.c   drm_connector_funcs + helper_funcs, get_modes, detect
 * beada-drm-edid.c   EDID 1.3 builder, rebuilt on every rotation change
 * beada-drm-plane.c  pipe_enable/disable/update, shadow buffer, blank frame
 * beada-drm-color.c  Gamma LUT (DRM-native gamma_lut path, no sysfs duplicates)
 * beada-drm-vblank.c hrtimer vblank, arm_pageflip / finish_pageflip helpers
 * beada-drm-sysfs.c  Per-device sysfs attributes (rotation, noblank)
 * beada-drm-pm.c     Resume handler registered with the MFD parent
 *
 * Power management
 * ────────────────
 * No platform_driver .pm callbacks.  The MFD parent's USB resume callback
 * dispatches our registered resume handler (beada-drm-pm.c) after the bus
 * is back up and Panel-Link has been re-armed.  Autosuspend is gated by
 * the pipe lifecycle: pipe_enable holds beada_pipe_active(), pipe_disable
 * releases it.  So an enabled pipe keeps the device powered (even on a
 * dim idle screen), and a disabled pipe lets autosuspend kick in.
 *
 * Rotation model
 * ──────────────
 * Rotation is controlled exclusively via sysfs ("rotation" attribute,
 * values 0..7 indexing the same orientation table as the mpro driver).
 * It is NOT exposed as a DRM plane property — the compositor sees only
 * a single landscape-or-portrait mode whose dimensions reflect the
 * current rotation setting.  The shadow buffer is always in NATIVE panel
 * coordinates (panel->width × panel->height × 2); rotation happens
 * during the blit.
 *
 * The Panel-Link firmware locks the frame format on its first START tag
 * (see beada.h / beada_send_frame docstring), so every frame sent to the
 * MFD parent is always the native panel size — we never tell the device
 * about rotation.
 *
 * Shadow / blank buffers
 * ──────────────────────
 *   shadow_buf  – Last fully converted frame in RGB565, native coordinates.
 *                 Re-sent to hardware by the resume handler after a USB
 *                 suspend cycle so the image is back on screen before
 *                 userspace unfreezes.  Protected by shadow_lock.
 *   blank_buf   – Same size as shadow_buf, zeroed once at probe.  Used by
 *                 pipe_disable to clear the display unless noblank=1, and
 *                 by the resume handler if shadow_valid is false.
 *
 * Vblank
 * ──────
 * BeadaPanel has no hardware vblank.  An hrtimer fires at 60 fps and
 * calls drm_crtc_handle_vblank().  Page-flip events are NOT sent from
 * pipe_update; they are armed for the next vblank tick (see
 * beada-drm-vblank.c).  This is what keeps Weston's "monotonic vblank
 * timestamp" assertion happy.
 */

#ifndef __BEADA_DRM_H
#define __BEADA_DRM_H

#include <linux/hrtimer.h>
#include <linux/mutex.h>
#include <linux/platform_device.h>
#include <linux/spinlock.h>
#include <linux/types.h>

#include <drm/drm_connector.h>
#include <drm/drm_device.h>
#include <drm/drm_simple_kms_helper.h>

#include "../beada.h"

/* ------------------------------------------------------------------ */
/* DRM driver constants                                                 */
/* ------------------------------------------------------------------ */

#define BEADA_DRM_NAME		"beada"
#define BEADA_DRM_DESC		"BeadaPanel USB auxiliary display"
#define BEADA_DRM_MAJOR		1
#define BEADA_DRM_MINOR		0

/* ------------------------------------------------------------------ */
/* CVT-derived display timing (shared between mode and EDID)            */
/* ------------------------------------------------------------------ */
/*
 * These timings are arbitrary — there is no real scanout — but they have
 * to be self-consistent between the DRM mode and the EDID detailed timing
 * descriptor so userspace tools (xrandr, edid-decode) don't flag a
 * mismatch.
 *
 * Numbers chosen to match a generic 60 Hz CVT-style timing for the most
 * common BeadaPanel resolutions (800x480, 480x320, 320x480, 1280x480).
 * Pixel clock is computed as htotal * vtotal * 60 Hz at apply time.
 */
#define BEADA_TIMING_HFP	40	/* horizontal front porch  */
#define BEADA_TIMING_HSYNC	48	/* horizontal sync pulse   */
#define BEADA_TIMING_HBP	40	/* horizontal back porch   */
#define BEADA_TIMING_VFP	13	/* vertical front porch    */
#define BEADA_TIMING_VSYNC	 3	/* vertical sync pulse     */
#define BEADA_TIMING_VBP	29	/* vertical back porch     */
#define BEADA_TIMING_REFRESH	60	/* Hz                      */

/* ------------------------------------------------------------------ */
/* Per-device state                                                     */
/* ------------------------------------------------------------------ */

/**
 * struct beada_drm_dev - Full state of the BeadaPanel DRM child driver
 *
 * @drm:    DRM device — MUST be the first member so container_of works
 *          with to_beada_drm() inline below.
 * @pipe:   drm_simple_display_pipe embedding plane + CRTC + encoder.
 * @conn:   Single DRM connector, always connected.
 *
 * @beada:  MFD parent handle; all USB I/O goes through it.
 * @panel:  Pointer to the parent's beada_panel_info (immutable for our
 *          lifetime).
 *
 * @width:  Logical width  in pixels for the CURRENT rotation
 *          (= panel->width  when rotation ∈ {0°, 180°},
 *             panel->height when rotation ∈ {90°, 270°}).
 * @height: Logical height, analogous.
 *
 * @rotation: Active rotation bits, combinations of DRM_MODE_ROTATE_*
 *            and DRM_MODE_REFLECT_X (REFLECT_Y is normalised away into
 *            ROTATE_180 + REFLECT_X by the sysfs setter).
 *
 * @noblank: When set, pipe_disable does NOT send a blank frame; the
 *           last image stays on the display.  Initial value is copied
 *           from the beada_noblank module parameter at probe; can be
 *           changed at runtime via the per-device sysfs attribute.
 *
 * @shadow_buf:   Persistent RGB565 buffer, NATIVE coordinates
 *                (size = panel->width × panel->height × 2).
 * @shadow_size:  Byte count of @shadow_buf.
 * @shadow_valid: True after the first complete blit.
 * @shadow_lock:  Guards @shadow_valid and the contents of @shadow_buf.
 *
 * @blank_buf:    Same size as @shadow_buf, zeroed once at probe.  Used by
 *                pipe_disable to clear the display unless noblank=1, and
 *                by the resume handler when there is no valid shadow.
 *                No lock — read-only after probe.
 *
 * @lut:          256-entry per-channel gamma LUT.  When @gamma_valid is
 *                false the blit code uses a linear mapping; when true it
 *                looks up each 8-bit channel value through @lut.
 * @gamma_valid:  False after init and whenever userspace clears the
 *                crtc gamma_lut property; true after a non-NULL gamma_lut
 *                is applied via apply_color_mgmt().
 * @lut_lock:     Guards @lut and @gamma_valid.
 *
 * @vblank_timer:   hrtimer firing at 60 fps for drm_crtc_handle_vblank().
 * @vblank_enabled: Set while the timer is running; guarded by @vblank_lock.
 * @vblank_lock:    Protects @vblank_enabled across timer callback and
 *                  enable/disable from DRM core.
 * @frame_time:     Cached ktime period (NSEC_PER_SEC / 60).
 *
 * @resume:  Resume handler struct registered with the MFD parent at
 *           probe and unregistered at remove.  The handler re-sends
 *           the shadow (or a blank frame) so the device is back on
 *           screen by the time userspace unfreezes from suspend.
 */
struct beada_drm_dev {
	struct drm_device              drm;
	struct drm_simple_display_pipe pipe;
	struct drm_connector           conn;

	struct beada_mfd_dev          *beada;
	const struct beada_panel_info *panel;

	u32 width;
	u32 height;
	u16 rotation;
	bool noblank;

	void          *shadow_buf;
	size_t         shadow_size;
	bool           shadow_valid;
	struct mutex   shadow_lock;

	void          *blank_buf;

	u8             lut[3][256];
	bool           gamma_valid;
	struct mutex   lut_lock;

	struct hrtimer vblank_timer;
	bool           vblank_enabled;
	spinlock_t     vblank_lock;
	ktime_t        frame_time;

	struct beada_resume_handler resume;
};

/* ------------------------------------------------------------------ */
/* container_of helpers                                                 */
/* ------------------------------------------------------------------ */

static inline struct beada_drm_dev *to_beada_drm(struct drm_device *d)
{
	return container_of(d, struct beada_drm_dev, drm);
}

static inline struct beada_drm_dev *pipe_to_beada_drm(
		struct drm_simple_display_pipe *p)
{
	return container_of(p, struct beada_drm_dev, pipe);
}

static inline struct beada_drm_dev *conn_to_beada_drm(
		struct drm_connector *c)
{
	return container_of(c, struct beada_drm_dev, conn);
}

/* ------------------------------------------------------------------ */
/* Inline helpers                                                       */
/* ------------------------------------------------------------------ */

/**
 * beada_rotation_swaps_dims() - Does this rotation transpose W and H?
 *
 * True for 90° / 270°.  REFLECT_X / REFLECT_Y don't transpose.
 */
static inline bool beada_rotation_swaps_dims(u16 rot)
{
	u16 angle = rot & (DRM_MODE_ROTATE_0  | DRM_MODE_ROTATE_90 |
			   DRM_MODE_ROTATE_180 | DRM_MODE_ROTATE_270);

	return angle == DRM_MODE_ROTATE_90 || angle == DRM_MODE_ROTATE_270;
}

/* ------------------------------------------------------------------ */
/* Module parameters (defined in beada-drm-drv.c)                       */
/* ------------------------------------------------------------------ */

/** noblank - skip the all-zero frame sent on pipe_disable */
extern bool beada_noblank;

/* ------------------------------------------------------------------ */
/* Cross-file prototypes, grouped by source file                        */
/* ------------------------------------------------------------------ */

/* beada-drm-kms.c */

/**
 * beada_kms_init() - Set up mode_config, pipe, connector, gamma LUT
 *                    and rotation handling.
 */
int  beada_kms_init(struct beada_drm_dev *bdev);

/**
 * beada_apply_rotation() - Switch active orientation
 * @bdev:         Device
 * @new_rotation: New rotation bits (validated by caller)
 *
 * Updates @rotation, swaps @width/@height as needed, refreshes
 * mode_config limits, rebuilds the EDID, and fires a hotplug event so
 * compositors re-query the mode list.  Called from sysfs after a
 * successful write.
 */
void beada_apply_rotation(struct beada_drm_dev *bdev, u16 new_rotation);

/** Pipe funcs table — referenced from kms.c, defined alongside plane.c */
extern const struct drm_simple_display_pipe_funcs beada_pipe_funcs;

/** Connector tables — referenced from kms.c, defined in conn.c */
extern const struct drm_connector_funcs        beada_conn_funcs;
extern const struct drm_connector_helper_funcs beada_conn_helper_funcs;

/* beada-drm-edid.c */

/**
 * beada_edid_update() - Build a fresh EDID block for the current rotation
 *                       and attach it as the connector's EDID property.
 *
 * The block reflects @bdev->width / @bdev->height (logical dimensions),
 * @bdev->panel->width_mm / @bdev->panel->height_mm (physical, swapped
 * with the rotation), and the panel's model name.
 *
 * Returns 0 on success, a negative errno from the connector property
 * update on failure (non-fatal).
 */
int beada_edid_update(struct beada_drm_dev *bdev);

/* beada-drm-plane.c */

void beada_pipe_enable(struct drm_simple_display_pipe *pipe,
		       struct drm_crtc_state *crtc_state,
		       struct drm_plane_state *plane_state);
void beada_pipe_disable(struct drm_simple_display_pipe *pipe);
void beada_pipe_update(struct drm_simple_display_pipe *pipe,
		       struct drm_plane_state *old_state);

/* beada-drm-color.c */

/**
 * beada_apply_color_mgmt() - Mirror crtc_state->gamma_lut into bdev->lut
 *
 * Reads crtc_state->gamma_lut (16-bit per channel) and writes the 8-bit
 * lookup tables used by the blit.  Sets @gamma_valid accordingly.
 * No-op if @crtc_state->color_mgmt_changed is false.
 */
void beada_apply_color_mgmt(struct beada_drm_dev *bdev,
			    struct drm_crtc_state *crtc_state);

/* beada-drm-vblank.c */

void beada_vblank_init(struct beada_drm_dev *bdev);

int  beada_vblank_enable(struct drm_simple_display_pipe *pipe);
void beada_vblank_disable(struct drm_simple_display_pipe *pipe);

/**
 * beada_arm_pageflip() - Arm the pending flip event for the next vblank tick
 *
 * Acquires a vblank reference and hands it to drm_crtc_arm_vblank_event()
 * so the event is delivered with a monotonically increasing timestamp.
 * Falls back to drm_crtc_send_vblank_event() if no vblank is available.
 *
 * Eliminates the Weston assertion
 *   "timespec_sub_to_nsec(stamp, &output->frame_time) >= 0"
 * that occurred when events were sent with an instantaneous timestamp.
 */
void beada_arm_pageflip(struct drm_simple_display_pipe *pipe);

/**
 * beada_finish_pageflip() - Consume any pending flip event immediately
 *
 * Sends the event with the current timestamp (no arming).  Used in
 * pipe_enable (no prior frame_time exists yet) and pipe_disable (the
 * compositor is tearing down; no further vblank ticks will come).
 */
void beada_finish_pageflip(struct drm_simple_display_pipe *pipe);

/* beada-drm-pm.c */

/**
 * beada_drm_resume_handler() - Re-push the shadow buffer after USB resume
 *
 * Registered with the MFD parent at probe via
 * beada_register_resume_handler(); the parent invokes it from
 * beada_usb_resume() after Panel-Link has been re-armed.  Re-sends
 * shadow_buf if valid, blank_buf if not and noblank is off, otherwise
 * does nothing.
 */
void beada_drm_resume_handler(void *priv);

/* beada-drm-sysfs.c */

int  beada_sysfs_init(struct beada_drm_dev *bdev);
void beada_sysfs_fini(struct beada_drm_dev *bdev);

#endif /* __BEADA_DRM_H */
