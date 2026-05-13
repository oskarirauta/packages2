/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * beada-priv.h – BeadaPanel MFD parent: private definitions
 *
 * Included by the parent driver's own .c files only.
 * Child drivers (beada-drm, beada-backlight) include only beada.h.
 *
 * Contents:
 *   1. Protocol constants  (Panel-Link and Status-Link)
 *   2. Wire-format structs (packed; never exposed to children)
 *   3. Model table entry   (internal to beada-model.c)
 *   4. Full beada_mfd_dev  (opaque pointer in the public beada.h)
 *   5. Cross-file function declarations, grouped by source file
 */

#ifndef __BEADA_PRIV_H
#define __BEADA_PRIV_H

#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/types.h>
#include <linux/usb.h>

#include "beada.h"

/* ==================================================================
 * Panel-Link v1.0 constants
 *
 * Type codes from NXElec panelLinkProtocol.c reference implementation.
 * The official spec Rev. 1.0 (2023) also defines types 5/6 ("Start of
 * Media Stream" / "End of Media Stream"), but BeadaPanel firmware uses
 * types 1/2 in practice — the spec's types 1/2 are labelled "Legacy
 * Command" but they are what the actual driver and all known devices use.
 *
 * Using spec types 5/6 caused STALL (-EPIPE/-EAGAIN) on tested hardware;
 * reverting to 1/2 restored operation.
 * ================================================================== */

#define PL_TYPE_START		1	/* Stream start (JT365 working code)  */
#define PL_TYPE_END		2	/* Stream end                         */
#define PL_TYPE_RESET		3	/* Stream reset                       */
#define PL_TYPE_CLEAR		4	/* Clear screen                       */

#define PL_VERSION		1	/* Tag version field per JT365 reference */

/**
 * Maximum pixel payload per bulk-OUT transfer.
 * Multiple of 512 (USB 2.0 HS max-packet) to avoid short-packet ambiguity.
 * Matches the NXElec daemon: PL_BUFF_SIZE = 512 * 25.
 */
#define PANELLINK_MAX_FRAME_BYTES	(1280 * 480 * 2)	/* 1.2 MB; biggest panel  */

#define PL_TAG_TIMEOUT_MS	500
#define PL_DATA_TIMEOUT_MS	1000

/* ==================================================================
 * Status-Link v1.3 constants
 *
 * Type codes from statusLinkProtocol.c (NXElec kernel driver).
 * ================================================================== */

#define SL_TYPE_GET_INFO	1	/* Get panel info             */
#define SL_TYPE_PL_RESET	2	/* Reset Panel-Link subsystem */
#define SL_TYPE_SET_BL		3	/* Set backlight              */
#define SL_TYPE_PUSH_STORAGE	4	/* Push to internal storage   */
#define SL_TYPE_GET_TIME	5	/* Get device clock           */
#define SL_TYPE_SET_TIME	6	/* Set device clock           */

/*
 * Status-Link protocol version.
 *
 * IMPORTANT: The official Status-Link USB Panel Control Protocol Specification
 * (Rev 1.3, NXElec) mandates version = 2 in every header.  Using version 1
 * causes the device firmware to reject the packet at the software USB stack
 * level without ACK-ing the bulk-OUT transfer, which manifests as -ETIMEDOUT
 * on the host side.
 */
#define SL_VERSION		2

#define SL_CMD_TIMEOUT_MS	200
#define SL_DATA_TIMEOUT_MS	1000

/* ==================================================================
 * Panel-Link wire format
 * ================================================================== */

/**
 * struct panellink_tag - 270-byte Panel-Link frame delimiter
 *
 * @protocol_name: "PANEL-LINK" (10 bytes, no NUL)
 * @version:       PL_VERSION (1)
 * @type:          PL_TYPE_START / PL_TYPE_END / PL_TYPE_RESET
 * @fmtstr:        GStreamer caps string for TYPE_START (256-byte field).
 *                 HEIGHT before WIDTH is required by device firmware.
 *                 Example: "video/x-raw, format=BGR16, height=480,
 *                            width=800, framerate=0/1"
 * @checksum16:    RFC 1071 checksum of the preceding 268 bytes.
 */
struct panellink_tag {
	char		protocol_name[10];
	u8		version;
	u8		type;
	char		fmtstr[256];
	__sum16		checksum16;
} __packed;

static_assert(sizeof(struct panellink_tag) == 270,
	      "panellink_tag wire format changed");
static_assert(offsetof(struct panellink_tag, checksum16) == 268,
	      "panellink_tag checksum offset wrong");

/* ==================================================================
 * Status-Link wire format
 * ================================================================== */

/**
 * struct statuslink_tag - 20-byte Status-Link packet header
 *
 * @protocol_name:   "STATUS-LINK" (11 bytes, no NUL)
 * @version:         SL_VERSION (2)
 * @type:            SL_TYPE_* command or response code
 * @reserved1:       Zero
 * @sequence_number: Echoed back by device; we always send 0
 * @length:          Total packet size (header + payload) in bytes
 * @checksum16:      RFC 1071 checksum of the 18 bytes before this field.
 *                   The payload is NEVER included in the checksum (NXElec
 *                   statusLinkProtocol.c behaviour).
 */
struct statuslink_tag {
	char		protocol_name[11];
	u8		version;
	u8		type;
	u8		reserved1;
	__le16		sequence_number;
	__le16		length;
	__sum16		checksum16;
} __packed;

static_assert(sizeof(struct statuslink_tag) == 20,
	      "statuslink_tag wire format changed");

/**
 * struct statuslink_info - GET_INFO response payload (follows statuslink_tag)
 *
 * @firmware_version:    Device firmware revision
 * @panellink_version:   Panel-Link protocol version on device
 * @statuslink_version:  Status-Link protocol version on device
 * @hardware_platform:   SoC/platform identifier
 * @os_version:          Model ID – used as key for beada_lookup_model()
 * @sn:                  64-byte serial number
 * @screen_resolution_x: Advertised horizontal pixels (informational)
 * @screen_resolution_y: Advertised vertical pixels (informational)
 * @storage_size:        Internal storage capacity in bytes
 * @max_brightness:      Maximum SET_BL level; 0 = no backlight control
 * @current_brightness:  Backlight level at time of query
 */
struct statuslink_info {
	__le16	firmware_version;
	u8	panellink_version;
	u8	statuslink_version;
	u8	hardware_platform;
	u8	os_version;
	u8	sn[64];
	__le16	screen_resolution_x;
	__le16	screen_resolution_y;
	__le32	storage_size;
	u8	max_brightness;
	u8	current_brightness;
} __packed;

static_assert(sizeof(struct statuslink_info) == 80,
	      "statuslink_info wire format changed");

/** Complete GET_INFO response */
struct statuslink_info_pack {
	struct statuslink_tag  header;
	struct statuslink_info value;
} __packed;

static_assert(sizeof(struct statuslink_info_pack) == 100);

/** SET_BL command */
struct statuslink_bl_pack {
	struct statuslink_tag  header;
	u8                     level;
} __packed;

static_assert(sizeof(struct statuslink_bl_pack) == 21);

/**
 * struct statuslink_systemtime - 16-byte date/time payload (SYSTEMTIME format)
 *
 * Used as the payload for both SET_TIME (request) and GET_TIME (response).
 * Matches the Windows SYSTEMTIME structure layout; all fields are LE u16.
 * Date and time are expressed in UTC.
 *
 * Defined in Status-Link spec Rev 1.3, tables 2-5 and 2-6.
 */
struct statuslink_systemtime {
	__le16	wYear;		  /* 1601..30827 */
	__le16	wMonth;		  /* 1..12 */
	__le16	wDayOfWeek;	  /* 0=Sunday..6=Saturday */
	__le16	wDay;		  /* 1..31 */
	__le16	wHour;		  /* 0..23 */
	__le16	wMinute;	  /* 0..59 */
	__le16	wSecond;	  /* 0..59 */
	__le16	wMilliseconds;	  /* 0..999 */
} __packed;

static_assert(sizeof(struct statuslink_systemtime) == 16,
	      "statuslink_systemtime must be 16 bytes");

/**
 * struct statuslink_time_pack - SET_TIME command / GET_TIME response packet
 *
 * Total: 20 B header + 16 B SYSTEMTIME payload = 36 B.
 *
 * Earlier versions of this driver used a 4-byte Unix timestamp.  The official
 * Status-Link specification (Rev 1.3, table 2-5 / 2-6) mandates the 16-byte
 * SYSTEMTIME breakdown format.
 */
struct statuslink_time_pack {
	struct statuslink_tag        header;
	struct statuslink_systemtime time;
} __packed;

static_assert(sizeof(struct statuslink_time_pack) == 36,
	      "statuslink_time_pack must be 36 bytes");

/* ==================================================================
 * Model table entry
 * ================================================================== */

/**
 * struct beada_model_entry - One row in beada_model_table[] (beada-model.c)
 *
 * @os_version: Matches statuslink_info.os_version
 * @width:      Pixels horizontal
 * @height:     Pixels vertical
 * @width_mm:   Active display width in mm
 * @height_mm:  Active display height in mm
 * @name:       NUL-terminated model string, e.g. "5C"
 */
struct beada_model_entry {
	u8	os_version;
	u16	width;
	u16	height;
	u16	width_mm;
	u16	height_mm;
	char	name[8];
};

/* ==================================================================
 * Private device state
 * ================================================================== */

/**
 * struct beada_mfd_dev - Complete per-device state of the MFD parent
 *
 * ── USB context ────────────────────────────────────────────────────
 * @udev:  USB device handle
 * @intf:  USB interface 0 (vendor-class, Panel-Link + Status-Link)
 * @dev:   &intf->dev – convenience for dev_*() logging
 *
 * ── Endpoints (discovered by beada_find_endpoints()) ───────────────
 * @ep_pl_out:      Bulk-OUT for Panel-Link pixel data   (0x01)
 * @ep_pl_in:       Bulk-IN  for Panel-Link              (0x81); reserved
 *                  for future use (display status / vsync hints)
 * @ep_sl_out:      Bulk-OUT for Status-Link commands    (0x02)
 * @ep_sl_in:       Bulk-IN  for Status-Link responses   (0x82)
 * @has_statuslink: True when both Status-Link endpoints are present
 *
 * ── Pixel format flag ──────────────────────────────────────────────
 * @pl_bgr_format: Currently always false; reserved for future devices
 *                 that may genuinely require BGR565 byte order on the
 *                 wire.
 *
 * ── Device information (from GET_INFO, set once at probe) ──────────
 * @panel:              Public display geometry and brightness range
 * @firmware_version:   BCD firmware version (e.g. 0x0717 = fw 7.17)
 * @panellink_version:  Panel-Link protocol version supported by firmware
 * @statuslink_version: Status-Link protocol version supported by firmware
 * @platform:           SoC identifier (1=i.MX6UL, 2=i.MX6ULL, 4=V3S, 5=T113)
 * @sn:                 NUL-terminated ASCII serial number (up to 64 chars)
 * @storage_size_kb:    On-board eMMC/storage capacity in KiB
 *
 * ── DMA-safe I/O buffers ───────────────────────────────────────────
 * @sl_buf:           256 B kmalloc scratch for Status-Link command/response
 *                    pairs.  Protected by @xfer_lock.
 * @pl_tag_buf:       270 B kmalloc buffer for Panel-Link tags.
 * @pl_frame_buf:     usb_alloc_coherent buffer sized for one full frame.
 *                    The xfer_worker copies @xfer_src into this buffer
 *                    before submitting one usb_bulk_msg() to the
 *                    Panel-Link OUT endpoint.  Coherent + below the 4 GB
 *                    boundary so 32-bit EHCI controllers DMA directly
 *                    without going through swiotlb (which under sustained
 *                    high-fps load would fragment and start failing with
 *                    -EAGAIN).
 * @pl_frame_buf_dma: DMA handle for @pl_frame_buf.
 *
 * ── Transfer serialisation and lifecycle flags ─────────────────────
 * @xfer_lock:    Mutex serialising ALL USB transfers (Panel-Link frame
 *                worker and Status-Link transactions).  Held by the
 *                xfer_worker for the duration of one frame and by
 *                Status-Link helpers for their entire command/response
 *                cycle.  Guarantees Panel-Link pixel chunks and
 *                Status-Link traffic never interleave on the wire.
 * @disconnected: Set true in beada_disconnect() while holding @xfer_lock.
 *                Permanent: every code path that touches USB checks this
 *                first and returns -ENODEV.
 * @suspended:    Set true between usb_driver->suspend and ->resume.
 *                Transient: device is expected to come back.  USB paths
 *                check this and return -EHOSTDOWN to avoid touching the
 *                bus while it's down.  Mainly a defense-in-depth net for
 *                system-suspend (S3); the autopm path is protected by
 *                pipe-lifecycle usb_autopm_get_interface() refcounting.
 *
 * ── Resume notification list ───────────────────────────────────────
 * @resume_handlers:      List of struct beada_resume_handler entries,
 *                         registered by child drivers in their probe()
 *                         and called by beada_usb_resume() after the
 *                         bus is back and Panel-Link is re-armed.
 *                         Used by DRM to re-send the shadow buffer and
 *                         by backlight to re-send the stored brightness.
 * @resume_handlers_lock: Protects the list during register/unregister
 *                         and iteration.  Separate from xfer_lock so a
 *                         handler can use the public API (which itself
 *                         takes xfer_lock) without recursion.
 *
 * ── Panel-Link dimension cache ─────────────────────────────────────
 * @cached_frame_w: Width latched into the firmware by the first START
 * @cached_frame_h: tag of this session.  Both zero after devm_kzalloc;
 *                  zeroed again on USB resume so the next frame will
 *                  re-send the START tag (the firmware may have lost
 *                  the latch over the suspend cycle).  Between those
 *                  resets the values are fixed: the firmware refuses
 *                  any further START tag until the next PL_RESET, so
 *                  pl_send() rejects mismatched dimensions to turn a
 *                  silent 270-byte misalignment into a clean error.
 *                  Accessed only under @xfer_lock.
 *
 * ── Async frame transfer ───────────────────────────────────────────
 * @xfer_wq:    Ordered workqueue running @xfer_work.
 * @xfer_work:  Work item; runs xfer_worker() which copies @xfer_src
 *              into @pl_frame_buf and submits one usb_bulk_msg().
 * @xfer_src:   Caller's source buffer; valid until next flush.
 * @xfer_len:   Source buffer length in bytes.
 * @xfer_w:     Frame width in pixels.
 * @xfer_h:     Frame height in pixels.
 *
 * Single-buffer model: there is no ping-pong.  Callers (the DRM child)
 * promise to call beada_flush_frame_transfer() before staging a new
 * frame, so the worker is guaranteed not to be racing with a fresh
 * send_frame().
 */
struct beada_mfd_dev {
	struct usb_device	*udev;
	struct usb_interface	*intf;
	struct device		*dev;

	u8			 ep_pl_out;
	u8			 ep_pl_in;
	u8			 ep_sl_out;
	u8			 ep_sl_in;
	bool			 has_statuslink;

	bool			 pl_bgr_format;

	struct beada_panel_info	 panel;
	u16			 firmware_version;
	u8			 panellink_version;
	u8			 statuslink_version;
	u8			 platform;
	char			 sn[65];	/* 64-char ASCII + NUL */
	u32			 storage_size_kb;

	u8			*sl_buf;
	u8			*pl_tag_buf;
	u8			*pl_frame_buf;
	dma_addr_t		 pl_frame_buf_dma;

	struct mutex		 xfer_lock;
	bool			 disconnected;
	bool			 suspended;

	/*
	 * in_resume: True only between the entry and exit of
	 * beada_usb_resume().  Used by public API functions that take a
	 * temporary autopm reference to detect that they are being called
	 * recursively from inside the resume callback — in which case the
	 * bus is already up, usb_autopm_get_interface() would deadlock
	 * waiting for resume to complete, and the autopm pair must be
	 * skipped.  Set/cleared by beada_usb_resume() under no lock; read
	 * with READ_ONCE elsewhere.
	 */
	bool			 in_resume;

	struct list_head	 resume_handlers;
	struct mutex		 resume_handlers_lock;

	u16			 cached_frame_w;
	u16			 cached_frame_h;

	struct workqueue_struct	*xfer_wq;
	struct work_struct	 xfer_work;
	const void		*xfer_src;
	size_t			 xfer_len;
	u16			 xfer_w;
	u16			 xfer_h;

	/* Pipeline tatistics */
	atomic_t stats_submitted;
	atomic_t stats_displayed;
	atomic_t stats_dropped;

	/*
	 * Resume-handler deferred dispatch.
	 *
	 * beada_usb_resume() must return quickly so the
	 * usb_autopm_get_interface() that triggered it can complete and
	 * the caller (e.g. pipe_enable) can proceed.  Running the
	 * resume handlers synchronously inside the callback delays that
	 * return by ~200 ms (PL_RESET + sync_time + per-handler USB) and
	 * has been observed to make SL_SET_BL time out with -ETIMEDOUT
	 * — the firmware appears to need breathing room between the
	 * resume sequence and subsequent Status-Link traffic.
	 *
	 * Instead, the callback schedules @resume_work via @xfer_wq
	 * (same ordered workqueue we use for frames, so handlers can
	 * not race with concurrent frame transfers).  The work runs
	 * after a short delay; by then resume is fully complete and
	 * the device is ready for SL_SET_BL.
	 */
	struct delayed_work	 resume_work;
};

/* ==================================================================
 * Cross-file function declarations
 * (grouped by the source file that implements each function)
 * ================================================================== */

/* -- beada-main.c ------------------------------------------------- */
/* No public declarations; module entry/exit only. */

/* -- beada-model.c ------------------------------------------------- */

/** Find geometry by model ID; never returns NULL. */
const struct beada_model_entry *beada_lookup_model(u8 os_version);

/**
 * Find geometry by model id; returns NULL if no entry matches.
 * Used as a fallback when Status-Link is unavailable and model id is
 * read from the USB iProduct string ("BeadaPanel WxH").
 */
const struct beada_model_entry *beada_lookup_model_by_modelid(char *model_id);

/**
 * Find geometry by pixel resolution; returns NULL if no entry matches.
 * Used as a fallback when Status-Link is unavailable and resolution is
 * read from the USB iProduct string ("BeadaPanel WxH").
 */
const struct beada_model_entry *beada_lookup_model_by_res(u16 width, u16 height);

/* -- beada-protocol.c ---------------------------------------------- */

/** RFC 1071 one's-complement 16-bit checksum; len must be even. */
__sum16 beada_checksum16(const void *data, size_t len);

/** Fill a Panel-Link tag (PL_TYPE_* / NULL fmtstr for non-START types). */
void pl_fill_tag(struct panellink_tag *tag, u8 type, const char *fmtstr);

/**
 * Fill a Status-Link header.  packet_len is the total packet size
 * (header + payload).  Checksum covers only the 18 header bytes before
 * checksum16 – never the payload (NXElec reference implementation).
 */
void sl_fill_tag(struct statuslink_tag *tag, u8 type, u16 packet_len);

/* -- beada-usb.c --------------------------------------------------- */

/** Synchronous bulk-OUT; caller holds xfer_lock. */
int beada_bulk_out(struct beada_mfd_dev *beada, u8 ep,
		   const void *data, size_t len, unsigned int timeout_ms);

/** Synchronous bulk-IN; caller holds xfer_lock. */
int beada_bulk_in(struct beada_mfd_dev *beada, u8 ep,
		  void *data, size_t len, unsigned int timeout_ms);

/** Discover bulk endpoints; sets ep_pl_*, ep_sl_*, has_statuslink. */
int beada_find_endpoints(struct beada_mfd_dev *beada);

/* -- beada-pipeline.c ---------------------------------------------- */

/**
 * beada_panellink_reset() - Reset the Panel-Link receiver state machine
 * @beada: Device handle
 *
 * Sends Status-Link type 2 (SL_TYPE_PL_RESET).  Required at probe to
 * recover from a "mid-frame" condition the firmware may be left in by
 * a previous driver session; without this the first frame's START tag
 * gets consumed as stale pixel data and the display is shifted by
 * sizeof(panellink_tag) = 270 bytes (~135 RGB565 pixels).
 *
 * Also called on USB resume for the same reason — the firmware may
 * have lost the dimension latch over the suspend cycle.
 *
 * Triggers a firmware-internal re-init that takes ~50 ms; the caller
 * must delay other Status-Link traffic by at least that long (the
 * device's standby clock also resets to its power-on default, which
 * is the visible cue that the re-init has run).
 *
 * Returns 0 on success, -EOPNOTSUPP if Status-Link is unavailable,
 * -ENODEV if disconnected, -EHOSTDOWN if suspended, or a USB error code.
 */
int beada_panellink_reset(struct beada_mfd_dev *beada);

/**
 * beada_query_device_info() - Populate panel info via Status-Link GET_INFO
 *
 * Sends GET_INFO, parses the response, and looks up the model in the
 * table.  Falls back gracefully if Status-Link is unavailable.  Must be
 * called at probe, before mfd_add_devices().
 */
int beada_query_device_info(struct beada_mfd_dev *beada);

/**
 * beada_sync_time() - Synchronise device RTC to current kernel time
 * @beada: Device handle
 *
 * Sends SL_TYPE_SET_TIME with current UTC as a Windows-style SYSTEMTIME
 * payload.  Called at probe, on resume (after PL_RESET wipes the RTC),
 * and on demand via the sysfs sync_time attribute.
 *
 * The sysfs path can be triggered while the device is autosuspended;
 * this function takes a temporary autopm reference so the bus is
 * resumed for the duration of the transfer.
 *
 * Returns 0 on success, -EOPNOTSUPP if Status-Link is unavailable,
 * -ENODEV if disconnected, or a USB error code.
 */
int beada_sync_time(struct beada_mfd_dev *beada);

/**
 * beada_pipeline_init() - Allocate workqueue and initialise work item
 *
 * Called from probe AFTER beada_query_device_info() has succeeded.
 * On disconnect/remove the workqueue is destroyed automatically via
 * the devm action registered here.
 */
int beada_pipeline_init(struct beada_mfd_dev *beada);

/* -- beada-pm.c ---------------------------------------------------- */

/**
 * beada_usb_suspend() / beada_usb_resume()
 *
 * Wired into struct usb_driver in beada-main.c.  On suspend we drain
 * the workqueue and flag @suspended.  On resume we clear @suspended,
 * re-arm Panel-Link with PL_RESET, zero the dimension cache, re-sync
 * the device RTC, and then call every registered resume handler so
 * children can re-push their state (DRM: shadow buffer, backlight:
 * stored brightness value).
 */
int beada_usb_suspend(struct usb_interface *intf, pm_message_t message);
int beada_usb_resume(struct usb_interface *intf);

/**
 * beada_resume_handlers_work() - Deferred resume-handler dispatch
 *
 * Workqueue function that runs resume handlers ~200 ms after the
 * usb_driver->resume callback returns.  See beada-priv.h's resume_work
 * field comment and beada-pm.c for rationale.
 */
void beada_resume_handlers_work(struct work_struct *work);

/* -- beada-sysfs.c ------------------------------------------------- */

/**
 * beada_sysfs_init() - Register the "beada_panel" attribute group
 *
 * Creates the "beadapanel" sysfs subdirectory under the USB interface
 * device with read-only info attributes (model, resolution, sn,
 * firmware_version, …) and the write-only sync_time action.  Uses devm
 * so no explicit cleanup is needed on disconnect.
 */
int beada_sysfs_init(struct beada_mfd_dev *beada);

#endif /* __BEADA_PRIV_H */
