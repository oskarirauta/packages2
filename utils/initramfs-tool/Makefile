include $(TOPDIR)/rules.mk

PKG_NAME:=initramfs-tool
PKG_VERSION:=20230310
PKG_RELEASE:=1

PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>

include $(INCLUDE_DIR)/package.mk

define Package/initramfs-tool
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=initramfs utility
  PKGARCH:=all
endef

define Package/initramfs-tool/description
  A small script that can be used for extracting, creating and
  even building a template of new initramfs file.
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/initramfs-tool/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) ./files/initramfs.sh $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,initramfs-tool))
