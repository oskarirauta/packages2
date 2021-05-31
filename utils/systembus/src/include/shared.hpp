#ifndef _SHARED_H_
#define _SHARED_H_ 1

#include <string>
#include <mutex>

#include "cpu.hpp"
#include "security.hpp"
#include "info.hpp"
#include "constants.hpp"
#include "logger.hpp"

enum {
        UBUS_STOPPED = 0,
	UBUS_RUNNING,
        __UBUS_STATE_MAX
};

extern Logger logError;
extern Logger logInfo;
extern Logger logVerbose;
extern Logger logExtra;
extern Logger logDebug;

extern std::mutex cpu_mutex;
extern std::mutex security_mutex;
extern std::mutex info_mutex;
extern std::mutex sig_mutex;

extern cpu_t *cpu_data;

extern std::string ubus_socket;

extern int delay;
extern bool mainloop_running;
extern bool sig_exit;

void register_signal_handlers(void);
void cpu_info(bool minimal = false);
void version_info(void);
void usage(char* cmd);

#endif
