#include <string>

#include "common.hpp"
#include "cpu.hpp"
#include "shared.hpp"
#include "security.hpp"
#include "ubus_security.hpp"

int systembus_sec_list(struct ubus_context *ctx, struct ubus_object *obj,
		struct ubus_request_data *req, const char *method,
		struct blob_attr *msg) {

	logDebug << APP_NAME << ": ubus call security::list received" << std::endl;
	std::lock_guard<std::mutex> guard(security_mutex);

	blob_buf_init(&b, 0);
	void *cookie = blobmsg_open_table(&b, "selinux");
	blobmsg_add_u8(&b, "enabled", security_data -> selinux_enabled);
	blobmsg_add_string(&b, "mode", security_data -> selinux_mode.c_str());
	blobmsg_close_table(&b, cookie);
	cookie = blobmsg_open_table(&b, "apparmor");
	blobmsg_add_u8(&b, "enabled", security_data -> apparmor_enabled);
	blobmsg_add_u16(&b, "profiles", security_data -> apparmor_profiles);
	blobmsg_close_table(&b, cookie);
	cookie = blobmsg_open_table(&b, "seccomp");
	blobmsg_add_u8(&b, "enabled", security_data -> seccomp_enabled);
	blobmsg_add_string(&b, "actions", security_data -> seccomp_actions_str.c_str());
	blobmsg_close_table(&b, cookie);
	ubus_send_reply(ctx, req, b.head);
	return 0;
}

int systembus_sec_selinux(struct ubus_context *ctx, struct ubus_object *obj,
		struct ubus_request_data *req, const char *method,
		struct blob_attr *msg) {

	logDebug << APP_NAME << ": ubus call security::selinux received" << std::endl;
	std::lock_guard<std::mutex> guard(security_mutex);

	blob_buf_init(&b, 0);
	blobmsg_add_u8(&b, "enabled", security_data -> selinux_enabled);
	ubus_send_reply(ctx, req, b.head);
	return 0;
}

int systembus_sec_selinux_mode(struct ubus_context *ctx, struct ubus_object *obj,
		struct ubus_request_data *req, const char *method,
		struct blob_attr *msg) {

	logDebug << APP_NAME << ": ubus call security::selinux -> mode received" << std::endl;
	std::lock_guard<std::mutex> guard(security_mutex);

	blob_buf_init(&b, 0);
	blobmsg_add_string(&b, "mode", security_data -> selinux_mode.c_str());
	ubus_send_reply(ctx, req, b.head);
	return 0;
}

int systembus_sec_apparmor(struct ubus_context *ctx, struct ubus_object *obj,
		struct ubus_request_data *req, const char *method,
		struct blob_attr *msg) {

	logDebug << APP_NAME << ": ubus call security::apparmor received" << std::endl;
	std::lock_guard<std::mutex> guard(security_mutex);

	blob_buf_init(&b, 0);
	blobmsg_add_u8(&b, "enabled", security_data -> apparmor_enabled);
	ubus_send_reply(ctx, req, b.head);
	return 0;
}

int systembus_sec_apparmor_profiles(struct ubus_context *ctx, struct ubus_object *obj,
		struct ubus_request_data *req, const char *method,
		struct blob_attr *msg) {

	logDebug << APP_NAME << ": ubus call security::apparmor -> profiles received" << std::endl;
	std::lock_guard<std::mutex> guard(security_mutex);

	blob_buf_init(&b, 0);
	blobmsg_add_u16(&b, "profiles", security_data -> apparmor_profiles);
	ubus_send_reply(ctx, req, b.head);
	return 0;
}

int systembus_sec_seccomp(struct ubus_context *ctx, struct ubus_object *obj,
		struct ubus_request_data *req, const char *method,
		struct blob_attr *msg) {

	logDebug << APP_NAME << ": ubus call security::seccomp received" << std::endl;
	std::lock_guard<std::mutex> guard(security_mutex);

	blob_buf_init(&b, 0);
	blobmsg_add_u8(&b, "enabled", security_data -> seccomp_enabled);
	ubus_send_reply(ctx, req, b.head);
	return 0;
}

int systembus_sec_seccomp_actions(struct ubus_context *ctx, struct ubus_object *obj,
		struct ubus_request_data *req, const char *method,
		struct blob_attr *msg) {

	logDebug << APP_NAME << ": ubus call security::seccomp -> actions received" << std::endl;
	std::lock_guard<std::mutex> guard(security_mutex);

        blob_buf_init(&b, 0);
        blobmsg_add_string(&b, "actions", security_data -> seccomp_actions_str.c_str());
        ubus_send_reply(ctx, req, b.head);
        return 0;
}
