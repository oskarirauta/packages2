#include <iostream>
#include <string>
#include <thread>

#include "cpu.hpp"
#include "shared.hpp"
#include "ubus.hpp"
#include "cmdline.hpp"
#include "loop.hpp"

void freedata(void) {

	free(info_data);
	free(security_data);
	free(cpu_data);
}

int main(int argc, char **argv) {

	cpu_data = new cpu_t();
	security_data = new security_t();
	info_data = new info_t();

	parse_cmdline(argc, argv);

	uloop_init();
	ctx = ubus_connect(ubus_socket == "" ? NULL : ubus_socket.c_str());

	if ( !ctx ) {
		logError << "cpubus: failed to connect to ubus socket " << ubus_socket << std::endl;
		freedata();
		return -1;
	}

	ubus_add_uloop(ctx);

	if ( loglevel >= LOG::VERBOSE )
		std::cout << APP_NAME << " version " << APP_VERSION << std::endl;

	if ( int ret = ubus_create(); ret != 0 ) {
		ubus_free(ctx);
		freedata();
		return(ret);
	}

	logDebug << APP_NAME << ": registering signal handlers" << std::endl;
	register_signal_handlers();

	logVerbose << APP_NAME << ": starting main loop" << std::endl;
	std::thread loop_thread(main_loop);

	logInfo << APP_NAME << ": service started" << std::endl;
	logVerbose << APP_NAME << ": starting ubus service" << std::endl;
	uloop_run();

	uloop_done();
	ubus_free(ctx);

	logVerbose << APP_NAME << ": ubus service has stopped" << std::endl;
	logExtra << APP_NAME << ": exiting main loop" << std::endl;
	
	sig_mutex.lock();
	sig_exit = true;
	sig_mutex.unlock();

	while ( mainloop_running ) {
		std::this_thread::sleep_for(std::chrono::milliseconds(SIG_DELAY));
	}

	logExtra << APP_NAME << ": main loop stopped" << std::endl;

	loop_thread.join();
	freedata();

	logVerbose << APP_NAME << ": ended service" << std::endl;

	return 0;
}
