include $(TOPDIR)/rules.mk

PKG_NAME:=testdisk
PKG_VERSION:=7.1
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=https://www.cgsecurity.org
PKG_HASH:=1413c47569e48c5b22653b943d48136cb228abcbd6f03da109c4df63382190fe

PKG_INSTALL:=1
PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

include $(INCLUDE_DIR)/package.mk

define Package/testdisk
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=Filesystem
  TITLE:=testdisk file recovery util
  DEPENDS:=+libcomerr +libext2fs +libjpeg-turbo +libncurses +libuuid +sudo +zlib
  URL:=https://www.cgsecurity.org/index.html?testdisk.html
  MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>
endef

define Package/testdisk/description
  Checks and undeletes partitions + PhotoRec, signature based recovery tool
endef

CONFIGURE_ARGS += -without-ntfs --without-ntfs3g --without-reiserfs --with-sudo-bin=/usr/bin/sudo

define Package/testdisk/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/{fidentify,photorec,testdisk} $(1)/usr/bin
endef

$(eval $(call BuildPackage,testdisk))
