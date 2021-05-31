#ifndef _UBUS_CMDS_H_
#define _UBUS_CMDS_H_ 1

#include "ubus.h"

enum {
	CPU_ID,
	CPU_NAME,
	CPU_CMD,
	CPU_FIELDS,
	__GET_ARGS_MAX
};

extern const struct blobmsg_policy get_policy[];

int get_policy_size(void);

int cpubus_get(struct ubus_context *ctx, struct ubus_object *obj,
		struct ubus_request_data *req, const char *method,
		struct blob_attr *msg);

int cpubus_count(struct ubus_context *ctx, struct ubus_object *obj,
		struct ubus_request_data *req, const char *method,
		struct blob_attr *msg);

int cpubus_load(struct ubus_context *ctx, struct ubus_object *obj,
		  struct ubus_request_data *req, const char *method,
		  struct blob_attr *msg);

int cpubus_list(struct ubus_context *ctx, struct ubus_object *obj,
		struct ubus_request_data *req, const char *method,
		struct blob_attr *msg);

int cpubus_all(struct ubus_context *ctx, struct ubus_object *obj,
		struct ubus_request_data *req, const char *method,
		struct blob_attr *msg);

int cpubus_exit(struct ubus_context *ctx, struct ubus_object *obj,
		struct ubus_request_data *req, const char *method,
		struct blob_attr *msg);

#endif
