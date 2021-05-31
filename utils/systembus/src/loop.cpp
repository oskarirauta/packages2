#include <thread>

#include "cpu.hpp"
#include "security.hpp"
#include "shared.hpp"
#include "ubus.hpp"
#include "loop.hpp"

void main_loop(void) {

	bool state = sig_exit;
	sig_mutex.lock();
	mainloop_running = true;
	sig_mutex.unlock();
	int security_c = 0,
		info_c = 0;

	while ( !state ) {

		std::this_thread::sleep_for(std::chrono::milliseconds(delay - SIG_DELAY));

		cpu_mutex.lock();
		cpu_data -> update();
		cpu_mutex.unlock();

		security_c++;
		if ( security_c > 9 ) {

			security_mutex.lock();
			security_data -> update();
			security_mutex.unlock();
			security_c = 0;
		}

		info_c++;
		if ( info_c > 5 ) {

			info_mutex.lock();
			info_data -> update();
			info_mutex.unlock();
			info_c = 0;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(SIG_DELAY));

		sig_mutex.lock();
		state = sig_exit;
		sig_mutex.unlock();
	}

	sig_mutex.lock();
	mainloop_running = false;
	sig_mutex.unlock();

}
