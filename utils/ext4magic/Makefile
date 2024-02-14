include $(TOPDIR)/rules.mk

PKG_NAME:=ext4magic
PKG_VERSION:=0.3.2
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL_FILE:=Release-beta-0-3-2.tar.gz
PKG_SOURCE_URL:=https://github.com/gktrk/ext4magic/archive/refs/tags/
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-Release-beta-0-3-2
PKG_HASH:=915c306b4b7602199ece3f0c9213dc77b94101bdab682ab7088e9705f6641a27

PKG_INSTALL:=1
PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

include $(INCLUDE_DIR)/package.mk

define Package/ext4magic
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=Filesystem
  TITLE:=ext4 recovery util
  DEPENDS:=+libblkid +libbz2 +libcomerr +libe2p +libext2fs +libmagic +libuuid +zlib
  MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>
endef

define Package/ext4magic/description
  ext4 fs recovery tool
endef

CONFIGURE_ARGS += --enable-expert-mode --enable-file-attr

define Package/ext4magic/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/ext4magic $(1)/usr/sbin
endef

$(eval $(call BuildPackage,ext4magic))
