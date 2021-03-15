#
# Copyright (C) 2006-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=slirp4netns
PKG_VERSION:=1.1.9
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/rootless-containers/slirp4netns/archive/v$(PKG_VERSION)
PKG_HASH:=5ff0d3e4bf6b11c8a4fcf5bd3219b8d52096e3b8cc73ca760aa554bb1eb08768

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1
PKG_FIXUP:=autoreconf

include $(INCLUDE_DIR)/package.mk

define Package/slirp4netns
  SECTION:=net
  CATEGORY:=Network
  TITLE:=slirp4netns
  DEPENDS:=+slirp +libseccomp +glib2
  URL:=https://github.com/rootless-containers/slirp4netns
endef

define Package/slirp4netns/description
	User-mode networking for unprivileged network namespaces
endef

define Package/slirp4netns/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/slirp4netns $(1)/usr/bin/
endef

$(eval $(call BuildPackage,slirp4netns))
