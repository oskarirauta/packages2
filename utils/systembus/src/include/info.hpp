#ifndef _INFO_HPP_
#define _INFO_HPP_ 1

#include <string>

extern "C" {
#include <uci.h>
}

class info_t {
	public:
		std::string release_name;

		info_t(void);
		void update(void);

	private:
		std::string parse_release_name(struct uci_package *p);
		std::string get_release_name(void);
};

extern info_t *info_data;

#endif
