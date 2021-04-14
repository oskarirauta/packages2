include $(TOPDIR)/rules.mk

PKG_NAME:=parted
PKG_VERSION:=3.4
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=http://ftp.gnu.org/gnu/parted/
PKG_HASH:=skip

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>

include $(INCLUDE_DIR)/package.mk

define Package/parted
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=parted
  SUBMENU:=Disc
  DEPENDS:=+libblkid +libreadline +libncurses +libdevmapper +libuuid
endef

define Package/parted/description
  GNU parted
endef

define Package/parted/install
	$(INSTALL_DIR) $(1)/usr/lib $(1)/sbin
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.so** $(1)/usr/lib/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/parted $(1)/sbin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/partprobe $(1)/sbin/
endef

$(eval $(call BuildPackage,parted))
