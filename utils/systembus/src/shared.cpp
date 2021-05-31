#include <iostream>
#include <csignal>
#include <string>
#include <algorithm>

#include "ubus.hpp"
#include "shared.hpp"

LOG loglevel = LOG::INFO;

Logger logError(LOG::ERROR);
Logger logInfo(LOG::INFO);
Logger logVerbose(LOG::VERBOSE);
Logger logExtra(LOG::VVERBOSE);
Logger logDebug(LOG::DEBUG);

std::mutex cpu_mutex;
std::mutex security_mutex;
std::mutex info_mutex;
std::mutex sig_mutex;

bool mainloop_running = false;
bool sig_exit = false;
int delay = DEFAULT_DELAY;
std::string ubus_socket = "";
cpu_t *cpu_data;
security_t *security_data;
info_t *info_data;

static void die_handler(int signum) {

	logInfo << APP_NAME << ": received TERM signal" << std::endl;

	if ( !uloop_cancelled ) {
		logVerbose << APP_NAME << ": exiting ubus service" << std::endl;
		uloop_end();
	}
}


void register_signal_handlers(void) {

	struct sigaction exit_action, ignore_action;

	exit_action.sa_handler = die_handler;
	sigemptyset (&exit_action.sa_mask);
	exit_action.sa_flags = 0;

	ignore_action.sa_handler = SIG_IGN;
	sigemptyset (&ignore_action.sa_mask);
	ignore_action.sa_flags = 0;

	sigaction(SIGTERM, &exit_action, NULL);

	sigaction(SIGALRM, &ignore_action, NULL);
	sigaction(SIGHUP, &ignore_action, NULL);
	sigaction(SIGINT, &exit_action, NULL);
	sigaction(SIGPIPE, &ignore_action, NULL);
	sigaction(SIGQUIT, &ignore_action, NULL);
	sigaction(SIGUSR1, &ignore_action, NULL);
	sigaction(SIGUSR2, &ignore_action, NULL);

}

void cpu_info(bool minimal) {

	logVerbose << "cpu count: " << cpu_data -> count << std::endl;
	std::cout << "total load: " << static_cast<int>(cpu_data -> load) << "%" << std::endl;

	std::for_each(std::begin(cpu_data -> nodes), std::end(cpu_data -> nodes), [minimal](cpu_node& node) {
		if ( loglevel < LOG::VERBOSE || minimal )
			std::cout << node.name << ": " << static_cast<int>(node.load) << "%" << std::endl;
		else {
			std::cout << "\nid: " << node.id;
			std::cout << "\nname: " << node.name;
			std::cout << "\nvendor: " << node.vendor;
			std::cout << "\nmodel: " << node.model;
			std::cout << "\nmhz: " << node.mhz;
			std::cout << "\nload: " << static_cast<int>(node.load) << "%" << std::endl;
		}
	});
}

void version_info(void) {
	std::cout << APP_NAME << " version " << APP_VERSION;
	std::cout << "\n" << "Written by Oskari Rauta" << std::endl;
}

void usage(char* cmd) {
	std::cout << "\nusage: " << cmd << " [parameters]\n\n";
	std::cout << "   -h, --h             show this help\n";
	std::cout << "   --version           show version\n";
	std::cout << "   -s, --socket path   connect to specified ubus socket\n";
	std::cout << "   --interval ms       set interval for cpu load refreshing(default: " << DEFAULT_DELAY << "ms)\n";
	std::cout << "   -q, --quiet         silence output\n";
	std::cout << "   --only-errors       output only errors\n";
	std::cout << "   -v, --verbose       verbose output\n";
	std::cout << "   -vv                 more verbose output\n";
	std::cout << "   --debug             maximum verbose output\n";
	std::cout << "   --cpu               output cpu statistics and exit\n";
	std::cout << std::endl;
}
