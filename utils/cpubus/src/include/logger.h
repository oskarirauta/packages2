#ifndef _LOGGER_H_
#define _LOGGER_H_ 1

#include <iostream>

enum LOG {
	NONE = 0,
	ERROR = 1,
	INFO = 2,
	VERBOSE = 3,
	VVERBOSE = 4,
	DEBUG = 5
};

extern LOG loglevel;

class Logger {

	public:

		using endl_type = std::ostream& (std::ostream&);

		LOG level;

		Logger(LOG level) {
			this -> level = level;
		}
    
		template<typename T>
		Logger& operator << (const T input) {
			if ( loglevel >= this -> level )
				std::cout << input;
			return *this;
		};

		Logger& operator<<(endl_type endl) {
			if ( loglevel >= this -> level )
				std::cout << endl;
			return *this;
		};

};

#endif
