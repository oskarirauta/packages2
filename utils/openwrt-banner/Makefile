include $(TOPDIR)/rules.mk

PKG_NAME:=openwrt-banner
PKG_VERSION:=0.89.2
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/oskarirauta/openwrt-banner/releases/download/$(PKG_VERSION)/
PKG_HASH:=d7bd4ba2f0115631bcba37ee036a14fe4c5af9f26004f278290c5e330ba74fec

PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

PKG_CONFIG_DEPENDS := CONFIG_OPENWRT_BANNER_RELEASE

include $(INCLUDE_DIR)/package.mk

define Package/openwrt-banner
  SECTION:=base
  CATEGORY:=Base system
  TITLE:=simple login banner
  DEPENDS:=+libstdcpp
  URL:=https://github.com/oskarirauta/openwrt-banner
endef

define Package/openwrt-banner/description
  alternative simple login banner/motd written in c++
  to replace banner generated by profile script
endef

define Package/openwrt-banner/config
        source "$(SOURCE)/Config.in"
endef

TARGET_CXXFLAGS += --std=c++23 -fPIC

ifneq ($(CONFIG_OPENWRT_BANNER_RELEASE),)
	MAKE_VARS += \
		HARDCODED_RELEASE="$(CONFIG_OPENWRT_BANNER_RELEASE)"
endif

define Build/Configure
endef

define Package/openwrt-banner/install
	$(INSTALL_DIR) $(1)/bin
	$(CP) $(PKG_BUILD_DIR)/banner $(1)/bin/
endef

$(eval $(call BuildPackage,openwrt-banner))
