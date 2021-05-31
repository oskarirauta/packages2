#ifndef _UBUS_H_
#define _UBUS_H_ 1

extern "C" {
#include <libubox/blobmsg_json.h>
#include <libubus.h>
}

typedef enum {
	FIELD_ID = 0,
	FIELD_COUNT,
	FIELD_NAME,
	FIELD_VENDOR,
	FIELD_MODEL,
	FIELD_MHZ,
	FIELD_LOAD,
	NUM_FIELDS
} field_type;

extern struct ubus_context *ctx;
extern struct blob_buf b;

int ubus_create(void);

#endif
