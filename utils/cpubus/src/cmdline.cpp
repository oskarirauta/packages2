#include <iostream>
#include <vector>
#include <string>
#include <thread>

#include "cpu.h"
#include "shared.h"
#include "cmdline.h"

void parse_cmdline(int argc, char **argv) {

	std::vector<std::string> args(argv + 1, argv + argc);
	int logging_altered = 0;
	int list_only = 0;

	for (auto i = args.begin(); i != args.end(); i++) {
		if ( *i == "--help" || *i == "--h" || *i == "-h" || *i == "-?" ) {
			version_info();
			usage(argv[0]);
			exit(0);
		} else if (( *i == "-v" || *i == "--verbose" ) && loglevel < LOG::VERBOSE ) {
			if ( loglevel == LOG::NONE ) {
				std::cout << "error: quiet and verbose logging cannot be used at same time." << std::endl;
				exit(-1);
			}
			loglevel = LOG::VERBOSE;
			logging_altered = 1;
		} else if (( *i == "-vv" || *i == "--vv" ) && loglevel < LOG::VVERBOSE ) {
			if ( loglevel == LOG::NONE ) {
				std::cout << "error: quiet and (extra) verbose logging cannot be used at same time." << std::endl;
				exit(-1);
			}
			loglevel = LOG::VVERBOSE;
			logging_altered = 1;
		} else if (( *i == "-debug" || *i == "--debug" ) && loglevel < LOG::DEBUG ) {
			if ( loglevel == LOG::NONE ) {
				std::cout << "error: quiet and debug logging cannot be used at same time." << std::endl;
				exit(-1);
			}
			loglevel = LOG::DEBUG;
			logging_altered = 1;
		} else if ( *i == "--only-errors" && !logging_altered ) {
			if ( loglevel == LOG::NONE ) {
				std::cout << "error: quiet and error-only logging cannot be used at same time." << std::endl;
				exit(-1);
			}
			loglevel = LOG::ERROR;
			logging_altered = 1;
		} else if ( *i == "-q" || *i == "--q" || *i == "-quiet" || *i == "--quiet" ) {
			if ( logging_altered ) {
				std::cout << "error: quiet and other logging levels cannot be used at same time." << std::endl;
				exit(-1);
			}
			loglevel = LOG::NONE;
		} else if ( *i == "-s" || *i == "--s" || *i == "--sock" || *i == "--socket" || *i == "-socket" ) {
			std::string this_arg = *i;
			if ( std::next(i) != args.end())
				ubus_socket = *++i;
			else {
				std::cout << "error: socket's path not given for " << this_arg << " option." << std::endl;
				exit(-1);
			}
		} else if ( *i == "-t" || *i == "--t" || *i == "-i" || *i == "--i" || *i == "--interval" ) {
			std::string this_arg = *i;
			if ( std::next(i) != args.end()) {
				std::string str = *++i;
				for ( char c : str ) if ( !isdigit(c)) {
					std::cout << "error: " << this_arg << " requires numeric parameter" << std::endl;
					exit(-1);
				}
				int value = 0;
				try {
					value = std::stoi(str, nullptr);
				} catch (...) {
					std::cout << "error: " << this_arg << " requires numeric parameter" << std::endl;
					exit(-1);
				}
				if ( value < 500 || value > 1800 ) {
					std::cout << "error: " << this_arg << " requires value between 500 and 1800 (seconds)." << std::endl;
					exit(-1);
				}
				delay = value;
			} else {
				std::cout << "error: " << *i << " requires value between 500 and 1800 (seconds)." << std::endl;
				exit(-1);
			}
		} else if ( *i == "-l" || *i == "--l" || *i == "-list" || *i == "--list" ) {
			list_only = 1;
		} else if ( *i == "-version" || *i == "--version" ) {
			version_info();
			exit(0);
		} else if ( *i != "" ) {
			std::cout << "Unknown argument: " << *i << std::endl;
			usage(argv[0]);
			exit(-1);
		}
	}

	if ( list_only ) {
		cpu_info();
		free(cpu_data);
		exit(0);
	}
}
