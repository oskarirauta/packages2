#!/bin/sh

[ -n "$INCLUDE_ONLY" ] || {
	. /lib/functions.sh
	. ../netifd-proto.sh
	init_proto "$@"
}

proto_infra_init_config() {
	no_device=1
	available=1

	proto_config_add_string "device:device"
	proto_config_add_string "namespace"
	proto_config_add_string "ipaddr"
	proto_config_add_string "netmask"
	proto_config_add_string "gateway"
	proto_config_add_string "script"
	proto_config_add_string "args"
}

find_pid() {
	local cfg="$1"
	local token ns pid

	for token in $(pidof catatonit); do
		ns=$(cat /proc/$token/environ | tr '\0' '\n'|grep NAMESPACE|cut -d'=' -f2)
		[ "$ns" = "$cfg" ] &&
			pid="$token"
	done

	echo $pid
}

proto_infra_setup() {
	local cfg="$1"
	local iface namespace ipaddr netmask mask gateway script args
	local pid=$(find_pid "$cfg")

	json_get_var iface "device"
	json_get_vars namespace ipaddr netmask gateway script args

	[ -z "$iface" ] &&
		iface="$cfg"

	[ -z "$ipaddr" ] && {
		echo "ip address not set for $cfg"
		proto_notify_error "$cfg" "NO_IP_ADDRESS"
		return 1
	}

	[ -z "$namespace" ] && {
		echo "namespace not set for $cfg"
		proto_notify_error "$cfg" "NO_NAMESPACE_SET"
		return 1
	}

	mask="$netmask"

	[ $(echo "$netmask" | cut -c1-1) = "/" ] ||
		mask=$(/bin/ipcalc.sh "$ipaddr" "$netmask" | grep PREFIX | cut -d'=' -f2)

	[ -z "$mask" ] && {
		echo "invalid netmask for $cfg"
		proto_notify_error "$cfg" "INVAlID_NETMASK"
		return 1
	}

	[ -z "$pid" ] && {
		proto_export NAMESPACE="$cfg"
		proto_run_command "$cfg" "/usr/bin/unshare" -u -i -n -U -C "/usr/bin/catatonit" -P
		pid=$(find_pid "$cfg")
	}

	[ -z "$pid" ] && {
		echo "catatonit pid for namespace $cfg was not found, catatonit not running?"
		proto_notify_error "$cfg" "PID_NOT_FOUND"
		return 1
	}

	proto_init_update "$iface" 1
	proto_add_ipv4_address $ipaddr $mask
	proto_add_data
	json_add_string "pid" "$pid"
	json_add_string "namespace" "$namespace"
	proto_close_data
	proto_send_update "$cfg"

	[ -z $(ip netns list | grep "$namespace") ] ||
		ip netns delete "$namespace"

	ip netns attach "$namespace" "$pid"
	ip link set "$iface" netns "$namespace"
	ip -n "$namespace" addr add "${ipaddr}/${mask}" dev "$iface"
	ip -n "$namespace" link set dev lo up
	ip -n "$namespace" link set dev "$iface" up

	[ -n "$gateway" ] &&
		ip -n "$namespace" route add default via "$gateway"

	[ -n "$script" -a -x "$script" ] && {
		interface="$iface" \
		pid="$pid" \
		ipaddr="$ipaddr" \
		netmask="$netmask" \
		mask="$mask" \
		gateway="$gateway" \
		namespace="$namespace" \
		$script "$cfg" $args
	} || {
		[ -n "$script" ] &&
			echo "post setup script $script for $cfg does not exist or is not executable"
	}
}

proto_infra_teardown() {
	local cfg="$1"
	return 0
}

[ -n "$INCLUDE_ONLY" ] || {
	add_protocol infra
}
