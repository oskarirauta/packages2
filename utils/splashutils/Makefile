include $(TOPDIR)/rules.mk

PKG_NAME:=splashutils
PKG_VERSION:=1.5.4.4
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=https://master.dl.sourceforge.net/project/fbsplash.berlios/
PKG_HASH:=74cd5623e35597eacde66f995ce0e390b4401b11c1cc72c834e1ec60beb21df1
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

PKG_INSTALL:=1
PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>
PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=COPYING
PKG_BUILD_DEPENDS:=libjpeg-turbo freetype libpng zlib freetype-dev
PKG_FIXUP:=autoreconf

include $(INCLUDE_DIR)/package.mk

define Package/splashutils
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=splash screen for Linux
  DEPENDS:=+libsplashutils
  URL:=https://sourceforge.net/projects/fbsplash.berlios/
endef

define Package/libsplashutils
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=libraries for splashutils
  DEPENDS:=+libjpeg-turbo +libfreetype +libpng +zlib
  URL:=https://sourceforge.net/projects/fbsplash.berlios/
endef

define Package/splashutils-scripts
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=perl and python scripts for splashutils
  DEPENDS:=
  URL:=https://sourceforge.net/projects/fbsplash.berlios/
endef

define Package/splashutils/description
  A userspace implementation of a splash screen for Linux (formerly known as gensplash)
endef

define Package/libsplashutils/description
  This package contains shared libraries needed by splashutils
endef

define Package/splashutils-scripts/description
  Various scripts from splashutils package
endef

CONFIGURE_ARGS += \
	--without-gpm \
	--without-mng \
	--with-png \
	--with-ttf \
	--with-freetype2 \
	--disable-fbcondecor \
	--disable-helper

define Build/Configure
	$(Build/Configure/Default)
	cp -f $(STAGING_DIR)/usr/lib/libjpeg.a $(PKG_BUILD_DIR)/libs/
	cp -f $(STAGING_DIR)/usr/lib/libfreetype.a $(PKG_BUILD_DIR)/libs/
	cp -f $(STAGING_DIR)/usr/lib/libpng.a $(PKG_BUILD_DIR)/libs/
	cp -f $(STAGING_DIR)/usr/lib/libz.a $(PKG_BUILD_DIR)/libs/
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib $(1)/usr/include $(1)/usr/lib/pkgconfig
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/{libfbsplash.a,libfbsplashrender.a} $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/include/fbsplash.h $(1)/usr/include/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/*.pc $(1)/usr/lib/pkgconfig/
endef

define Package/splashutils/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/{fbsplashd,splash_util} $(1)/usr/sbin/
endef

define Package/libsplashutils/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/{libfbsplash.so.1.0.0,libfbsplashrender.so.1.0.0} $(1)/usr/lib/
	$(LN) libfbsplash.so.1.0.0 $(1)/usr/lib/libfbsplash.so.1
	$(LN) libfbsplash.so.1.0.0 $(1)/usr/lib/libfbsplash.so
	$(LN) libfbsplashrender.so.1.0.0 $(1)/usr/lib/libfbsplashrender.so.1
	$(LN) libfbsplashrender.so.1.0.0 $(1)/usr/lib/libfbsplashrender.so
endef

define Package/splashutils-scripts/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/{bootsplash2fbsplash,splash_manager,splash_resize,splashy2fbsplash.py} $(1)/usr/bin/
endef

$(eval $(call BuildPackage,splashutils))
$(eval $(call BuildPackage,libsplashutils))
$(eval $(call BuildPackage,splashutils-scripts))
