#!/bin/sh /etc/rc.common

START=10
STOP=90

boot() {

	[ -f "/sys/fs/cgroup/cgroup.subtree_control" ] &&
		echo "+cpuset +cpu +io +memory +pids +rdma" > /sys/fs/cgroup/cgroup.subtree_control
}
