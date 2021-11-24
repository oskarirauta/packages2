#
# Copyright (C) 2016 Rafał Miłecki <rafal@milecki.pl>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=systembus
PKG_VERSION:=0.95
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/oskarirauta/systembus.git
PKG_SOURCE_DATE:=2021-11-24
PKG_SOURCE_VERSION:=23dcc03f03062593d8e25dec77f08b1d272f2d54
PKG_MIRROR_HASH:=12f7e3e8091543d7923d8953151da0fc16ebbd4eb99a35f4a27afd24426ef443

PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

include $(INCLUDE_DIR)/package.mk

define Package/systembus
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=System statistics via ubus
  DEPENDS:=+libubus +libubox +libblobmsg-json +libuci +libstdcpp +jsoncpp
  MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>
endef

define Package/systembus/description
  systembus provides some statistics of running system via
  ubus. systembus is written in C++.
endef

define Build/Configure
endef

define Package/systembus/install
	$(INSTALL_DIR) $(1)/usr/bin $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/systembus $(1)/usr/bin/
	$(INSTALL_BIN) ./files/systembus.init $(1)/etc/init.d/systembus
endef

$(eval $(call BuildPackage,systembus))
