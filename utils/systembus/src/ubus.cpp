#include <iostream>
#include <thread>
#include <algorithm>

#include "cpu.hpp"
#include "shared.hpp"
#include "ubus_cpu.hpp"
#include "ubus_security.hpp"
#include "ubus_info.hpp"
#include "ubus.hpp"

struct ubus_context *ctx;
struct blob_buf b;

static const struct ubus_method cpu_methods[] = {
	{ .name = "get", .handler = systembus_cpu_get, .policy = cpu_policy, .n_policy = cpu_policy_size()},
	{ .name = "count", .handler = systembus_cpu_count },
	{ .name = "load", .handler = systembus_cpu_load },
	{ .name = "all", .handler = systembus_cpu_all },
	{ .name = "list", .handler = systembus_cpu_list },
};

static struct ubus_object_type cpu_object_type = {
	.name = "system.cpu",
	.id = 0,
	.methods = cpu_methods,
	.n_methods = ARRAY_SIZE(cpu_methods),
};

static struct ubus_object cpu_object = {
	.name = "system.cpu",
	.type = &cpu_object_type,
	.methods = cpu_methods,
	.n_methods = ARRAY_SIZE(cpu_methods),
};

static const struct ubus_method security_methods[] = {
	{ .name = "list", .handler = systembus_sec_list },
	{ .name = "selinux", .handler = systembus_sec_selinux },
	{ .name = "selinux-mode", .handler = systembus_sec_selinux_mode },
	{ .name = "apparmor", .handler = systembus_sec_apparmor },
	{ .name = "apparmor-profiles", .handler = systembus_sec_apparmor_profiles },
	{ .name = "seccomp", .handler = systembus_sec_seccomp },
	{ .name = "seccomp-actions", .handler = systembus_sec_seccomp_actions },
};

static struct ubus_object_type security_object_type = {
	.name = "system.security",
	.id = 0,
	.methods = security_methods,
	.n_methods = ARRAY_SIZE(security_methods),
};

static struct ubus_object security_object = {
	.name = "system.security",
	.type = &security_object_type,
	.methods = security_methods,
	.n_methods = ARRAY_SIZE(security_methods),
};

static const struct ubus_method info_methods[] = {
	{ .name = "list", .handler = systembus_info_list },
	{ .name = "systembus_loaded", .handler = systembus_info_is_enabled },
        { .name = "release_name", .handler = systembus_info_release_name },
};

static struct ubus_object_type info_object_type = {
        .name = "system.info",
        .id = 0,
        .methods = info_methods,
        .n_methods = ARRAY_SIZE(info_methods),
};

static struct ubus_object info_object = {
        .name = "system.info",
        .type = &info_object_type,
        .methods = info_methods,
        .n_methods = ARRAY_SIZE(info_methods),
};

int ubus_create(void) {

	logDebug << APP_NAME << ": creating ubus objects" << std::endl;

	if ( int ret = ubus_add_object(ctx, &cpu_object); ret != 0 ) {
		logError << APP_NAME << ": failed to add system::cpu object to ubus" << std::endl;
		logError << "error: " << ubus_strerror(ret) << std::endl;
		return ret;
	}

	if ( int ret = ubus_add_object(ctx, &security_object); ret != 0 ) {
		logError << APP_NAME << ": failed to add system::security object to ubus" << std::endl;
		logError << "error: " << ubus_strerror(ret) << std::endl;
		return ret;
	}

	if ( int ret = ubus_add_object(ctx, &info_object); ret != 0 ) {
		logError << APP_NAME << ": failed to add system::info object to ubus" << std::endl;
		logError << "error: " << ubus_strerror(ret) << std::endl;
		return ret;
	}

	return 0;
}
