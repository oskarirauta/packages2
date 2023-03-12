include $(TOPDIR)/rules.mk

PKG_NAME:=splash
PKG_VERSION:=1.4.4
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/oskarirauta/splash.git
PKG_SOURCE_DATE:=2023-03-12
PKG_SOURCE_VERSION:=963b9650571a7f3d654879e0266ef7b43e6e4329
PKG_MIRROR_HASH:=856a44015bd7ff2ad79c2f588f5c4c9ab79db78fa37c91fd67a64d3903597190

PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>
PKG_BUILD_DEPENDS:=SPLASH_STATIC_LINK:libpng SPLASH_STATIC_LINK:freetype SPLASH_STATIC_LINK:zlib

include $(INCLUDE_DIR)/package.mk

define Package/splash
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Boot splash
  DEPENDS:=+(SPLASH_JPG):libjpeg-turbo +(!SPLASH_STATIC_LINK):libpng +(!SPLASH_STATIC_LINK):libfreetype +(!SPLASH_STATIC_LINK):zlib
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

define Package/splash/config
  source "$(SOURCE)/Config.in"
endef

define Build/Configure
endef

MAKE_VARS += \
	DISTRIBUTION="openwrt" \
	WITH_JPEG=$(if $(CONFIG_SPLASH_JPG),1,0) \
	WITH_BMP=$(if $(CONFIG_SPLASH_BMP),1,0) \
	LIBJPEG="-ljpeg" \
	LIBFREETYPE="$(if $(CONFIG_SPLASH_STATIC_LINK),$(STAGING_DIR)/usr/lib/libfreetype.a,-lfreetype)" \
	LIBPNG="$(if $(CONFIG_SPLASH_STATIC_LINK),$(STAGING_DIR)/usr/lib/libpng16.a,-lpng)" \
	LIBZ="$(if $(CONFIG_SPLASH_STATIC_LINK),$(STAGING_DIR)/usr/lib/libz.a,-lz)"

TARGET_CFLAGS += -I$(STAGING_DIR)/usr/include/freetype2 -I$(STAGING_DIR)/usr/include/freetype2/freetype

define Package/splash/install
	$(INSTALL_DIR) $(1)/usr/bin $(1)/usr/share/splash/fonts $(1)/usr/share/splash/images
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/splash $(1)/usr/bin/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/res/fonts/** $(1)/usr/share/splash/fonts/
	$(INSTALL_DATA) ./files/openwrt.png $(1)/usr/share/splash/images/splash.png

	[ "$(CONFIG_NO_STRIP)" = "y" ] && \
		STRIP=$(STAGING_DIR_HOST)/bin/sstrip $(SCRIPT_DIR)/rstrip.sh $(1)/usr/bin
endef

define Package/splash-init/install
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/splash.init $(1)/etc/init.d/splash
endef

$(eval $(call BuildPackage,splash))
$(eval $(call BuildPackage,splash-init))
