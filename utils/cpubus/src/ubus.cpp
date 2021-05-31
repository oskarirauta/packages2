#include <iostream>
#include <thread>
#include <algorithm>

#include "cpu.h"
#include "shared.h"
#include "ubus_cmds.h"
#include "ubus_node.h"
#include "ubus.h"

struct ubus_context *ctx;
struct blob_buf b;

static const struct ubus_method cpu_methods[] = {
	{ .name = "get", .handler = cpubus_get, .policy = get_policy, .n_policy = get_policy_size()},
	{ .name = "count", .handler = cpubus_count },
	{ .name = "load", .handler = cpubus_load },
	{ .name = "list", .handler = cpubus_list },
	{ .name = "all", .handler = cpubus_all },
	{ .name = "die", .handler = cpubus_exit },
};

static struct ubus_object_type cpu_object_type = {
	.name = UBUS_PATH,
	.id = 0,
	.methods = cpu_methods,
	.n_methods = ARRAY_SIZE(cpu_methods),
};

static struct ubus_object cpu_object = {
	.name = UBUS_PATH,
	.type = &cpu_object_type,
	.methods = cpu_methods,
	.n_methods = ARRAY_SIZE(cpu_methods),
};

static struct ubus_method cpu_node_methods[] = {
	{ .name = "get", .handler = cpu_node_get, .policy = node_policy, .n_policy = node_policy_size()},
	{ .name = "id", .handler = cpu_node_id },
	{ .name = "name", .handler = cpu_node_name },
	{ .name = "vendor", .handler = cpu_node_vendor },
	{ .name = "model", .handler = cpu_node_model },
	{ .name = "mhz", .handler = cpu_node_mhz },
	{ .name = "load", .handler = cpu_node_load },
	{ .name = "info", .handler = cpu_node_info },
};

static struct ubus_object_type cpu_node_type = {
	.name = "node",
	.id = 0,
	.methods = cpu_node_methods,
	.n_methods = ARRAY_SIZE(cpu_node_methods),
};

int server_main(void) {

	logDebug << APP_NAME << ": creating ubus objects" << std::endl;

	std::lock_guard<std::mutex> guard(cpu_mutex);

	int ret = ubus_add_object(ctx, &cpu_object);

	if ( ret ) {
		logError << APP_NAME << ": failed to add ubus object" << std::endl;
		logError << "error: " << ubus_strerror(ret) << std::endl;
		return ret;
	}

	std::for_each(std::begin(cpu_data -> nodes), std::end(cpu_data -> nodes), [](cpu_node& node) {

		char *name = NULL;
		struct ubus_object *obj = &node.ubus;

		asprintf(&name, "%s.%s", UBUS_PATH, node.name.c_str());

		if (name) {

			obj -> name = name;
			obj -> type = &cpu_node_type;
			obj -> methods = cpu_node_methods;
			obj -> n_methods = ARRAY_SIZE(cpu_node_methods);

			int ret = ubus_add_object(ctx, &node.ubus);

			if ( ret ) {
				logError << APP_NAME << ": failed to add ubus object for '" << node.name << "'" << std::endl;
				logError << "error: " << ubus_strerror(ret) << std::endl;
				free(name);
				obj -> name = NULL;
			}
		}
	});

	return 0;
}

void server_loop(void) {
	ubus_mutex.lock();
	ubus_state = UBUS_RUNNING;
	ubus_mutex.unlock();
	uloop_run();
	ubus_mutex.lock();
	ubus_state = UBUS_STOPPED;
	ubus_mutex.unlock();
	logVerbose << APP_NAME << ": ubus stopped" << std::endl;
	uloop_done();
	logExtra << APP_NAME << ": exiting main loop" << std::endl;
	sig_mutex.lock();
	sig_exit = 1;
	sig_mutex.unlock();
}
