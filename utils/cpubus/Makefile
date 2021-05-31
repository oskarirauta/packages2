#
# Copyright (C) 2016 Rafał Miłecki <rafal@milecki.pl>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=cpubus
PKG_VERSION:=0.99
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/cpubus
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=CPUBus
  DEPENDS:=+libubus +libubox +libblobmsg-json +libstdcpp
  MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>
endef

define Package/cpubus/description
  Provides cpu stats through ubus
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		CXX="$(TARGET_CXX)" \
		CXXFLAGS="$(TARGET_CXXFLAGS) -Wall" \
		LDFLAGS="-L$(STAGING_DIR)/lib -L$(STAGING_DIR)/usr/lib $(TARGET_LDFLAGS)"
endef

define Package/cpubus/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/cpubus $(1)/usr/bin/
endef

$(eval $(call BuildPackage,cpubus))
