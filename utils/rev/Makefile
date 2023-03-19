include $(TOPDIR)/rules.mk

PKG_NAME:=rev
PKG_VERSION:=20230319
PKG_RELEASE:=1

PKG_BUILD_DEPENDS:=util-linux
PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>

include $(INCLUDE_DIR)/package.mk

define Package/rev
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=utility to reverse lines of file
endef

define Package/rev/description
  The rev utility copies the specified files to the standard output,
  reversing the order of characters in everyline.
  If no files are specified, the standard input is read.
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/rev/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(find $(BUILD_DIR)/util-linux-* -wholename \*ipkg-install/usr/bin/rev) $(1)/usr/bin/
endef

$(eval $(call BuildPackage,rev))
