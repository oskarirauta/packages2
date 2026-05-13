// SPDX-License-Identifier: GPL-2.0
/*
 * mpro_pipeline.c — asynchronous USB framebuffer pipeline.
 *
 * Single transfer in flight; new frames replace any pending frame
 * (latest-wins coalescing). Provides natural backpressure: producer
 * (DRM) cannot outrun USB throughput, and stale frames are dropped
 * automatically when newer ones arrive.
 *
 * Optional LZ4 compression: if mpro->lz4_level > 0, frames are
 * compressed before submission. Compression uses a shared workmem
 * (mpro->lz4_workmem) protected by mpro->lz4_lock.
 *
 * Control commands (backlight, sleep mode, model query) bypass this
 * pipeline and use mpro_make_request() / mpro_send_command() in
 * mpro_protocol.c.
 */

#include <linux/module.h>
#include <linux/usb.h>
#include <linux/slab.h>
#include <linux/lz4.h>
#include "mpro_internal.h"
#include "mpro.h"

#define MPRO_CTRL_LEN_FULL		 6
#define MPRO_CTRL_LEN_PARTIAL		12
#define MPRO_CTRL_LEN_COMPRESSED	14

struct screen_command {
	u8	mode;
	u8	cmd;
	u8	size[4];
	__le16	x, y, w, h;
} __packed;

struct mpro_xfer {
	struct mpro_device	*mpro;
	bool			partial;

	void			*data;
	dma_addr_t		dma;
	size_t			len;
	bool			dma_coherent;

	u16 x, y, w, h;

	struct urb		*ctrl_urb;
	struct urb		*bulk_urb;
	struct usb_ctrlrequest	dr;
	struct screen_command	cmd;
	bool			compressed;
	size_t			orig_len;
};

/* ------------------------------------------------------------------ */
/* xfer alloc / free                                                  */
/* ------------------------------------------------------------------ */

static void mpro_xfer_free(struct mpro_xfer *xfer)
{
	struct mpro_device *mpro;

	if (!xfer)
		return;
	mpro = xfer->mpro;

	if (xfer->ctrl_urb)
		usb_free_urb(xfer->ctrl_urb);
	if (xfer->bulk_urb)
		usb_free_urb(xfer->bulk_urb);

	if (xfer->data) {
		if (xfer->dma_coherent)
			usb_free_coherent(mpro->udev, xfer->len,
					  xfer->data, xfer->dma);
		else if (xfer->compressed)
			kvfree(xfer->data);	/* kvmalloc-buffer */
		else
			kfree(xfer->data);
	}
	kfree(xfer);
}

static struct mpro_xfer *mpro_xfer_alloc(struct mpro_device *mpro,
					 bool partial,
					 const void *src, size_t len,
					 u16 x, u16 y, u16 w, u16 h)
{
	struct mpro_xfer *xfer;

	xfer = kzalloc(sizeof(*xfer), GFP_KERNEL);
	if (!xfer)
		return NULL;

	xfer->mpro = mpro;
	xfer->partial = partial;
	xfer->len = len;
	xfer->x = x;
	xfer->y = y;
	xfer->w = w;
	xfer->h = h;

	if (mpro->dma_dev) {
		xfer->data = usb_alloc_coherent(mpro->udev, len, GFP_KERNEL,
						&xfer->dma);
		if (xfer->data)
			xfer->dma_coherent = true;
	}
	if (!xfer->data) {
		xfer->data = kmalloc(len, GFP_KERNEL);
		if (!xfer->data)
			goto err;
	}
	if (src)
		memcpy(xfer->data, src, len);

	xfer->ctrl_urb = usb_alloc_urb(0, GFP_KERNEL);
	xfer->bulk_urb = usb_alloc_urb(0, GFP_KERNEL);
	if (!xfer->ctrl_urb || !xfer->bulk_urb)
		goto err;

	return xfer;
err:
	mpro_xfer_free(xfer);
	return NULL;
}

/* ------------------------------------------------------------------ */
/* LZ4 compression                                                    */
/* ------------------------------------------------------------------ */

bool mpro_firmware_supports_lz4(const struct mpro_device *mpro)
{
	if (mpro->fw_major < 0)
		return false;	/* unknown version → do not risk it */

	if (mpro->fw_major > MPRO_LZ4_MIN_MAJOR)
		return true;
	if (mpro->fw_major == MPRO_LZ4_MIN_MAJOR &&
	    mpro->fw_minor >= MPRO_LZ4_MIN_MINOR)
		return true;
	return false;
}
EXPORT_SYMBOL_GPL(mpro_firmware_supports_lz4);

static bool mpro_lz4_should_compress(struct mpro_device *mpro, size_t len)
{
	int level = READ_ONCE(mpro->lz4_level);

	if (level <= 0)
		return false;
	/* Margin devices do not support LZ4 or the partial protocol. */
	if (mpro->model && mpro->model->margin > 0)
		return false;
	if (!mpro_firmware_supports_lz4(mpro))
		return false;

	return len >= (size_t)READ_ONCE(mpro->lz4_threshold);
}

static struct mpro_xfer *mpro_xfer_alloc_compressed(struct mpro_device *mpro,
						    bool partial,
						    const void *src,
						    size_t srclen,
						    u16 x, u16 y, u16 w, u16 h)
{
	struct mpro_xfer *xfer;
	int level = READ_ONCE(mpro->lz4_level);
	int compressed_max = LZ4_compressBound(srclen);
	int compressed_size;
	void *compressed;

	if (level <= 0 || !mpro->lz4_workmem)
		return NULL;

	/*
	 * kvmalloc — the upper bound for a full frame is large (~770KB)
	 * and a plain kmalloc may fail under memory fragmentation.
	 */
	compressed = kvmalloc(compressed_max, GFP_KERNEL);
	if (!compressed)
		return NULL;

	/* workmem is shared and LZ4 is not reentrant */
	mutex_lock(&mpro->lz4_lock);

	if (level == 1)
		compressed_size = LZ4_compress_default(src, compressed,
						       srclen, compressed_max,
						       mpro->lz4_workmem);
	else
		compressed_size = LZ4_compress_HC(src, compressed,
						  srclen, compressed_max,
						  level, mpro->lz4_workmem);

	mutex_unlock(&mpro->lz4_lock);

	if (compressed_size <= 0 || compressed_size >= srclen) {
		/* No win or compressor error — fall back to the raw path */
		kvfree(compressed);
		return NULL;
	}

	xfer = kzalloc(sizeof(*xfer), GFP_KERNEL);
	if (!xfer) {
		kvfree(compressed);
		return NULL;
	}

	xfer->mpro = mpro;
	xfer->partial = partial;
	xfer->compressed = true;
	xfer->orig_len = srclen;
	xfer->len = compressed_size;
	xfer->x = x;
	xfer->y = y;
	xfer->w = w;
	xfer->h = h;
	xfer->data = compressed;
	xfer->dma_coherent = false;

	xfer->ctrl_urb = usb_alloc_urb(0, GFP_KERNEL);
	xfer->bulk_urb = usb_alloc_urb(0, GFP_KERNEL);
	if (!xfer->ctrl_urb || !xfer->bulk_urb)
		goto err;

	return xfer;

err:
	mpro_xfer_free(xfer);
	return NULL;
}

/* ------------------------------------------------------------------ */
/* URB completion + state machine                                     */
/* ------------------------------------------------------------------ */

static int mpro_xfer_submit(struct mpro_xfer *xfer);

static void mpro_complete_work(struct work_struct *work)
{
	struct mpro_device *mpro =
	    container_of(work, struct mpro_device, complete_work);
	struct mpro_xfer *done = NULL, *next = NULL;
	unsigned long flags;
	int ret;

	spin_lock_irqsave(&mpro->state_lock, flags);
	done = mpro->current_xfer;
	if (mpro->pending_xfer && mpro->running) {
		next = mpro->pending_xfer;
		mpro->pending_xfer = NULL;
		mpro->current_xfer = next;
		/* in_flight stays true */
	} else {
		mpro->current_xfer = NULL;
		mpro->in_flight = false;
	}
	spin_unlock_irqrestore(&mpro->state_lock, flags);

	if (done)
		mpro_xfer_free(done);

	if (!next)
		return;

	ret = mpro_xfer_submit(next);
	if (ret) {
		spin_lock_irqsave(&mpro->state_lock, flags);
		mpro->current_xfer = NULL;
		mpro->in_flight = false;
		spin_unlock_irqrestore(&mpro->state_lock, flags);
		mpro_xfer_free(next);
	}
}

static void mpro_bulk_complete(struct urb *urb)
{
    struct mpro_xfer *xfer = urb->context;
    struct mpro_device *mpro = xfer->mpro;

    if (urb->status && urb->status != -ENOENT &&
        urb->status != -ESHUTDOWN && urb->status != -ECONNRESET)
        dev_dbg(&mpro->intf->dev, "bulk URB status %d\n", urb->status);

    if (urb->status == 0) {
        u64 now_ns = ktime_get_ns();
        u64 prev_ns = atomic64_xchg(&mpro->last_frame_ns, now_ns);

        atomic_inc(&mpro->stats_displayed);

        if (prev_ns) {
            u64 delta_ns = now_ns - prev_ns;
            unsigned long flags;

            spin_lock_irqsave(&mpro->fps_lock, flags);
            if (mpro->ewma_period_ns == 0)
                mpro->ewma_period_ns = (u32)min_t(u64, delta_ns, U32_MAX);
            else {
                /* EWMA α=1/16 */
                u32 d = (u32)min_t(u64, delta_ns, U32_MAX);
                mpro->ewma_period_ns =
                    mpro->ewma_period_ns -
                    (mpro->ewma_period_ns >> 4) +
                    (d >> 4);
            }
            spin_unlock_irqrestore(&mpro->fps_lock, flags);
        }
    }

    queue_work(mpro->wq, &mpro->complete_work);
}

static void mpro_ctrl_complete(struct urb *urb)
{
	struct mpro_xfer *xfer = urb->context;
	struct mpro_device *mpro = xfer->mpro;
	int ret;

	if (urb->status) {
		if (urb->status != -ENOENT && urb->status != -ESHUTDOWN &&
		    urb->status != -ECONNRESET)
			dev_dbg(&mpro->intf->dev, "ctrl URB status %d\n",
				urb->status);
		queue_work(mpro->wq, &mpro->complete_work);
		return;
	}

	/* Submit the bulk URB from atomic context — GFP_ATOMIC */
	usb_fill_bulk_urb(xfer->bulk_urb, mpro->udev,
			  usb_sndbulkpipe(mpro->udev, 2),
			  xfer->dma_coherent ? NULL : xfer->data,
			  xfer->len, mpro_bulk_complete, xfer);

	if (xfer->dma_coherent) {
		xfer->bulk_urb->transfer_dma = xfer->dma;
		xfer->bulk_urb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;
	}

	usb_anchor_urb(xfer->bulk_urb, &mpro->anchor);
	ret = usb_submit_urb(xfer->bulk_urb, GFP_ATOMIC);
	if (ret) {
		usb_unanchor_urb(xfer->bulk_urb);
		dev_warn_ratelimited(&mpro->intf->dev,
				     "bulk submit failed: %d\n", ret);
		queue_work(mpro->wq, &mpro->complete_work);
	}
}

static int mpro_xfer_submit(struct mpro_xfer *xfer)
{
	struct mpro_device *mpro = xfer->mpro;
	__le32 le_len = cpu_to_le32(xfer->len);
	size_t cmd_len;
	int ret;

	/* mode: 0x04 = LZ4-compressed, 0x00 = raw */
	xfer->cmd.mode = xfer->compressed ? 0x04 : 0x00;
	xfer->cmd.cmd = 0x2c;
	memcpy(xfer->cmd.size, &le_len, 4);
	xfer->cmd.x = cpu_to_le16(xfer->x);
	xfer->cmd.y = cpu_to_le16(xfer->y);
	xfer->cmd.w = cpu_to_le16(xfer->w);
	xfer->cmd.h = cpu_to_le16(xfer->h);

	/*
	 * Command length on the wire:
	 *   full:        6 — mode + cmd + size (4 bytes)
	 *   partial:    12 — full + x + y + w (h omitted)
	 *   compressed: 14 — full + x + y + w + h
	 *
	 * For partial transfers the device infers h from the data, or
	 * doesn't need it. For compressed transfers all fields are
	 * required so the device can decompress into the right region.
	 */
	cmd_len = xfer->compressed ? MPRO_CTRL_LEN_COMPRESSED :
		  xfer->partial    ? MPRO_CTRL_LEN_PARTIAL :
				     MPRO_CTRL_LEN_FULL;

	xfer->dr.bRequestType	= MPRO_USB_OUT;
	xfer->dr.bRequest	= MPRO_CMD;
	xfer->dr.wValue		= cpu_to_le16(0);
	xfer->dr.wIndex		= cpu_to_le16(0);
	xfer->dr.wLength	= cpu_to_le16(cmd_len);

	usb_fill_control_urb(xfer->ctrl_urb, mpro->udev,
			     usb_sndctrlpipe(mpro->udev, 0),
			     (unsigned char *)&xfer->dr,
			     &xfer->cmd, cmd_len, mpro_ctrl_complete, xfer);

	usb_anchor_urb(xfer->ctrl_urb, &mpro->anchor);
	ret = usb_submit_urb(xfer->ctrl_urb, GFP_KERNEL);
	if (ret)
		usb_unanchor_urb(xfer->ctrl_urb);
	return ret;
}

/* ------------------------------------------------------------------ */
/* Public API                                                         */
/* ------------------------------------------------------------------ */

static int mpro_send_xfer(struct mpro_device *mpro, struct mpro_xfer *xfer)
{
	struct mpro_xfer *displaced = NULL;
	unsigned long flags;
	bool need_submit;
	int ret;

	spin_lock_irqsave(&mpro->state_lock, flags);

	if (!mpro->running) {
		spin_unlock_irqrestore(&mpro->state_lock, flags);
		mpro_xfer_free(xfer);
		return -ESHUTDOWN;
	}

	/*
	 * Drop frame silently when the display is idle. The compositor
	 * may keep rendering while we are dark (backlight off, black
	 * frame shown); returning 0 avoids confusing it with an error.
	 * Frames resume flowing once wake_work clears is_idle.
	 */
	if (READ_ONCE(mpro->is_idle)) {
		spin_unlock_irqrestore(&mpro->state_lock, flags);
		atomic_inc(&mpro->stats_dropped);
		mpro_xfer_free(xfer);
		return 0;
	}

	atomic_inc(&mpro->stats_submitted);

	if (!mpro->in_flight) {
		mpro->in_flight = true;
		mpro->current_xfer = xfer;
		need_submit = true;
	} else {
		displaced = mpro->pending_xfer;	/* coalesce */
		mpro->pending_xfer = xfer;
		need_submit = false;
	}
	spin_unlock_irqrestore(&mpro->state_lock, flags);

	if (displaced) {
		atomic_inc(&mpro->stats_dropped);
		mpro_xfer_free(displaced);
	}

	if (!need_submit)
		return 0;

	ret = mpro_xfer_submit(xfer);
	if (ret) {
		spin_lock_irqsave(&mpro->state_lock, flags);
		mpro->current_xfer = NULL;
		mpro->in_flight = false;
		spin_unlock_irqrestore(&mpro->state_lock, flags);
		mpro_xfer_free(xfer);
	}
	return ret;
}

int mpro_send_full_frame(struct mpro_device *mpro, const void *data, size_t len)
{
	struct mpro_xfer *xfer = NULL;

	if (!mpro || !data || !len)
		return -EINVAL;

	if (mpro_lz4_should_compress(mpro, len)) {
		/*
		 * LZ4 requires cmd_len=14 with w,h set to the actual frame
		 * dimensions. The device treats partial(0,0,full_w,full_h)
		 * as a full frame, so we always use the partial protocol
		 * when compression is active.
		 */
		xfer = mpro_xfer_alloc_compressed(mpro, true, data, len,
						  0, 0,
						  mpro->model->width,
						  mpro->model->height);
	}

	if (!xfer)
		xfer = mpro_xfer_alloc(mpro, false, data, len, 0, 0, 0, 0);
	if (!xfer)
		return -ENOMEM;

	return mpro_send_xfer(mpro, xfer);
}
EXPORT_SYMBOL_GPL(mpro_send_full_frame);

int mpro_send_partial_frame(struct mpro_device *mpro,
			    const void *data, u16 x, u16 y, u16 w, u16 h)
{
	struct mpro_xfer *xfer = NULL;
	size_t len;
	int fy;

	if (!mpro || !mpro->model || !data || !w || !h)
		return -EINVAL;

	/* Protocol quirk: the device's y-axis origin is bottom-left */
	fy = (int)mpro->model->height - 1 - (y + h);
	if (fy < 0)
		return -EINVAL;

	len = (size_t)w *h * 2;

	if (mpro_lz4_should_compress(mpro, len))
		xfer = mpro_xfer_alloc_compressed(mpro, true, data, len,
						  x, fy, w, h);

	if (!xfer)
		xfer = mpro_xfer_alloc(mpro, true, data, len, x, fy, w, h);
	if (!xfer)
		return -ENOMEM;

	return mpro_send_xfer(mpro, xfer);
}
EXPORT_SYMBOL_GPL(mpro_send_partial_frame);

/* ------------------------------------------------------------------ */
/* Init / shutdown — called from mpro_core.c                          */
/* ------------------------------------------------------------------ */

void mpro_io_init(struct mpro_device *mpro)
{
	spin_lock_init(&mpro->state_lock);
	spin_lock_init(&mpro->fps_lock);
	atomic64_set(&mpro->last_frame_ns, 0);
	mpro->ewma_period_ns = 0;
	init_usb_anchor(&mpro->anchor);
	INIT_WORK(&mpro->complete_work, mpro_complete_work);
	mpro->in_flight = false;
	mpro->current_xfer = NULL;
	mpro->pending_xfer = NULL;
}
EXPORT_SYMBOL_GPL(mpro_io_init);

void mpro_io_shutdown(struct mpro_device *mpro)
{
	struct mpro_xfer *cur, *pend;
	unsigned long flags;

	/*
	 * The caller must have set mpro->running = false already so no
	 * new submissions can come in. Now kill any in-flight URBs and
	 * drain the work queue.
	 */
	usb_kill_anchored_urbs(&mpro->anchor);
	cancel_work_sync(&mpro->complete_work);

	spin_lock_irqsave(&mpro->state_lock, flags);
	cur = mpro->current_xfer;
	pend = mpro->pending_xfer;
	mpro->current_xfer = NULL;
	mpro->pending_xfer = NULL;
	mpro->in_flight = false;
	spin_unlock_irqrestore(&mpro->state_lock, flags);

	mpro_xfer_free(cur);
	mpro_xfer_free(pend);
}
EXPORT_SYMBOL_GPL(mpro_io_shutdown);
