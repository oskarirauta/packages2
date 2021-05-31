#include <string>

#include "common.hpp"
#include "cpu.hpp"
#include "shared.hpp"
#include "info.hpp"
#include "ubus_info.hpp"

int systembus_info_list(struct ubus_context *ctx, struct ubus_object *obj,
                struct ubus_request_data *req, const char *method,
                struct blob_attr *msg) {

	logDebug << APP_NAME << ": ubus call info::list received" << std::endl;
	std::lock_guard<std::mutex> guard(info_mutex);

	blob_buf_init(&b, 0);
	blobmsg_add_u8(&b, "systembus_loaded", true);
	blobmsg_add_string(&b, "release_name", info_data -> release_name.c_str());
	ubus_send_reply(ctx, req, b.head);
	return 0;
}

int systembus_info_is_enabled(struct ubus_context *ctx, struct ubus_object *obj,
		struct ubus_request_data *req, const char *method,
		struct blob_attr *msg) {

	logDebug << APP_NAME << ": ubus call info::systembus_loaded received" << std::endl;

	blob_buf_init(&b, 0);
	blobmsg_add_u8(&b, "systembus_loaded", true);
	ubus_send_reply(ctx, req, b.head);
	return 0;
}

int systembus_info_release_name(struct ubus_context *ctx, struct ubus_object *obj,
		struct ubus_request_data *req, const char *method,
		struct blob_attr *msg) {

	logDebug << APP_NAME << ": ubus call info::release_name received" << std::endl;
	std::lock_guard<std::mutex> guard(info_mutex);

	blob_buf_init(&b, 0);
	blobmsg_add_string(&b, "release_name", info_data -> release_name.c_str());
	ubus_send_reply(ctx, req, b.head);
	return 0;
}
