include $(TOPDIR)/rules.mk

PKG_NAME:=libuargp

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/xhebox/libuargp.git
PKG_SOURCE_DATE:=2023-03-12
PKG_SOURCE_VERSION:=1f92296a97ef8fcfeb5e2440f5a62081fe654f75
PKG_MIRROR_HASH:=9dee0936cf370ccd0df84f257a14ef0ffa6ef854a426bd431b557cb7fb6f3251

PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>
PKG_LICENSE:=LGPL-2.1
PKG_LICENSE_FILES:=COPYING

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/libuargp
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=another choice for old argp-standalone
  URL:=https://github.com/xhebox/libuargp
endef

define Package/libuargp/description
  extracted from uclibc-ng, so this project
  is to provide another choice than the old
  argp-standalone.
endef

MAKE_INSTALL_FLAGS += \
	prefix=/usr

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/include/argp.h $(1)/usr/include/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libargp.a $(1)/usr/lib/
endef

define Package/libuargp/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libargp.so $(1)/usr/lib/
	$(LN) libargp.so $(1)/usr/lib/libargp.so.0
endef

$(eval $(call BuildPackage,libuargp))
