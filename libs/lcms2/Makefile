#
# Copyright (C) 2007-2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=lcms2
PKG_VERSION:=2.12
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@SF/lcms
PKG_HASH:=18663985e864100455ac3e507625c438c3710354d85e5cbb7cd4043e11fe10f5

PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/lcms2
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Little CMS color engine
  DEPENDS:=+libjpeg +zlib
  URL:=http://www.littlecms.com/
  SUBMENU:=Printing
endef

define Package/lcms2/description
	Little CMS color engine
endef

CONFIGURE_ARGS+= \
	--without-tiff \
	--with-fastfloat

define Package/lcms2/install
	$(INSTALL_DIR) $(1)/usr/bin $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/liblcms2*.so* $(1)/usr/lib/
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include $(1)/usr/lib $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/liblcms2*.so* $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/liblcms2*.a $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/liblcms2*.la $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/lcms2.pc $(1)/usr/lib/pkgconfig/
endef

$(eval $(call BuildPackage,lcms2))
