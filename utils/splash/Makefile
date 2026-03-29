include $(TOPDIR)/rules.mk

PKG_NAME:=splash
PKG_VERSION:=1.6.1
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/oskarirauta/$(PKG_NAME)/releases/download/$(PKG_VERSION)/
PKG_HASH:=082cf8359d9184fb40caf3bbfcf0f3582948d09a52c8b95f27a344edd8b0a9ba

PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>
PKG_BUILD_DEPENDS:=freetype-dev
PKG_RUNTIME_DEPENDS:=
PKG_FORTIFY_SOURCE:=0

define Package/splash/config
  source "$(SOURCE)/Config.in"
endef

PKG_BUILD_DEPENDS+=libpng freetype zlib

ifeq ($(CONFIG_SPLASH_JPG),y)
	PKG_BUILD_DEPENDS+=libjpeg-turbo
endif

include $(INCLUDE_DIR)/package.mk

define Package/splash
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Boot splash
  DEPENDS:=+libpng +libfreetype +zlib +SPLASH_JPG:libjpeg-turbo
  VARIANT:=dynamic
endef

define Package/splash/description
  Splash is a graphical term for representation of boot process.
endef

define Package/splash-static
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Boot splash (static build)
  DEPENDS:=
  VARIANT:=static
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
	EXTRA_LDFLAGS="-L$(STAGING_DIR)/usr/lib -L$(STAGING_DIR)/lib"

ifeq ($(BUILD_VARIANT),dynamic)
	MAKE_VARS += \
		LIBJPEG="-ljpeg" LIBFREETYPE="-lfreetype" LIBPNG="-lpng" LIBZ="-lz"
else
	MAKE_VARS += \
		LIBJPEG="$(STAGING_DIR)/usr/lib/libjpeg.a" \
		LIBFREETYPE="$(STAGING_DIR)/usr/lib/libfreetype.a" \
		LIBPNG="$(STAGING_DIR)/usr/lib/libpng16.a" \
		LIBZ="$(STAGING_DIR)/usr/lib/libz.a"
	TARGET_CFLAGS += -static
endif

TARGET_CFLAGS += -nostdinc -I$(STAGING_DIR)/usr/include -I$(STAGING_DIR)/usr/include/freetype2 -I$(STAGING_DIR)/usr/include/freetype2/freetype

define Package/splash/install
	$(INSTALL_DIR) $(1)/usr/bin $(1)/usr/share/splash/fonts $(1)/usr/share/splash/images
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/splash $(1)/usr/bin/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/res/fonts/** $(1)/usr/share/splash/fonts/
	$(INSTALL_DATA) ./files/openwrt.png $(1)/usr/share/splash/images/splash.png
endef

define Package/splash-static/install
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
$(eval $(call BuildPackage,splash-static))
$(eval $(call BuildPackage,splash-init))
