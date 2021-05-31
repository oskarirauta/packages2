#ifndef _COMMON_H_
#define _COMMON_H_ 1

#include <string>
#include <vector>

namespace common {

	uint64_t constexpr mix(char m, uint64_t s) {
		return ((s<<7) + ~(s>>3)) + ~m;
	}

	uint64_t constexpr hash(const char *m) {
		return (*m) ? common::mix(*m,hash(m+1)) : 0;
	}

	bool has_prefix(const std::string str, const std::string prefix);

	std::string str_first(const std::string str, char delim = ' ');

	std::string trim(std::string &str, const std::string trimchars);

	std::string trim(const std::string &str, const std::string trimchars);

	std::vector<std::string> split(const std::string &str,
		char delim = ' ', const std::string trimchars = "");

	std::string to_lower(std::string &str);

	std::string to_lower(const std::string &str);

	std::string join_vector(const std::vector<std::string> v, const std::string delimeter = ", ");

	std::string c_tostr(const char *str);
}

#endif
