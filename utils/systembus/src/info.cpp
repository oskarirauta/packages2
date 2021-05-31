#include "info.hpp"
#include "common.hpp"

info_t::info_t(void) {

	this -> update();
}

void info_t::update(void) {

	this -> release_name = this -> get_release_name();
}

std::string info_t::parse_release_name(struct uci_package *p) {

	struct uci_element *e;

	uci_foreach_element(&p -> sections, e) {

		struct uci_section *s = uci_to_section(e);

		if ( common::c_tostr(s -> type) != "system" )
			continue;

		struct uci_element *n;
		uci_foreach_element(&s -> options, n) {

			struct uci_option *o = uci_to_option(n);

			if ( common::c_tostr(n -> name) != "openwrt_release" ||
				o -> type != UCI_TYPE_STRING ) continue;

			return common::c_tostr(o -> v.string);
		}
	}

	return "";
}

std::string info_t::get_release_name(void) {

	struct uci_context *uci_ctx = uci_alloc_context();

	if ( !uci_ctx )
		return "";

	struct uci_package *p = NULL;
	uci_load(uci_ctx, "system", &p);
	
	if ( !p )
		return "";

	std::string ret = parse_release_name(p);

	uci_free_context(uci_ctx);
	return ret;
}
