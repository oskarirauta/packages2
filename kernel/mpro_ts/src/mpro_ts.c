#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/usb.h>
#include <linux/poll.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/input.h>
#include <linux/usb/input.h>

struct mpro_ts {

	struct usb_interface *interface;
	struct usb_device *udev;
	struct input_dev *input;
	char name[64];
	char phys[64];
	struct urb *irq;
	dma_addr_t dma;
	int data_size;
	unsigned char *data;
	int width;
	int height;
};

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

	struct mpro_ts *info = urb->context;
	struct device *dev = &info->interface->dev;
	struct touch *t = (struct touch *)info->data;
	int ret, x, y, touch;

	/* urb->status == ENOENT: closed by usb_kill_urb */
	if (urb->status)
		return;

	x = (((int)t->p[0].xh.x.h) << 8) + t->p[0].xl;
	y = (((int)t->p[0].yh.y.h) << 8) + t->p[0].yl;
	touch = (t->p[0].xh.x.f != 1) ? 1 : 0;

	input_report_key(info->input, BTN_TOUCH, touch);
	input_report_abs(info->input, ABS_X, x);
	input_report_abs(info->input, ABS_Y, y);
	input_sync(info->input);
	dev_dbg(dev, "touch x=%d, y=%d, key=%d\n", x, y, touch);

	ret = usb_submit_urb(urb, GFP_ATOMIC);
	if (ret)
		dev_err(dev, "usb_submit_urb failed at irq: %d\n", ret);
}

static int mpro_touch_open(struct input_dev *input)
{
	struct mpro_ts *info = input_get_drvdata(input);
	int ret;

	ret = usb_submit_urb(info->irq, GFP_KERNEL);
	if (ret) {
		dev_err(&info->interface->dev,
			"usb_submit_urb failed at open: %d\n", ret);
		return -EIO;
	}

	return 0;
}

static void mpro_touch_close(struct input_dev *input)
{
	struct mpro_ts *info = input_get_drvdata(input);
	usb_kill_urb(info->irq);
}

static int mpro_ts_probe(struct usb_interface *interface, const struct usb_device_id *id) {

	struct mpro_ts *info = NULL;
	int ret;

	info = kzalloc(sizeof(struct mpro_ts), GFP_KERNEL);
	if (info == NULL) {
		ret = -ENOMEM;
		dev_err(&interface->dev, "Out of memory\n");
		goto error_release;
	}

	info -> interface = interface;
	usb_set_intfdata(interface, info);

	info -> udev = usb_get_dev(interface_to_usbdev(interface));

	/* detect screen here and set width and height */

	info -> width = 800;
	info -> height = 480;

	//info->udev = usb_get_dev(udev);
	usb_set_intfdata(interface, info);

	info->irq = usb_alloc_urb(0, GFP_KERNEL);
	if (!info->irq) {
		dev_err(&interface->dev, "unable to allocate usb irq\n");
		goto error_release;
	}

	info->data_size = 14;	/* touch infomation data length */
	info->data = usb_alloc_coherent(info->udev, info->data_size,
					 GFP_KERNEL, &info->dma);
	usb_fill_int_urb(info->irq, info->udev,
			 usb_rcvintpipe(info->udev, 1), info->data,
			 info->data_size, mpro_touch_irq, info, 0);
	info->irq->dev = info->udev;
	info->irq->transfer_dma = info->dma;
	info->irq->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;

	/* register input device for touch */
	usb_make_path(info->udev, info->phys, sizeof(info->phys));
	strlcat(info->phys, "/input0", sizeof(info->phys));
	snprintf(info->name, sizeof(info->name), "VoCore Touch");

	info->input = input_allocate_device();
	usb_to_input_id(info->udev, &info->input->id);
	info->input->name = info->name;
	info->input->phys = info->phys;
	info->input->dev.parent = &interface->dev;

	info->input->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_ABS);
	info->input->keybit[BIT_WORD(BTN_TOUCH)] = BIT_MASK(BTN_TOUCH);
	input_set_abs_params(info->input, ABS_X, 0, info->width, 0, 0);
	input_set_abs_params(info->input, ABS_Y, 0, info->height, 0, 0);

	info->input->open = mpro_touch_open;
	info->input->close = mpro_touch_close;
	input_set_drvdata(info->input, info);

	ret = input_register_device(info->input);
	if (ret) {
		dev_err(&interface->dev, "unable to register touch device\n");
		goto error_free_urb;
	}

	dev_info(&interface->dev, "mpro touch device: width=%d height=%d\n",
		 info->width, info->height);

	return 0;

error_free_urb:
	usb_free_urb(info->irq);
	usb_free_coherent(info->udev, info->data_size, info->data,
			  info->dma);

 error_release:
	kfree(info);
	return ret;
}

static void mpro_ts_disconnect(struct usb_interface *interface)
{
	struct mpro_ts *info = usb_get_intfdata(interface);

	input_unregister_device(info->input);
	usb_free_urb(info->irq);
	usb_free_coherent(info->udev, info->data_size, info->data,
			  info->dma);
	kfree(info);
	dev_info(&interface->dev, "mpro touch screen device disconnected\n");
}

static const struct usb_device_id mpro_ids[] = {
	{USB_DEVICE(0xc872, 0x1004)},
	{}
};

MODULE_DEVICE_TABLE(usb, mpro_ids);

static struct usb_driver mpro_ts_driver = {
	.name =		"mpro_ts",
	.probe =	mpro_ts_probe,
	.disconnect =	mpro_ts_disconnect,
	.id_table =	mpro_ids,
};

module_usb_driver(mpro_ts_driver);

MODULE_AUTHOR("Qin Wei (me@vonger.cn)");
MODULE_AUTHOR("Oskari Rauta (oskari.rauta@gmail.com)");
MODULE_DESCRIPTION("VoCore USB touch screen driver");
MODULE_LICENSE("GPL");
