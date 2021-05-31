#include <iostream>
#include <csignal>
#include <string>
#include <algorithm>

#include "shared.h"
#include "ubus.h"

LOG loglevel = LOG::INFO;

Logger logError(LOG::ERROR);
Logger logInfo(LOG::INFO);
Logger logVerbose(LOG::VERBOSE);
Logger logExtra(LOG::VVERBOSE);
Logger logDebug(LOG::DEBUG);

std::mutex cpu_mutex;
std::mutex ubus_mutex;
std::mutex sig_mutex;

int ubus_state = UBUS_STOPPED;
int sig_exit = 0;
int delay = DEFAULT_DELAY;
std::string ubus_socket = "";
cpu_t *cpu_data;

static void die_handler(int signum) {

	int state;
	logInfo << APP_NAME << ": received TERM signal" << std::endl;

	ubus_mutex.lock();
	state = ubus_state;
	ubus_mutex.unlock();

	if ( state == UBUS_RUNNING ) {
		logVerbose << APP_NAME << ": calling exit ubus service" << std::endl;

		pid_t pid = fork();

		if ( pid == -1 ) {
			logError << APP_NAME << ": failed to fork, ubus service cannot be stopped" << std::endl;
		} else if ( pid == 0 ) {
			char *arg_list[] = { UBUS_BIN, "call", UBUS_PATH, "die", nullptr };
			execve(UBUS_BIN, arg_list, nullptr);
			logError << APP_NAME << ": call to ubus service failed" << std::endl;
			exit(EXIT_FAILURE);
		}
	} else if ( state == UBUS_STOPPED ) {
		logVerbose << APP_NAME << ": stopping main service" << std::endl;
		sig_mutex.lock();
		sig_exit = 1;
		sig_mutex.unlock();
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
			std::cout << node.name << ": " << static_cast<int>(node.load) << std::endl;
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
	std::cout << "   -i, --interval ms   set interval for load refreshing(default: " << DEFAULT_DELAY << "ms)\n";
	std::cout << "   -q, --quiet         silence output\n";
	std::cout << "   --only-errors       output only errors\n";
	std::cout << "   -v, --verbose       verbose output\n";
	std::cout << "   -vv                 more verbose output\n";
	std::cout << "   --debug             maximum verbose output\n";
	std::cout << "   -l, --list          output cpu statistics and exit\n";
	std::cout << std::endl;
}
