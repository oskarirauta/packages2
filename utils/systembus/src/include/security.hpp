#ifndef _SECURITY_HPP_
#define _SECURITY_HPP_ 1

#include <vector>
#include <string>

class security_t {
        public:
		bool selinux_enabled;
		int selinux_state;
		std::string selinux_mode;
		bool apparmor_enabled;
		int apparmor_profiles;
		bool seccomp_enabled;
		std::vector<std::string> seccomp_actions;
		std::string seccomp_actions_str;

		security_t(void);
		void update(void);

	private:
		int get_selinux_status(void);
		int get_apparmor_profiles(void);
		bool get_seccomp_actions(std::vector<std::string> &actions);
};

extern security_t *security_data;

#endif
