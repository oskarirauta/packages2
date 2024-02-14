include $(TOPDIR)/rules.mk

PKG_NAME:=extundelete
PKG_VERSION:=0.2.4
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=@SF/$(PKG_NAME)
PKG_HASH:=a1f9dd61247056d36401ce5d6785e74d08a184340eebd3865c345ddaa93f19f4

PKG_INSTALL:=1
PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

include $(INCLUDE_DIR)/package.mk

define Package/extundelete
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=Filesystem
  TITLE:=undelete for ext3/ext4
  DEPENDS:=+libcomerr +libext2fs +libstdcpp
  MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>
endef

define Package/extundelete/description
  Extundelete is a utility to undelete files from an ext3 or ext4 partition.
endef

define Package/extundelete/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/extundelete $(1)/usr/sbin
endef

$(eval $(call BuildPackage,extundelete))
