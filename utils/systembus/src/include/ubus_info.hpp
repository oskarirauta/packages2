#ifndef _UBUS_SECURITY_HPP_
#define _UBUS_SECURITY_HPP_ 1

#include "ubus.hpp"

int systembus_info_list(struct ubus_context *ctx, struct ubus_object *obj,
		struct ubus_request_data *req, const char *method,
		struct blob_attr *msg);

int systembus_info_is_enabled(struct ubus_context *ctx, struct ubus_object *obj,
		struct ubus_request_data *req, const char *method,
		struct blob_attr *msg);

int systembus_info_release_name(struct ubus_context *ctx, struct ubus_object *obj,
		struct ubus_request_data *req, const char *method,
		struct blob_attr *msg);

#endif
