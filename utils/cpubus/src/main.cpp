#include <iostream>
#include <string>
#include <thread>

#include "cpu.h"
#include "shared.h"
#include "ubus.h"
#include "cmdline.h"
#include "loop.h"

int main(int argc, char **argv) {

	int ret;
	cpu_data = new cpu_t();
	parse_cmdline(argc, argv);

	uloop_init();
	ctx = ubus_connect(ubus_socket == "" ? NULL : ubus_socket.c_str());

	if ( !ctx ) {
		logError << "cpubus: failed to connect to ubus socket " << ubus_socket << std::endl;
		free(cpu_data);
		return -1;
	}

	ubus_add_uloop(ctx);

	if ( loglevel == LOG::DEBUG ) {
		std::cout << APP_NAME << " version " << APP_VERSION << "\n";
		cpu_info(true);
		std::cout << std::endl;
	}

	ret = server_main();
	if ( ret ) {
		ubus_free(ctx);
		free(cpu_data);
		return(ret);
	}

	logDebug << APP_NAME << ": registering signal handlers" << std::endl;
	register_signal_handlers();
	logVerbose << APP_NAME << ": starting ubus service loop" << std::endl;
	std::thread uloop_thread(server_loop);

	main_loop();

	logDebug << APP_NAME << ": mainloop stopped" << std::endl;

	uloop_thread.join();
	ubus_free(ctx);
	free(cpu_data);

	logExtra << APP_NAME << ": exit" << std::endl;

	return 0;
}
