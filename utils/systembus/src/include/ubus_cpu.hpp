#ifndef _UBUS_CPU_H_
#define _UBUS_CPU_H_ 1

#include "ubus.hpp"

enum {
	CPU_ID,
	CPU_NAME,
	CPU_CMD,
	CPU_FIELDS,
	__GET_CPU_ARGS_MAX
};

extern const struct blobmsg_policy cpu_policy[];

int cpu_policy_size(void);

int systembus_cpu_get(struct ubus_context *ctx, struct ubus_object *obj,
		struct ubus_request_data *req, const char *method,
		struct blob_attr *msg);

int systembus_cpu_count(struct ubus_context *ctx, struct ubus_object *obj,
		struct ubus_request_data *req, const char *method,
		struct blob_attr *msg);

int systembus_cpu_load(struct ubus_context *ctx, struct ubus_object *obj,
		  struct ubus_request_data *req, const char *method,
		  struct blob_attr *msg);

int systembus_cpu_all(struct ubus_context *ctx, struct ubus_object *obj,
		struct ubus_request_data *req, const char *method,
		struct blob_attr *msg);

int systembus_cpu_list(struct ubus_context *ctx, struct ubus_object *obj,
		struct ubus_request_data *req, const char *method,
		struct blob_attr *msg);

#endif
