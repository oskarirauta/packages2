#
# Copyright (C) 2007-2018 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=host-util-linux
PKG_VERSION:=2.36.1
PKG_RELEASE:=2

PKG_SOURCE:=util-linux-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=@KERNEL/linux/utils/$(PKG_NAME)/v2.36
PKG_HASH:=09fac242172cd8ec27f0739d8d192402c69417617091d8c6e974841568f37eed
PKG_CPE_ID:=cpe:/a:kernel:util-linux
HOST_BUILD_DIR:=$(BUILD_DIR_HOST)/util-linux-$(PKG_VERSION)

PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=	COPYING					\
			libblkid/COPYING			\
			libmount/COPYING			\
			Documentation/licenses/COPYING.GPLv2	\
			Documentation/licenses/COPYING.LGPLv2.1	\
			libuuid/COPYING				\
			Documentation/licenses/COPYING.BSD-3

PKG_BUILD_PARALLEL:=1
PKG_BUILD_DEPENDS:=host-util-linux/host
PKG_FIXUP:=autoreconf
PKG_INSTALL:=1

DISABLE_NLS:=--disable-nls

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/host-build.mk

define Package/host-util-linux
  SECTION:=devel
  CATEGORY:=Development
  TITLE:=host-util-linux-support
  URL:=http://www.kernel.org/pub/linux/utils/util-linux/
  ABI_VERSION:=1
endef

define Package/host-util-linux/description
  Util-linux support for host. Do not attempt to build,
  it has been designed to fail build since this is provided
  by util-linux package which lacks host package building.
  This is mostly required by glib2's host package build.
endef

CONFIGURE_ARGS += \
	--disable-use-tty-group		\
	--disable-rpath			\
	--disable-tls			\
	--disable-su			\
	--disable-sulogin		\
	--disable-makeinstall-chown	\
	--disable-login 		\
	--disable-nologin		\
	--disable-lslogins		\
	--disable-runuser		\
	--disable-chfn-chsh		\
	--without-python		\
	--without-udev			\
	--without-readline		\
	--without-libmagic		\
	--with-ncursesw

TARGET_CFLAGS += $(FPIC) -std=gnu99

HOST_CONFIGURE_ARGS += \
	--disable-use-tty-group --disable-rpath --disable-tls --disable-nls \
	--disable-widechar --disable-all-progams --disable-libuuid \
	--enable-libblkid --enable-libmount --disable-plymouth-support \
	--disable-bash-completion --disable-pylibmount --disable-pg-bell \
	--without-systemd --without-user --without-selinux --without-audit \
	--without-econf --without-python --without-udev --without-readline \
	--without-libmagic --without-cfdisk --disable-cfdisk --without-ncursesw \
	--without-btrfs --without-libmagic --without-cap-ng --disable-libfdisk \
	--disable-fdisks --disable-makeinstall-chown

HOST_CFLAGS += -std=gnu99

define Build/Prepare
	./files/fail-target-package-now
endef

define Package/host-util-linux/install
	$(INSTALL_DIR) $(1)/failure
	$(INSTALL_BIN) ./files/must-fail $(1)/failure
endef

$(eval $(call BuildPackage,host-util-linux))
$(eval $(call HostBuild))
