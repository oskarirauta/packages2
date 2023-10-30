include $(TOPDIR)/rules.mk

PKG_NAME:=ubus-bm
PKG_VERSION:=0.97.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/oskarirauta/ubus-bm/releases/download/$(PKG_VERSION)/
PKG_HASH:=d7bd4ba2f0115631bcba37ee036a14fe4c5af9f26004f278290c5e330ba74fec

PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

include $(INCLUDE_DIR)/package.mk

define Package/ubus-bm
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=ubus bandwidth monitor
  DEPENDS:=
  URL:=https://github.com/oskarirauta/ubus-bm
endef

define Package/ubus-bm/description
  Very minimal bandwidth monitor for Openwrt. Exposes usage reports through ubus. Written in C++
endef

TARGET_CXXFLAGS += --std=c++23 -fPIC

define Build/Configure
endef

define Package/ubus-bm/install
	$(INSTALL_DIR) $(1)/usr/bin /etc/init.d
	$(CP) $(PKG_BUILD_DIR)/banner $(1)/usr/bin/
	$(INSTALL_BIN) ./files/ubus-bm.init $(1)/etc/init.d/ubus-bm
endef

$(eval $(call BuildPackage,ubus-bm))
