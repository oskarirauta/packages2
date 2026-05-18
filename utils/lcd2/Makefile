include $(TOPDIR)/rules.mk

PKG_NAME:=lcd2
PKG_VERSION:=1.0.2
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/oskarirauta/lcd2/releases/download/$(PKG_VERSION)/
PKG_HASH:=f236f180b5ada1497f18875ec9d6e1bb2b4bd149096e2c312351591f00c64db9

PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>

include $(INCLUDE_DIR)/package.mk

define Package/lcd2
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=modern lcd4linux rewrite
  DEPENDS:=+libdrm +libgd +libusb-compat +libstdcpp
  URL:=https://github.com/oskarirauta/lcd2
endef

define Package/lcd2/description
  A modern, C++23 rewrite of lcd4linux for Linux display systems.
  The application renders information widgets onto framebuffer displays using DRM/KMS or USB display drivers.
endef

TARGET_CXXFLAGS += --std=c++23 -Os -Wall
MAKE_VARS += DRM_INCLUDES="-I$(STAGING_DIR)/usr/include/libdrm"
MAKE_ARGS += WITH_UBUS=1

define Build/Configure
endef

define Package/lcd2/install
	$(INSTALL_DIR) $(1)/etc $(1)/etc/init.d $(1)/usr/bin
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/example_drm.conf $(1)/etc/lcd2.conf
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/lcd2 $(1)/usr/bin/
	$(INSTALL_BIN) ./files/lcd2.init $(1)/etc/init.d/lcd2
endef

$(eval $(call BuildPackage,lcd2))
