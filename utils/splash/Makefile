include $(TOPDIR)/rules.mk

PKG_NAME:=splash
PKG_VERSION:=1.6.0
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/oskarirauta/$(PKG_NAME)/releases/download/$(PKG_VERSION)/
PKG_HASH:=e7ebc26a6ca21259f1398076265bb1d1b6384bc54f283fdceda7849db431d21e

PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>
PKG_BUILD_DEPENDS:=freetype-dev
PKG_RUNTIME_DEPENDS:=

define Package/splash/config
  source "$(SOURCE)/Config.in"
endef

ifeq ($(CONFIG_SPLASH_STATIC_LINK),y)
	ifeq ($(CONFIG_SPLASH_JPG),y)
		PKG_BUILD_DEPENDS+=libjpeg-turbo libpng freetype zlib
	else
		PKG_BUILD_DEPENDS+=libpng freetype zlib
	endif
else
	ifeq ($(CONFIG_SPLASH_JPG),y)
		PKG_RUNTIME_DEPENDS+=+libjpeg-turbo +libpng +libfreetype +zlib
	else
		PKG_RUNTIME_DEPENDS+=+libpng +libfreetype +zlib
	endif
endif

include $(INCLUDE_DIR)/package.mk

define Package/splash
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Boot splash
  DEPENDS:=$(PKG_RUNTIME_DEPENDS)
endef

define Package/splash/description
  Splash is a graphical term for representation of boot process.
endef

define Package/splash-init
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Boot splash initscript
  DEPENDS:=splash
endef

define Package/splash-init/description
  Init script to display openwrt splash on boot and shutdown
endef

define Build/Configure
endef

MAKE_VARS += \
	DISTRIBUTION="openwrt" \
	WITH_JPEG=$(if $(CONFIG_SPLASH_JPG),1,0) \
	WITH_BMP=$(if $(CONFIG_SPLASH_BMP),1,0) \
	LIBJPEG="$(if $(CONFIG_SPLASH_STATIC_LINK),$(STAGING_DIR)/usr/lib/libjpeg.a,-ljpeg)" \
	LIBFREETYPE="$(if $(CONFIG_SPLASH_STATIC_LINK),$(STAGING_DIR)/usr/lib/libfreetype.a,-lfreetype)" \
	LIBPNG="$(if $(CONFIG_SPLASH_STATIC_LINK),$(STAGING_DIR)/usr/lib/libpng16.a,-lpng)" \
	LIBZ="$(if $(CONFIG_SPLASH_STATIC_LINK),$(STAGING_DIR)/usr/lib/libz.a,-lz)"

TARGET_CFLAGS += -I$(STAGING_DIR)/usr/include/freetype2 -I$(STAGING_DIR)/usr/include/freetype2/freetype

ifeq ($(CONFIG_SPLASH_STATIC_LINK),y)
	TARGET_CFLAGS += -static
endif

define Package/splash/install
	$(INSTALL_DIR) $(1)/usr/bin $(1)/usr/share/splash/fonts $(1)/usr/share/splash/images
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/splash $(1)/usr/bin/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/res/fonts/** $(1)/usr/share/splash/fonts/
	$(INSTALL_DATA) ./files/openwrt.png $(1)/usr/share/splash/images/splash.png
endef

define Package/splash-init/install
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/splash.init $(1)/etc/init.d/splash
endef

$(eval $(call BuildPackage,splash))
$(eval $(call BuildPackage,splash-init))
