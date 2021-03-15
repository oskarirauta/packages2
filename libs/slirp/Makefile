#
# Copyright (C) 2006-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=slirp
PKG_VERSION:=4.4.0
PKG_RELEASE:=1

PKG_SOURCE:=lib$(PKG_NAME)-v$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://gitlab.freedesktop.org/slirp/libslirp/-/archive/v$(PKG_VERSION)
PKG_HASH:=43513390c57bee8c23b31545bfcb765200fccf859062b1c8101e72befdabce2e

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1
PKG_BUILD_DEPENDS:=meson/host
PKG_BUILD_DIR=$(BUILD_DIR)/lib$(PKG_NAME)-v$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk
include ../../devel/meson/meson.mk

define Package/slirp
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=slirp
  DEPENDS:= +glib2
  URL:=https://gitlab.freedesktop.org/slirp/libslirp
endef

define Package/slirp/description
	A general purpose TCP-IP emulator
endef

define Package/slirp/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libslirp.so $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libslirp.so.* $(1)/usr/lib/
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib $(1)/usr/include/slirp $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libslirp.so $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libslirp.so.* $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/include/slirp/** $(1)/usr/include/slirp/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/** $(1)/usr/lib/pkgconfig/
endef

$(eval $(call BuildPackage,slirp))
