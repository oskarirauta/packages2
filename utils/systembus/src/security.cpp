#include <algorithm>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>

#ifdef SELINUX_ENABLED
#include <selinux/selinux.h>
#endif

#include "common.hpp"
#include "security.hpp"
#include "cpu.hpp"
#include "shared.hpp"

security_t::security_t(void) {

	this -> update();
}

int security_t::get_selinux_status(void) {
#ifdef SELINUX_ENABLED
	int rs = is_selinux_enabled();
	if ( rc < 0 )
		return -1;
	else if ( rc == 1 ) {
		rc = security_getenforce();
		if ( rc < 0 ) return -1;
		return rc + 1;
	} else return 0;
#else
	return -1;
#endif
}

int security_t::get_apparmor_profiles(void) {

	std::ifstream fd("/sys/kernel/security/apparmor/profiles");

	if ( !fd.good())
		return 0;

	int ret = 0;
	std::string line;

	while ( std::getline(fd, line))
		ret++;

	fd.close();
	return ret;
}

bool security_t::get_seccomp_actions(std::vector<std::string> &actions) {

	std::ifstream fd("/proc/sys/kernel/seccomp/actions_avail");
	if ( !fd.good())
		return false;

	std::vector<std::string> new_actions;
	std::string line, act;
	bool ret = false;

	while ( std::getline(fd, line)) {
		std::istringstream ss(line);
		while ( ss >> act ) {
			if ( !act.empty()) {
				ret = true;
				new_actions.push_back(act);
			}
		}
	}

	fd.close();
	actions = new_actions;
	return ret;
}

void security_t::update(void) {

	this -> selinux_state = this -> get_selinux_status();
	this -> selinux_enabled = this -> selinux_state > 0 ? true : false;

	switch ( this -> selinux_state ) {
		case -1: this -> selinux_mode = "not supported"; break;
		case 0: this -> selinux_mode = "disabled"; break;
		case 1: this -> selinux_mode = "permissive"; break;
		case 2: this -> selinux_mode = "enforcing"; break;
		default: this -> selinux_mode = "unknown"; break;
	}

	this -> apparmor_profiles = this -> get_apparmor_profiles();
	this -> apparmor_enabled = this -> apparmor_profiles > 1 ? true : false;

	this -> seccomp_enabled = this -> get_seccomp_actions(this -> seccomp_actions);
	this -> seccomp_actions_str = this -> seccomp_enabled ? common::join_vector(this -> seccomp_actions) : "";
}
