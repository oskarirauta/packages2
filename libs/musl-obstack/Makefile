include $(TOPDIR)/rules.mk

PKG_NAME:=musl-obstack

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/void-linux/musl-obstack.git
PKG_SOURCE_DATE:=2023-02-28
PKG_SOURCE_VERSION:=f4385255be1615688c6a5f042277304d7ab288b1
PKG_MIRROR_HASH:=8b4839f02daf30566b814e38c84b17186e6bff3f07bd002c4d3d4cdf6e232d01

PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>
PKG_LICENSE_FILES:=COPYING

PKG_BUILD_PARALLEL:=1
PKG_FIXUP:=autoreconf
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/musl-obstack
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=musl obstack functions and macros
  URL:=https://github.com/void-linux/musl-obstack
endef

define Package/musl-obstack/description
  The musl-obstack package is a copy + paste of the obstack functions and
  macros found in GNU gcc libiberty library for use with musl libc.
endef

MAKE_INSTALL_FLAGS += \
	prefix=/usr

define Build/Configure
	cd $(PKG_BUILD_DIR) && ./bootstrap.sh
	$(Build/Configure/Default)
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include $(1)/usr/lib $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/include/obstack.h $(1)/usr/include/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/musl-obstack.pc $(1)/usr/lib/pkgconfig/

	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libobstack.a $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libobstack.la $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libobstack.so.1.0.0 $(1)/usr/lib/
	$(LN) libobstack.so.1.0.0 $(1)/usr/lib/libobstack.so.1
	$(LN) libobstack.so.1.0.0 $(1)/usr/lib/libobstack.so
endef

define Package/musl-obstack/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libobstack.so.1.0.0 $(1)/usr/lib/
	$(LN) libobstack.so.1.0.0 $(1)/usr/lib/libobstack.so.1
	$(LN) libobstack.so.1.0.0 $(1)/usr/lib/libobstack.so
endef

$(eval $(call BuildPackage,musl-obstack))
