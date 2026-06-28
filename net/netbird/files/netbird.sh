#!/bin/sh
# netbird netifd protocol - DRAFT v4 for upstreaming to the netbird package.
#
# Runs the netbird daemon DIRECTLY under netifd supervision via
# proto_run_command / proto_kill_command (the pattern used by autoip and the
# experimental zerotier proto). netifd owns the daemon's lifecycle, so a
# `network` restart / ifdown+ifup tears it down and brings it back up and the VPN
# recovers on its own (the 'external' proto can't - its teardown is a no-op and it
# never touches the daemon, so the tunnel stays dead until a manual restart).
#
# netbird manages wt0's ROUTES itself - its peer/network routes live in a custom
# 'netbird' routing table with policy rules. So this proto does NOT set PROTO_ROUTE
# (letting netifd manage routes breaks netbird's policy routing). It DOES report the
# interface ADDRESSES for display (ifstatus / LuCI), flagged address-external
# (proto_init_update 3rd arg = 1) so netifd shows but does not manage or flush them.
#
# Daemon command + env come from the stock netbird init script:
#   /usr/bin/netbird service run
#   NB_STATE_DIR=/root/.config/netbird  NB_DISABLE_SSH_CONFIG=1
#   NB_DNS_STATE_FILE=/var/lib/netbird/state.json
#
# REQUIRED: disable the init-script service so it does not also run the daemon
# (two `service run` instances fight over /var/run/netbird.sock):
#     /etc/init.d/netbird disable ; /etc/init.d/netbird stop
#
# config interface 'netbird'
#     option proto      'netbird'
#     option device     'wt0'                     # tunnel device netbird creates (default wt0)
#     option searchdomain 'netbird.cloud'
#     option state_dir  '/root/.config/netbird'   # NB_STATE_DIR (default shown)
#     option delay      '25'                      # max seconds to wait for the tunnel

DAEMON=/usr/bin/netbird

[ -n "$INCLUDE_ONLY" ] || {
	. /lib/functions.sh
	. ../netifd-proto.sh
	init_proto "$@"
}

proto_netbird_init_config() {
	available=1
	no_device=1

	proto_config_add_string "device"
	proto_config_add_string "searchdomain"
	proto_config_add_string "state_dir"
	proto_config_add_int "delay"
}

proto_netbird_setup() {
	local cfg="$1"
	local device searchdomain state_dir delay

	json_get_vars device searchdomain state_dir delay
	[ -n "$device" ]    || device="wt0"
	[ -n "$state_dir" ] || state_dir="/root/.config/netbird"
	[ -n "$delay" ]     || delay=25

	# run the daemon in the foreground under netifd's supervision
	proto_run_command "$cfg" env \
		"NB_STATE_DIR=${state_dir}" \
		"NB_DISABLE_SSH_CONFIG=1" \
		"NB_DNS_STATE_FILE=/var/lib/netbird/state.json" \
		"$DAEMON" service run

	# wait for the tunnel device to come up and acquire its address
	local waited=0
	while [ "$waited" -lt "$delay" ]; do
		if [ -L "/sys/class/net/${device}" ]; then
			local a="$(ip -json address list dev "$device" 2>/dev/null)"
			[ -n "$a" -a "$a" != "[]" ] && break
		fi
		sleep 1
		waited=$(( waited + 1 ))
	done

	[ -L "/sys/class/net/${device}" ] || {
		echo "netbird: tunnel device $device did not appear within ${delay}s"
		proto_notify_error "$cfg" NO_DEVICE
		return 1
	}

	# collect the device's global addresses (for display only)
	IP4ADDRS=
	IP6ADDRS=

	local addresses="$(ip -json address list dev "$device")"
	json_init
	json_load "{\"addresses\":${addresses}}"

	if json_is_a addresses array; then
		json_select addresses
		json_select 1

		if json_is_a addr_info array; then
			json_select addr_info

			local i=1
			while json_is_a ${i} object; do
				json_select ${i}
				json_get_vars scope family local prefixlen broadcast

				if [ "${scope}" == "global" ]; then
					case "${family}" in
						inet)  append IP4ADDRS "$local/$prefixlen/$broadcast/" ;;
						inet6) append IP6ADDRS "$local/$prefixlen/$broadcast///" ;;
					esac
				fi

				json_select ..
				i=$(( i + 1 ))
			done
		fi
	fi

	# Mark up with the device flagged EXTERNAL: report the addresses for ifstatus/LuCI
	# but let netbird own the L3 (netifd will not manage or flush them). Deliberately
	# NO PROTO_ROUTE - netbird's policy routing (custom table + rules) stays untouched.
	proto_init_update "$device" 1 1

	PROTO_IPADDR="${IP4ADDRS}"
	PROTO_IP6ADDR="${IP6ADDRS}"

	[ -n "$searchdomain" ] && proto_add_dns_search "$searchdomain"

	echo "netbird: $device is up (routes managed by netbird)"

	proto_send_update "$cfg"
}

proto_netbird_teardown() {
	local cfg="$1"
	# netifd stops the supervised daemon; on a `network restart` this is followed by
	# setup again, so netbird is cleanly restarted and re-establishes its tunnel + L3.
	proto_kill_command "$cfg"
}

[ -n "$INCLUDE_ONLY" ] || {
	add_protocol netbird
}
