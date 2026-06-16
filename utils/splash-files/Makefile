include $(TOPDIR)/rules.mk

PKG_NAME:=splash-files
PKG_VERSION:=1.0.0
PKG_RELEASE:=1

PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>

include $(INCLUDE_DIR)/package.mk

define Package/splash-files
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Bootsplash logo and simple fonts
  DEPENDS:=
endef

define Package/splash-files/description
  This package contains a "default" bootsplash logo image and
  some simple fonts. Can be shared with splash and splash-drm.
endef

define Build/Prepare
	$(INSTALL_DIR) $(PKG_BUILD_DIR)
endef

define Build/Configure
endef

define Package/splash-files/install
	$(INSTALL_DIR) $(1)/usr/share/splash/images $(1)/usr/share/splash/fonts
	$(INSTALL_DATA) ./files/openwrt.png $(1)/usr/share/splash/images/splash.png
	$(INSTALL_DATA) ./files/fonts/** $(1)/usr/share/splash/fonts/
endef

$(eval $(call BuildPackage,splash-files))
