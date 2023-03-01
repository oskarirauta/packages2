#
# Copyright (C) 2016 Rafał Miłecki <rafal@milecki.pl>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=systembus
PKG_VERSION:=0.99
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/oskarirauta/systembus.git
PKG_SOURCE_DATE:=2023-03-01
PKG_SOURCE_VERSION:=3e7dee1a9322d031449506b32b86eeeab86a4f07
PKG_MIRROR_HASH:=skip

PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

include $(INCLUDE_DIR)/package.mk

define Package/systembus
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=System statistics via ubus
  DEPENDS:=+libubus +libubox +libblobmsg-json +libuci +libstdcpp +jsoncpp +SYSTEMBUS_SELINUX:libselinux
  MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>
endef

define Package/systembus/description
  systembus provides statistics of running system via
  ubus. systembus is written in C++.
endef

define Package/systembus/config
	source "$(SOURCE)/Config.in"
endef

define Build/Configure
endef

ifeq ($(CONFIG_SYSTEMBUS_SELINUX),y)
MAKE_VARS += selinux=1
endif

TARGET_CXXFLAGS += -Wno-builtin-declaration-mismatch

define Package/systembus/install
	$(INSTALL_DIR) $(1)/usr/bin $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/systembus $(1)/usr/bin/
	$(INSTALL_BIN) ./files/systembus.init $(1)/etc/init.d/systembus
	[ "$(CONFIG_NO_STRIP)" = "y" ] && \
		STRIP=$(STAGING_DIR_HOST)/bin/sstrip $(SCRIPT_DIR)/rstrip.sh $(1)/usr/bin
endef

$(eval $(call BuildPackage,systembus))
