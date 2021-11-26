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
PKG_SOURCE_VERSION:=d7434b5a842d74dc6985a04a9a1ddb948f668a5f
PKG_MIRROR_HASH:=729277bed9ee34c7c68b6950ca790c7589172c9a76daf869d6e5aba8932d611e

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
