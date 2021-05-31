#include <thread>

#include "cpu.h"
#include "shared.h"
#include "ubus.h"
#include "loop.h"

void main_loop(void) {

	int state = sig_exit;

	while ( !state ) {

		std::this_thread::sleep_for(std::chrono::milliseconds(delay - SIG_DELAY));

		cpu_mutex.lock();
		cpu_data -> update();
		cpu_mutex.unlock();

		std::this_thread::sleep_for(std::chrono::milliseconds(SIG_DELAY));

		sig_mutex.lock();
		state = sig_exit;
		sig_mutex.unlock();
	}
}
