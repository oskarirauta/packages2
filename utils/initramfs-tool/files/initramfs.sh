#!/bin/sh

EXE="$0"
WANT_HELP=""
WANT_EXTRACT=""
WANT_CREATE=""
WANT_BUILD=""

exiterr() {
	echo "error: $1"
	exit 1
}

check_usage() {
	local __wanthelp=""

	[ "$1" = "--h" -o "$1" = "-h" ] && __wanthelp="true"
	[ "$1" = "--help" -o "$1" = "-help" ] && __wanthelp="true"
	[ "$1" = "--usage" -o "$1" = "-usage" ] && __wanthelp="true"

	export "$2=$__wanthelp"
}

check_extract() {
	local __wantx=""

	[ "$1" = "extract" -o "$1" = "x" ] && __wantx="true"
	[ "$1" = "--extract" -o "$1" = "-extract" ] && __wantx="true"
	[ "$1" = "--x" -o "$1" = "-x" ] && __wantx="true"

	export "$2=$__wantx"
}

check_create() {
	local __wantc=""

	[ "$1" = "create" -o "$1" = "c" ] && __wantc="true"
	[ "$1" = "--create" -o "$1" = "-create" ] && __wantc="true"
	[ "$1" = "--c" -o "$1" = "-c" ] && __wantc="true"

	export "$2=$__wantc"
}

check_build() {
	local __wantb=""

	[ "$1" = "build" -o "$1" = "b" ] && __wantb="true"
	[ "$1" = "--build" -o "$1" = "-build" ] && __wantb="true"
	[ "$1" = "--b" -o "$1" = "-b" ] && __wantb="true"

	export "$2=$__wantb"
}

initrd_help() {
	echo "  $1 --create FILENAME SOURCEPATH"
	echo "  $1 --extract FILENAME [TARGETDIR]"
	echo "  $1 --build TARGETDIR"
	echo "  $1 --help"
}

validate_action() {
	local __exe=$(basename "$EXE")
	local __wantc="$1"
	local __wantx="$2"
	local __wantb="$3"
	local __wantu

	[ -z "$__wantc" -a -z "$__wantx" ] && {
		[ -z "$__wantb" ] && __wantu="true"
	}
	[ -n "$__wantc" -a -n "$__wantx" ] && __wantu="true"
	[ -n "$__wantc" -a -n "$__wantb" ] && __wantu="true"
	[ -n "$__wantx" -a -n "$__wantb" ] && __wantu="true"

	[ "$__wantu" = "true" ] && {
		echo "Usage:"
		initrd_help "$__exe"
		exit 0
	}
}

usage() {
	local __wantc="$1"
	local __wantx="$2"
	local __wantb="$3"
	local __exe=$(basename "$EXE")

	echo "Usage:"

	validate_action "$__wantc" "$__wantx" "$__wantb"

	[ "$__wantc" = "true" ] && {
		echo "  $__exe --create FILENAME SOURCEPATH"
		exit 0
	}

	[ "$__wantx" = "true" ] && {
		echo "  $__exe --extract FILENAME [TARGETDIR]"
		exit 0
	}

	[ "$__wantb" = "true" ] && {
		echo "  $__exe --build TARGETDIR"
		exit 0
	}

	echo "Unknown parameters"
	initrd_help "$__exe"
	exit 1
}

get_file() {
	local __fpath
	[ -z "$1" ] && exiterr "filename argument missing"
	[ -n "$1" ] && __fpath=$(readlink -f "$1")
	[ -n "$__fpath" -a -d "$__fpath" ] && \
		exiterr "file $1 is not file, it is directory"
	[ -z "$__fpath" -o ! -f "$__fpath" ] && exiterr "file does not exist, or path does not resolve: $1"
	export "$2=$__fpath"
}

get_dir() {
	local __fpath
	[ -z "$1" ] && exiterr "path argument missing"
	[ -n "$1" ] && __fpath=$(readlink -f "$1")
	[ -n "$__fpath" -a -f "$__fpath" ] && \
		exiterr "path $1 is not a directory, it is file"
	[ -z "$__fpath" -o ! -d "$__fpath" ] && exiterr "path does not exist, or it is unresolvable: $1"
	export "$2=$__fpath"
}

get_pwd() {
	local __fpath=$(readlink -f "$(pwd)")
	[ -z "$__fpath" ] && exiterr "could not get working directory"
	[ -d "$__fpath" ] || exiterr "current working directory $__fpath is not directory???"
	export "$1=$__fpath"
}

get_tmpfile() {
	local __file=$(mktemp -u)
	[ -f "$__file" ] && get_tmpfile __file
	touch "$__file"
	[ -f "$__file" ] && rm -f "$__file" || exiterr "temporary file error, generated file $__file is not writable"
	export "$1=$__file"
}

get_yes() {
	local _key
	stty raw
	_key=$(dd bs=1 count=1 2> /dev/null)
	stty -raw
	[ "$_key" = "y" -o "$_key" = "Y" ] && _key="true" || _key=""
	echo ""
	export "$1=$_key"
}

get_newfile() {
	[ -z "$1" ] && exiterr "filename argument missing"

	local __fpath=$(readlink -f "$1")
	local __bname=$(basename "${__fpath:-$1}")
	local __dname=$(dirname "$1")
	local __yes

	[ -z "$__fpath" ] && __fpath="${__dname}/${__bname}"
	[ -n "$__fpath" -a -d "$__fpath" ] && exiterr "file $1 is directory; enter filename as argument"
	[ -n "$__fpath" -a -e "$__fpath" ] && {
		[ -d "$__fpath" ] && exiterr "$__fpath is directory; enter filename as argument"
		[ -f "$__fpath" ] || exiterr "$__fpath is not file; enter filename as argument"
		echo "file $1 already exists, do you want to remove it? [y/N]"
		get_yes __yes
		[ -z "$__yes" ] && {
			echo "Aborted."
			exit 0
		}
		rm -f "$__fpath"
		[ -f "$__fpath" ] && exiterr "failed to remove $__fpath - aborting"
	}
	[ -z "$__fpath" ] && exiterr "unknown error, cannot resolve target file"
	touch "$__fpath"
	[ -f "$__fpath" ] && __fpath=$(readlink -f "$__fpath") || exiterr "file $__fpath is not writable"
	rm -f "$__fpath"
	export "$2=$__fpath"
}

validate_src() {
	[ -z "$1" ] && exiterr "source path missing"
	[ -f "$1" ] && exiterr "path required, but argument is file: $1"
	[ -d "$1" ] || exiterr "path $1 does not exist"

	[ -f "${1}/init" ] || exiterr "file ${1}/init does not exist"
	[ -x "${1}/init" ] || exiterr "file ${1}/init is not executable"
	[ -d "${1}/bin" ] || exiterr "path ${1}/bin does not exist"
	[ -f "${1}/bin/busybox" ] || exiterr "file ${1}/bin/busybox does not exist"
	[ -x "${1}/bin/busybox" ] || exiterr "file ${1}/bin/busybox is not executable"
}

link_bb() {
	for i in "$@"; do ln -s busybox "bin/$i" ; done
}

initrd_extract() {

	local __file __dir __pwd

	get_file "$1" __file
	get_dir "${2:-.}" __dir
	get_pwd __pwd

	[ "$__pwd" = "$__dir" ] || cd "$__dir"
	zcat "$__file" | cpio -idmv
	[ "$__pwd" = "$__dir" ] || cd "$__pwd"
}

initrd_create() {
	local __tmp __newfile __src __pwd

	get_tmpfile __tmp
	get_newfile "$1" __newfile
	get_dir "$2" __src
	get_pwd __pwd
	validate_src "$__src"

	[ "$__pwd" = "$__src" ] || cd "$__src"
	find . | cpio -oH newc | gzip -9 | tee "$__tmp" > /dev/null
	[ "$__pwd" = "$__src" ] || cd "$__pwd"
	mv "$__tmp" "$__newfile"
}

initrd_build() {
	local __dir __pwd __libblkid
	get_dir "${1:-.}" __dir
	get_pwd __pwd
	[ "$(ls -A ${__dir})" ] && exiterr "path $1 is not empty"

	cd "$__dir"

	mkdir -p bin boot dev etc lib mnt proc root sbin sys usr mnt usr/lib usr/bin usr/sbin lib/modules

	link_bb awk basename blockdev cat clear cut echo egrep env expr false fgrep find findfs grep head \
		insmod ls lsmod mkdir mknod mktemp modinfo modprobe mount mountpoint pivot_root readlink \
		realpath reset rmmod sed sh sleep stty switch_root sync tee touch tty umount yes

	cp /lib/modules/$(uname -r)/fat.ko lib/modules/
	cp /lib/modules/$(uname -r)/nls_cp437.ko lib/modules/
	cp /lib/modules/$(uname -r)/nls_cp852.ko lib/modules/
	cp /lib/modules/$(uname -r)/nls_iso8859-1.ko lib/modules/
	cp /lib/modules/$(uname -r)/vfat.ko lib/modules/

	cp /lib/libc.so lib/
	ln -s libc.so lib/ld-musl-x86_64.so.1

	cp /lib/libgcc_s.so.1 lib/

	__libblkid=$(readlink /usr/lib/libblkid.so.1)
	cp "/usr/lib/$__libblkid" usr/lib/
	ln -s "$__libblkid" "usr/lib/libblkid.so.1"

	cp /bin/busybox bin/
	cp /usr/sbin/blkid usr/sbin/

	mknod dev/console c 5 1
	mknod dev/tty c 5 0
	mknod dev/null c 1 3
	mknod dev/fb0 c 29 0

	cat > init << EOF
#!/bin/busybox sh

exit_shell() {
	[ -n "\$1" ] && echo -e "\${1}-e[?25h" > /dev/tty1
	echo -e "Something went wrong..-e[?25h" > /dev/tty1
	exec </dev/tty1 >/dev/tty1 2>/dev/tty1
	exec /bin/busybox sh
}

mount -t proc none /proc
mount -t sysfs none /sys
mount -t devtmpfs none /dev

insmod /lib/modules/nls_cp437.ko
insmod /lib/modules/nls_cp852.ko
insmod /lib/modules/nls_iso8859-1.ko
insmod /lib/modules/fat.ko
insmod /lib/modules/vfat.ko

echo 5 > /proc/sys/kernel/printk
echo -e "\e[?25l"

ROOTUUID=\$(cat /proc/cmdline | sed -e 's/^.*root=//' -e 's/ .*\$//' | cut -d '=' -f 2)
[ -z "\$ROOTUUID" ] && exit_shell "root partition's uuid parsing has failed"
ROOTPART=\$(/usr/sbin/blkid | grep "\$ROOTUUID" | cut -d ':' -f 1)
[ -z "\$ROOTPART" ] && exit_shell "cannot find root partition with uuid \$ROOTUUID"

mount "\$ROOTPART" /mnt || exit_shell "failed to mount root partition \$ROOTPART to /mnt"

# put your init stuff here

umount /proc /sys
mount --move /dev /mnt/dev

exec switch_root /mnt /sbin/init
EOF
	chmod +x init

	cd "$__pwd"
}

[ -z "$1" ] && {
	echo "Usage:"
	initrd_help $(basename "$EXE")
	exit 0
}

# main

check_usage "$1" WANT_HELP

[ -n "$WANT_HELP" ] && {
	check_create "$2" WANT_CREATE
	check_extract "$2" WANT_EXTRACT
	check_build "$2" WANT_BUILD
	usage "$WANT_CREATE" "$WANT_EXTRACT" "$WANT_BUILD"
} || {
	check_create "$1" WANT_CREATE
	check_extract "$1" WANT_EXTRACT
	check_build "$1" WANT_BUILD
	[ -n "$2" ] && check_usage "$2" WANT_HELP
	[ -n "$3" -a -z "$WANT_HELP" ] && check_usage "$3" WANT_HELP
	[ -n "$4" -a -z "$WANT_HELP" ] && check_usage "$4" WANT_HELP
	[ -n "$WANT_HELP" ] && usage "$WANT_CREATE" "$WANT_EXTRACT" "$WANT_BUILD"
}

validate_action "$WANT_CREATE" "$WANT_EXTRACT" "$WANT_BUILD"

[ -n "$WANT_CREATE" ] && {
	[ -n "$4" ] && exiterr "Too many arguments for create: $4"
	initrd_create "$2" "$3"
	exit 0
}

[ -n "$WANT_EXTRACT" ] && {
	[ -n "$4" ] && exiterr "Too many arguments for extract: $4"
	initrd_extract "$2" "$3"
	exit 0
}

[ -n "$WANT_BUILD" ] && {
	[ -n "$3" ] && exiterr "Too many arguments for build: $3"
	initrd_build "$2"
	exit 0
}
