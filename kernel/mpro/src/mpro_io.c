#include "mpro.h"

struct usb_device *mpro_to_usb_device(struct mpro_device *mpro) {

	return interface_to_usbdev(to_usb_interface(mpro -> dev.dev));
}

int mpro_send_command(struct mpro_device *mpro, void* cmd, unsigned int len) {

	struct usb_device *udev = mpro_to_usb_device(mpro);

	if ( len == 0 )
		return 0;

	if ( mpro -> partial == 1 ) {

		if ( len > 64 )
			return -EINVAL;

		/* Make sure use malloc memory space */
		memcpy(mpro -> cmd, cmd, len);

		/* 0x40 USB_DIR_OUT | USB_TYPE VENDOR | USB_RECIP_DEVICE */
		return usb_control_msg(udev, usb_sndctrlpipe(udev, 0),
				       0xb0, 0x40, 0, 0, mpro -> cmd, len,
				       MPRO_MAX_DELAY);

	}

	return usb_control_msg(udev, usb_sndctrlpipe(udev, 0),
			       0xb0, 0x40, 0, 0, cmd, len,
			       MPRO_MAX_DELAY);
}
