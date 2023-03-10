include $(TOPDIR)/rules.mk

PKG_NAME:=splash
PKG_VERSION:=1.4.2
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/oskarirauta/splash.git
PKG_SOURCE_DATE:=2023-03-10
PKG_SOURCE_VERSION:=4d5b920380d797ca793fbfeeab911bff2b948e77
PKG_MIRROR_HASH:=2cf31fbe4970243c11bf64b2099afa56eda4353da39b347d044cfc7c70bd2e7e

PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=LICENSE
PKG_BUILD_DEPENDS:=freetype libpng libjpeg-turbo zlib

include $(INCLUDE_DIR)/package.mk

define Package/splash
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Boot splash
  DEPENDS:=
  MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>
endef

define Package/splash/description
  Splash is a graphical term for representation of boot process.
endef

define Build/Configure
endef

TARGET_LDFLAGS += -w -static
MAKE_VARS += LIBS="$(STAGING_DIR)/usr/lib/libfreetype.a $(STAGING_DIR)/usr/lib/libjpeg.a $(STAGING_DIR)/usr/lib/libpng16.a $(STAGING_DIR)/usr/lib/libz.a"

define Package/splash/install
	$(INSTALL_DIR) $(1)/usr/bin $(1)/usr/share/splash/fonts $(1)/usr/share/splash/images
	$(INTALL_BIN) $(PKG_BUILD_DIR)/splash $(1)/usr/bin
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/res/fonts/** $(1)/usr/share/splash/fonts/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/res/images/** $(1)/usr/share/splash/images/

	[ "$(CONFIG_NO_STRIP)" = "y" ] && \
		STRIP=$(STAGING_DIR_HOST)/bin/sstrip $(SCRIPT_DIR)/rstrip.sh $(1)/usr/bin
endef

$(eval $(call BuildPackage,splash))
