// SPDX-License-Identifier: GPL-2.0
/*
 * mpro_touch.c — USB interrupt-endpoint pipeline for touch input.
 *
 * Owns the interrupt URB (endpoint 0x81) and its DMA buffer. On each
 * completed transfer the raw packet is forwarded to all registered
 * mpro_touch_listener callbacks so that the mpro_touchscreen child
 * driver can parse and report coordinates without touching USB directly.
 *
 * Start/stop control is delegated to the child: it calls
 * mpro_touch_start() / mpro_touch_stop() based on userspace open/close
 * and screen-state transitions. The parent only owns the hardware
 * resources.
 */

#include <linux/module.h>
#include <linux/usb.h>
#include <linux/slab.h>

#include "mpro_internal.h"
#include "mpro.h"

#define MPRO_TOUCH_EP          0x81
#define MPRO_TOUCH_PKT_SIZE    14
#define MPRO_TOUCH_INTERVAL_MS 8

static void mpro_touch__irq_complete(struct urb *urb)
{
	struct mpro_device *mpro = urb->context;
	struct mpro_touch_listener *l;
	unsigned long flags;
	int ret;

	switch (urb->status) {
	case 0:
		if (urb->actual_length >= MPRO_TOUCH_PKT_SIZE) {
			spin_lock_irqsave(&mpro->touch_listeners_lock, flags);
			list_for_each_entry(l, &mpro->touch_listeners, node)
				if (l->touch_packet)
					l->touch_packet(l->priv,
							urb->transfer_buffer,
							urb->actual_length);
			spin_unlock_irqrestore(&mpro->touch_listeners_lock,
					       flags);
		}
		break;

	case -ENOENT:
	case -ECONNRESET:
	case -ESHUTDOWN:
	case -EPROTO:
		/*
		 * Fatal: device gone, suspend, or explicit kill via
		 * mpro_touch_stop(). Do not resubmit.
		 */
		atomic_set(&mpro->touch_submitted, 0);
		return;

	default:
		/* Transient error (e.g. -EILSEQ CRC) — retry */
		dev_dbg(&mpro->intf->dev, "touch URB status %d, retrying\n",
			urb->status);
		break;
	}

	/*
	 * Check submitted before resubmitting. If mpro_touch_stop()
	 * cleared it while this completion was running, skip the
	 * resubmit — stop() has already called usb_kill_urb() which
	 * will deliver -ENOENT or -ECONNRESET to the next completion,
	 * preventing any further resubmission.
	 */
	if (!atomic_read(&mpro->touch_submitted))
		return;

	ret = usb_submit_urb(urb, GFP_ATOMIC);
	if (ret) {
		dev_warn_ratelimited(&mpro->intf->dev,
				     "touch URB resubmit failed: %d\n", ret);
		atomic_set(&mpro->touch_submitted, 0);
	}
}

int mpro_touch_start(struct mpro_device *mpro)
{
	int ret;

	if (!mpro->touch_urb)
		return -ENODEV;

	/* Submit only if not already running */
	if (atomic_cmpxchg(&mpro->touch_submitted, 0, 1) != 0)
		return 0;

	mpro->touch_urb->dev = mpro->udev;
	ret = usb_submit_urb(mpro->touch_urb, GFP_KERNEL);
	if (ret) {
		dev_warn(&mpro->intf->dev,
			 "touch URB submit failed: %d\n", ret);
		atomic_set(&mpro->touch_submitted, 0);
	}
	return ret;
}
EXPORT_SYMBOL_GPL(mpro_touch_start);

void mpro_touch_stop(struct mpro_device *mpro)
{
	if (!mpro->touch_urb || !atomic_read(&mpro->touch_submitted))
		return;

	usb_kill_urb(mpro->touch_urb);
	atomic_set(&mpro->touch_submitted, 0);
}
EXPORT_SYMBOL_GPL(mpro_touch_stop);

int mpro_touch_listener_register(struct mpro_device *mpro,
				 struct mpro_touch_listener *l)
{
	unsigned long flags;

	spin_lock_irqsave(&mpro->touch_listeners_lock, flags);
	list_add_tail(&l->node, &mpro->touch_listeners);
	spin_unlock_irqrestore(&mpro->touch_listeners_lock, flags);
	return 0;
}
EXPORT_SYMBOL_GPL(mpro_touch_listener_register);

void mpro_touch_listener_unregister(struct mpro_device *mpro,
				    struct mpro_touch_listener *l)
{
	unsigned long flags;

	if (!mpro || !l)
		return;

	spin_lock_irqsave(&mpro->touch_listeners_lock, flags);
	list_del(&l->node);
	spin_unlock_irqrestore(&mpro->touch_listeners_lock, flags);
}
EXPORT_SYMBOL_GPL(mpro_touch_listener_unregister);

int mpro_touch_init(struct mpro_device *mpro)
{
	int pipe;

	spin_lock_init(&mpro->touch_listeners_lock);
	INIT_LIST_HEAD(&mpro->touch_listeners);
	atomic_set(&mpro->touch_submitted, 0);

	mpro->touch_urb = usb_alloc_urb(0, GFP_KERNEL);
	if (!mpro->touch_urb)
		return -ENOMEM;

	mpro->touch_buf = usb_alloc_coherent(mpro->udev, MPRO_TOUCH_PKT_SIZE,
					     GFP_KERNEL, &mpro->touch_buf_dma);
	if (!mpro->touch_buf) {
		usb_free_urb(mpro->touch_urb);
		mpro->touch_urb = NULL;
		return -ENOMEM;
	}

	pipe = usb_rcvintpipe(mpro->udev, MPRO_TOUCH_EP);
	usb_fill_int_urb(mpro->touch_urb, mpro->udev, pipe,
			 mpro->touch_buf, MPRO_TOUCH_PKT_SIZE,
			 mpro_touch__irq_complete, mpro,
			 MPRO_TOUCH_INTERVAL_MS);
	mpro->touch_urb->transfer_dma    = mpro->touch_buf_dma;
	mpro->touch_urb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;

	return 0;
}
EXPORT_SYMBOL_GPL(mpro_touch_init);

void mpro_touch_shutdown(struct mpro_device *mpro)
{
	mpro_touch_stop(mpro);

	if (mpro->touch_buf) {
		usb_free_coherent(mpro->udev, MPRO_TOUCH_PKT_SIZE,
				  mpro->touch_buf, mpro->touch_buf_dma);
		mpro->touch_buf = NULL;
	}

	usb_free_urb(mpro->touch_urb);
	mpro->touch_urb = NULL;
}
EXPORT_SYMBOL_GPL(mpro_touch_shutdown);
