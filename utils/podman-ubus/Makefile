#
# Copyright (C) 2016 Rafał Miłecki <rafal@milecki.pl>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=podman-ubus
PKG_VERSION:=0.97
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/oskarirauta/podman-ubus.git
PKG_SOURCE_DATE:=2023-02-15
PKG_SOURCE_VERSION:=d9e54c181d64aa3ccfd30ff7efa643335da50a1f
PKG_MIRROR_HASH:=9bdb668377cf5c6e541b404430968d3535727a4d38617e34baa14d9a5287fdc8

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
  externed from C. It uses staticly linked libcurl for unix
  socket operations.
endef

MAKE_VARS += \
	BUILD_DIR="$(PKG_BUILD_DIR)"

define Build/Configure
endef

define Package/podman-ubus/install
	$(INSTALL_DIR) $(1)/usr/bin $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/podman-ubus $(1)/usr/bin/
	$(INSTALL_BIN) ./files/podman-ubus.init $(1)/etc/init.d/podman-ubus
endef

$(eval $(call BuildPackage,podman-ubus))
