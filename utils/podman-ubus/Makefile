#
# Copyright (C) 2016 Rafał Miłecki <rafal@milecki.pl>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=podman-ubus
PKG_VERSION:=0.96
PKG_RELEASE:=2

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/oskarirauta/podman-ubus.git
PKG_SOURCE_DATE:=2021-11-25
PKG_SOURCE_VERSION:=e98ed5d84cf42e212bf7738f748aa79966fedf8f
PKG_MIRROR_HASH:=780aa372ca4f9eb9b5dae59ad5e75c79d88b40b9eaba0fe374782d12d866ebf8

PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

include $(INCLUDE_DIR)/package.mk

define Package/podman-ubus
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Podman manager via Ubus
  DEPENDS:=+libubus +libubox +libblobmsg-json +libuci +jsoncpp +libstdcpp +podman
  MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>
endef

define Package/podman-ubus/description
  podman-ubus extends ubus with bindings to podman and allows
  controlling and statistics over pods and containers through
  ubus.

  podman-ubus is written in C++ except for ubus parts which are
  externed from C.
endef

define Build/Configure
endef

define Package/podman-ubus/install
	$(INSTALL_DIR) $(1)/usr/bin $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/podman-ubus $(1)/usr/bin/
	$(INSTALL_BIN) ./files/podman-ubus.init $(1)/etc/init.d/podman-ubus
endef

$(eval $(call BuildPackage,podman-ubus))
