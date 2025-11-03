include $(TOPDIR)/rules.mk

PKG_NAME:=intltool
PKG_VERSION:=0.51.0
PKG_RELEASE:=3

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://launchpad.net/intltool/trunk/$(PKG_VERSION)/+download
PKG_HASH:=67c74d94196b153b774ab9f89b2fa6c6ba79352407037c8c14d5aeb334e959cd
PKG_MAINTAINER:=Ted Hess <thess@kitschensync.net>

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/intltool
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=set of tools to centralize translation using GNU gettext
  URL:=https://www.freedesktop.org/wiki/Software/intltool
endef

define Package/intltool/description
  intltool is a set of tools to centralize translation of many different
  file formats using GNU gettext-compatible PO files.
endef

define Package/intltool/install
	$(INSTALL_DIR) $(1)/usr/bin $(1)/usr/share/aclocal
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/intltool-extract $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/intltool-merge $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/intltool-prepare $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/intltool-update $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/intltoolize $(1)/usr/bin/
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/share/aclocal/intltool.m4 $(1)/usr/share/aclocal/
endef

$(eval $(call BuildPackage,intltool))
