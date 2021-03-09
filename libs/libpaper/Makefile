#
# Copyright (C) 2006-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libpaper
PKG_VERSION:=1.1.28
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)_$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://ftp.debian.org/debian/pool/main/libp/libpaper
#PKG_HASH:=5048f1c8fc509cc636c2f97f4b40c293338b6041a5652082d5ee2cf54b530c56
PKG_HASH:=skip

PKG_LICENSE:=LGPL-2.0
PKG_LICENSE_FILES:=COPYING

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/libpaper
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Library for handling paper characteristics
  URL:=https://packages.debian.org/unstable/source/libpaper
endef

define Package/libpaper/description
  Library for handling paper characteristics.
endef

CONFIGURE_ARGS+= \
	--enable-shared \
	--enable-static \
	--disable-rpath \
	--disable-nls \
	--without-libiconv-prefix \
	--without-libintl-prefix

TARGET_CFLAGS += $(FPIC)

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/include/*.h $(1)/usr/include/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libpaper*.{a,so*} $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) ./files/libpaper.pc $(1)/usr/lib/pkgconfig/
endef

define Package/libpaper/install
	$(INSTALL_DIR) $(1)/usr/bin $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/paperconf $(1)/usr/bin/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libpaper*.so.* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libpaper))
