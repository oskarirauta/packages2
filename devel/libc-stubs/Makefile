include $(TOPDIR)/rules.mk

PKG_NAME:=libc-stubs
PKG_VERSION:=20230222
PKG_RELEASE:=1

PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>

include $(INCLUDE_DIR)/package.mk

define Package/libc-stubs/Default
endef

define Package/libc-stubs
	SECTION:=devel
	CATEGORY:=Development
	SUBMENU:=Libraries
	TITLE:=Missing libc library stubs
	PKGARCH:=all
endef

define Package/libc-stubs/description
  MUSL does not have libpthread, libdl or librt independently, but
  instead files are provided as empty archives; stubs.
  Usually only developers need these stubs, or anyone compiling something
  that requires these files, so openwrt does not ship with them.
  This package adds those stubs to system.
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/libc-stubs/install
	$(INSTALL_DIR) $(1)/lib
	$(INSTALL_DATA) ./files/stub.a $(1)/lib/libpthread.a
	$(INSTALL_DATA) ./files/stub.a $(1)/lib/libdl.a
	$(INSTALL_DATA) ./files/stub.a $(1)/lib/librt.a
	$(INSTALL_DATA) ./files/stub.a $(1)/lib/libresolv.a
endef

$(eval $(call BuildPackage,libc-stubs))
