include $(TOPDIR)/rules.mk

PKG_NAME:=ext4magic
PKG_VERSION:=0.3.2
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@SF/ext4magic
PKG_HASH:=8d9c6a594f212aecf4eb5410d277caeaea3adc03d35378257dfd017ef20ea115

PKG_INSTALL:=1
PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>
PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=COPYING

include $(INCLUDE_DIR)/package.mk

define Package/ext4magic
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=Filesystem
  TITLE:=ext4 recovery util
  DEPENDS:=+libblkid +libbz2 +libcomerr +libe2p +libext2fs +libmagic +libuuid +zlib
  URL:=https://sourceforge.net/projects/ext4magic/
  MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>
endef

define Package/ext4magic/description
  Recover deleted or overwritten files on ext3 and ext4 filesystems
endef

CONFIGURE_ARGS += --enable-expert-mode --enable-file-attr

define Package/ext4magic/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/ext4magic $(1)/usr/sbin
endef

$(eval $(call BuildPackage,ext4magic))
