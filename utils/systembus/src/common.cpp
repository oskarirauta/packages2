#include <iostream>
#include <algorithm>
#include <sstream>

#include "common.hpp"

namespace common {

	bool has_prefix(const std::string str, const std::string prefix) {

		return ((prefix.length() <= str.length()) &&
			std::equal(prefix.begin(), prefix.end(), str.begin()));
	}

	std::string str_first(const std::string str, char delim) {

		std::stringstream ss(str);
		std::string ret;
		while ( std::getline(ss, ret, delim)) break;
		return ret;
	}

	std::string trim(std::string &str, const std::string trimchars) {

		if ( str.empty() || trimchars.empty()) return str;

		for ( const char &ch : trimchars )
			str.erase(std::remove(str.begin(), str.end(), ch), str.end());

		return str;
	}

	std::string trim(const std::string &str, const std::string trimchars) {

		std::string s = str;
		return common::trim(s, trimchars);
	}

	std::vector<std::string> split(const std::string &str, char delim, const std::string trimchars) {

		std::vector<std::string> elems;
		if ( str.empty() ) return elems;

		std::stringstream ss(str);
		std::string item;

		while( std::getline(ss, item, delim)) {
			if ( !trimchars.empty())
				for ( const char &ch : trimchars )
					item.erase(std::remove(item.begin(), item.end(), ch), item.end());
			elems.push_back(item);
		}

		elems.erase(std::remove_if(elems.begin(), elems.end(),
			[](const std::string& s) { return s.empty(); }),
			elems.end());

		return elems;

	}

	std::string to_lower(std::string &str) {
		for ( auto& ch : str ) ch = tolower(ch);
		return str;
	}

	std::string to_lower(const std::string &str) {
		std::string s = str;
		return common::to_lower(str);
	}

	std::string join_vector(const std::vector<std::string> v, const std::string delimeter) {

		std::string result;

                if (auto i = v.begin(), e = v.end(); i != e) {
                        result += *i++;
                        for (; i != e; ++i) result.append(delimeter).append(*i);
                }

		return result;
	}

	std::string c_tostr(const char *str) {

		std::string str2(str);
		return str2;
	}

}
