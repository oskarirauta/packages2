0;83;19M0;83;19m#
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
PKG_SOURCE_DATE:=2023-03-02
PKG_SOURCE_VERSION:=57b8cbf92f9d4c729db409dc754e327751e7a0d3
PKG_MIRROR_HASH:=skip

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
	[ "$(CONFIG_NO_STRIP)" = "y" ] && \
		STRIP=$(STAGING_DIR_HOST)/bin/sstrip $(SCRIPT_DIR)/rstrip.sh $(1)/usr/bin
endef

$(eval $(call BuildPackage,podman-ubus))
