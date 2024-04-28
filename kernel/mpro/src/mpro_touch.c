#include "mpro.h"

#define TOUCHSCREEN_DESC             "VoCore Touch"

union axis {

	struct hx {
		unsigned char h:4;
		unsigned char u:2;
		unsigned char f:2;
	} x;

	struct hy {
		unsigned char h:4;
		unsigned char id:4;
	} y;

	char c;
};

struct point {

	union axis xh;
	unsigned char xl;
	union axis yh;
	unsigned char yl;

	unsigned char weight;
	unsigned char misc;
};

struct touch {

	unsigned char unused[2];
	unsigned char count;
	struct point p[2];
};

static void mpro_touch_irq(struct urb *urb) {

	struct mpro_device *mpro = urb -> context;
	struct device *dev = mpro -> dmadev;
	struct touch *t = (struct touch *)mpro -> touch.data;

	/* urb -> status == ENOENT: closed by usb_kill_urb */
	if ( urb -> status )
		return;

	int x = (((int)t -> p[0].xh.x.h) << 8) + t -> p[0].xl;
	int y = (((int)t -> p[0].yh.y.h) << 8) + t -> p[0].yl;
	int touch = (t -> p[0].xh.x.f != 1) ? 1 : 0;

	input_report_key(mpro -> touch.input, BTN_TOUCH, touch);
	input_report_abs(mpro -> touch.input, ABS_X, x);
	input_report_abs(mpro -> touch.input, ABS_Y, y);

	input_sync(mpro -> touch.input);
	dev_dbg(dev, "touch x=%d, y=%d, key=%d.\n", x, y, touch);

	int ret = usb_submit_urb(urb, GFP_ATOMIC);
	if ( ret )
		dev_err(dev, "usb_submit_urb failed at irq: %d\n", ret);
}

static int mpro_touch_open(struct input_dev *input) {

	struct mpro_device *mpro = input_get_drvdata(input);

	int ret = usb_submit_urb(mpro -> touch.irq, GFP_KERNEL);
	if ( ret ) {
		dev_err(mpro -> dmadev, "usb_submit_urb failed to open: %d\n", ret);
		return -EIO;
	}

	return 0;
}

static void mpro_touch_close(struct input_dev *input) {

	struct mpro_device *mpro = input_get_drvdata(input);
	usb_kill_urb(mpro -> touch.irq);
}


void mpro_touch_init(struct mpro_device *mpro) {

	struct usb_device *udev = mpro_to_usb_device(mpro);

	mpro -> touch.irq = usb_alloc_urb(0, GFP_KERNEL);

	if ( !mpro -> touch.irq ) {
		dev_err(mpro -> dmadev, "unable to allocate usb irq for touch screen\n");
		return;
	}

	mpro -> touch.data_size = 14; // touch information data length
	mpro -> touch.data = usb_alloc_coherent(udev, mpro -> touch.data_size,
						GFP_KERNEL, &mpro -> touch.dma);

	usb_fill_int_urb(mpro -> touch.irq, udev,
			 usb_rcvintpipe(udev, 1), mpro -> touch.data,
			 mpro -> touch.data_size, mpro_touch_irq,
			 mpro, 0);

	mpro -> touch.irq -> dev = udev;
	mpro -> touch.irq -> transfer_dma = mpro -> touch.dma;
	mpro -> touch.irq -> transfer_flags |= URB_NO_TRANSFER_DMA_MAP;

	/* register input device for touch */
	usb_make_path(udev, mpro -> touch.phys, sizeof(mpro -> touch.phys));
	strlcat(mpro -> touch.phys, "/input0", sizeof(mpro -> touch.phys));
	snprintf(mpro -> touch.name, sizeof(mpro -> touch.name), TOUCHSCREEN_DESC);

	mpro -> touch.input = input_allocate_device();
	usb_to_input_id(udev, &mpro -> touch.input -> id);

	mpro -> touch.input -> name = mpro -> touch.name;
	mpro -> touch.input -> phys = mpro -> touch.phys;
	mpro -> touch.input -> dev.parent = mpro -> dmadev;

	mpro -> touch.input -> evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_ABS);
	mpro -> touch.input -> keybit[BIT_WORD(BTN_TOUCH)] = BIT_MASK(BTN_TOUCH);
	input_set_abs_params(mpro -> touch.input, ABS_X, 0, mpro -> width, 0, 0);
	input_set_abs_params(mpro -> touch.input, ABS_Y, 0, mpro -> height, 0, 0);

	mpro -> touch.input -> open = mpro_touch_open;
	mpro -> touch.input -> close = mpro_touch_close;
	input_set_drvdata(mpro -> touch.input, mpro);

	if ( input_register_device(mpro -> touch.input)) {

		dev_err(mpro -> dmadev, "unable to register touch screen\n");
		usb_free_urb(mpro -> touch.irq);
		usb_free_coherent(udev, mpro -> touch.data_size, mpro -> touch.data, mpro -> touch.dma);
		mpro -> touch.irq = NULL;
	}
}

void mpro_touch_exit(struct mpro_device *mpro) {

	if ( !mpro -> touch.irq )
		return;

	struct usb_device *udev = mpro_to_usb_device(mpro);

	input_unregister_device(mpro -> touch.input);
	usb_free_urb(mpro -> touch.irq);
	usb_free_coherent(udev, mpro -> touch.data_size, mpro -> touch.data, mpro -> touch.dma);
	mpro -> touch.irq = NULL;
}
