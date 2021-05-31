#
# Copyright (C) 2016 Rafał Miłecki <rafal@milecki.pl>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=systembus
PKG_VERSION:=0.89
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

PKG_CONFIG_DEPENDS:= \
	CONFIG_SYSTEMBUS_SELINUX

define Package/systembus
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=System information via Ubus
  DEPENDS:=+libubus +libubox +libblobmsg-json +libuci +libstdcpp +SYSTEMBUS_SELINUX:libselinux
  MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>
endef

define Package/systembus/description
  Provides cpu and system stats through ubus
endef

define Package/systembus/config
  if PACKAGE_systembus
	config SYSTEMBUS_SELINUX
		bool "Enable SELinux support"
		default n
		help
		  This option enables SELinux status retrieval
  endif
endef

SELINUX_FLAGS:=

ifeq ($(CONFIG_SYSTEMBUS_SELINUX),y)
  DEPENDS+= +libselinux
  SELINUX_FLAGS+= selinux=1
endif

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		CXX="$(TARGET_CXX)" \
		CXXFLAGS="$(TARGET_CXXFLAGS) -std=c++17 -Wall" \
		LDFLAGS="-L$(STAGING_DIR)/lib -L$(STAGING_DIR)/usr/lib $(TARGET_LDFLAGS)" \
		$(SELINUX_FLAGS)
endef

define Package/systembus/install
	$(INSTALL_DIR) $(1)/usr/bin $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/systembus $(1)/usr/bin/
	$(INSTALL_BIN) ./files/systembus.init $(1)/etc/init.d/systembus
endef

$(eval $(call BuildPackage,systembus))
