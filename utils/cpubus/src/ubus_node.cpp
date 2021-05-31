#include <algorithm>

#include "common.h"
#include "cpu.h"
#include "shared.h"
#include "ubus_node.h"

const struct blobmsg_policy node_policy[] = {
	[NODE_FIELDS] = { .name = "fields", .type = BLOBMSG_TYPE_STRING },
};

int node_policy_size(void) {
	return ARRAY_SIZE(node_policy);
}

int cpu_node_get(struct ubus_context *ctx, struct ubus_object *obj,
		struct ubus_request_data *req, const char *method,
		struct blob_attr *msg) {

	struct blob_attr *tb[__NODE_ARGS_MAX];
	blobmsg_parse(node_policy, ARRAY_SIZE(node_policy), tb, blob_data(msg), blob_len(msg));
	bool fields[NUM_FIELDS] = { false, false, false, false, false, false, false };
	bool selective = false;

	if ( tb[NODE_FIELDS] ) {
		std::string fields_str = common::to_lower(std::string((char*)blobmsg_data(tb[NODE_FIELDS])));
		if ( common::trim(fields_str, ", \t\r\n").empty()) return UBUS_STATUS_INVALID_ARGUMENT;
		std::vector<std::string> fields_v = common::split(fields_str, ',', " \t\r\n");
		std::for_each(std::begin(fields_v), std::end(fields_v), [&fields, &selective](std::string& field) {

			switch ( common::hash(field.c_str())) {
				case common::hash("id"): fields[FIELD_ID] = true; selective = true; break;
				case common::hash("name"): fields[FIELD_NAME] = true; selective = true; break;
				case common::hash("vendor"): fields[FIELD_VENDOR] = true; selective = true; break;
				case common::hash("model"): fields[FIELD_MODEL] = true; selective = true; break;
				case common::hash("mhz"): fields[FIELD_MHZ] = true; selective = true; break;
				case common::hash("load"): fields[FIELD_LOAD] = true; selective = true; break;
			}
		});

		if ( !selective ) return UBUS_STATUS_INVALID_ARGUMENT;
	} else return UBUS_STATUS_INVALID_ARGUMENT;

	std::lock_guard<std::mutex> guard(cpu_mutex);

	struct cpu_node *node;
	node = container_of(obj, cpu_node, ubus);
	blob_buf_init(&b, 0);

	if ( fields[FIELD_ID]) blobmsg_add_u16(&b, "id", node -> id);
	if ( fields[FIELD_NAME]) blobmsg_add_string(&b, "name", node -> name.c_str());
	if ( fields[FIELD_VENDOR]) blobmsg_add_string(&b, "vendor", node -> vendor.c_str());
	if ( fields[FIELD_MODEL]) blobmsg_add_string(&b, "model", node -> model.c_str());
	if ( fields[FIELD_MHZ]) blobmsg_add_u16(&b, "mhz", node -> mhz);
	if ( fields[FIELD_LOAD]) blobmsg_add_u16(&b, "load", static_cast<int>(node -> load));

	ubus_send_reply(ctx, req, b.head);
	return 0;
}

int cpu_node_id(struct ubus_context *ctx, struct ubus_object *obj,
		struct ubus_request_data *req, const char *method,
		struct blob_attr *msg) {

	std::lock_guard<std::mutex> guard(cpu_mutex);
	struct cpu_node *node;
	node = container_of(obj, cpu_node, ubus);
	blob_buf_init(&b, 0);
	blobmsg_add_u16(&b, "id", node -> id);
	ubus_send_reply(ctx, req, b.head);
	return 0;
}

int cpu_node_name(struct ubus_context *ctx, struct ubus_object *obj,
                struct ubus_request_data *req, const char *method,
                struct blob_attr *msg) {

	std::lock_guard<std::mutex> guard(cpu_mutex);
	struct cpu_node *node;
	node = container_of(obj, cpu_node, ubus);
	blob_buf_init(&b, 0);
	blobmsg_add_string(&b, "name", node -> name.c_str());
	ubus_send_reply(ctx, req, b.head);
	return 0;
}

int cpu_node_vendor(struct ubus_context *ctx, struct ubus_object *obj,
                struct ubus_request_data *req, const char *method,
                struct blob_attr *msg) {

	std::lock_guard<std::mutex> guard(cpu_mutex);
        struct cpu_node *node;
        node = container_of(obj, cpu_node, ubus);
        blob_buf_init(&b, 0);
        blobmsg_add_string(&b, "vendor", node -> vendor.c_str());
        ubus_send_reply(ctx, req, b.head);
        return 0;
}

int cpu_node_model(struct ubus_context *ctx, struct ubus_object *obj,
                struct ubus_request_data *req, const char *method,
                struct blob_attr *msg) {

	std::lock_guard<std::mutex> guard(cpu_mutex);
        struct cpu_node *node;
        node = container_of(obj, cpu_node, ubus);
        blob_buf_init(&b, 0);
        blobmsg_add_string(&b, "name", node -> model.c_str());
        ubus_send_reply(ctx, req, b.head);
        return 0;
}

int cpu_node_mhz(struct ubus_context *ctx, struct ubus_object *obj,
                struct ubus_request_data *req, const char *method,
                struct blob_attr *msg) {

	std::lock_guard<std::mutex> guard(cpu_mutex);
        struct cpu_node *node;
        node = container_of(obj, cpu_node, ubus);
        blob_buf_init(&b, 0);
        blobmsg_add_u16(&b, "mhz", node -> mhz);
        ubus_send_reply(ctx, req, b.head);
        return 0;
}

int cpu_node_load(struct ubus_context *ctx, struct ubus_object *obj,
                struct ubus_request_data *req, const char *method,
                struct blob_attr *msg) {

	std::lock_guard<std::mutex> guard(cpu_mutex);
        struct cpu_node *node;
        node = container_of(obj, cpu_node, ubus);
        blob_buf_init(&b, 0);
	blobmsg_add_u16(&b, "load", static_cast<int>(node -> load));
        ubus_send_reply(ctx, req, b.head);
        return 0;
}

int cpu_node_info(struct ubus_context *ctx, struct ubus_object *obj,
                struct ubus_request_data *req, const char *method,
                struct blob_attr *msg) {

	std::lock_guard<std::mutex> guard(cpu_mutex);
        struct cpu_node *node;
        node = container_of(obj, cpu_node, ubus);
        blob_buf_init(&b, 0);
	blobmsg_add_u16(&b, "id", node -> id);
        blobmsg_add_string(&b, "name", node -> name.c_str());
	blobmsg_add_string(&b, "vendor", node -> vendor.c_str());
	blobmsg_add_string(&b, "model", node -> model.c_str());
	blobmsg_add_u16(&b, "mhz", node -> mhz);
	blobmsg_add_u16(&b, "load", static_cast<int>(node -> load));
        ubus_send_reply(ctx, req, b.head);
        return 0;
}
