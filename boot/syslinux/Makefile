include $(TOPDIR)/rules.mk

PKG_NAME:=syslinux
PKG_VERSION:=6.04
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-x86_64-precompiled.tar.gz
PKG_SOURCE_URL:=https://github.com/oskarirauta/owrt-packages2/raw/main/boot/syslinux/files
PKG_HASH:=43d966c80cd1b25f2ce59e594a8f71dabe67d51dad0149cc8831206871446014

PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>
PKG_LICENSE:=GPL-2.0-or-later
PKG_LICENSE_FILES:=COPYING
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/syslinux
  $(call Package/syslinux/Default)
  SECTION:=boot
  CATEGORY:=Boot Loaders
  TITLE:=syslinux
  DEPENDS:=@TARGET_x86_64 +libuuid
endef

define Package/syslinux/description
  Boot loader for the Linux operating system
endef

STRIP:=echo "Not stripping syslinux."
RSTRIP:=echo "Not stripping syslinux."

define Build/Configure
	# Nothing to configure
endef

define Build/Compile
	# Nothing to build
endef

define Build/Install
	# Nothing to install
endef

define Package/syslinux/install
	$(INSTALL_DIR) $(1)/usr/bin $(1)/usr/lib/syslinux
	$(INSTALL_DIR) $(1)/usr/lib/syslinux/{bios,config,diag,efi32,efi64}
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/** $(1)/usr/bin/
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/syslinux/bios/** $(1)/usr/lib/syslinux/bios/
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/syslinux/config/openwrt.png $(1)/usr/lib/syslinux/config/
	$(INSTALL_CONF) $(PKG_INSTALL_DIR)/usr/lib/syslinux/config/*.cfg $(1)/usr/lib/syslinux/config/
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/syslinux/diag/** $(1)/usr/lib/syslinux/diag/
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/syslinux/efi32/*.c32 $(1)/usr/lib/syslinux/efi32/
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/syslinux/efi32/ldlinux.e32 $(1)/usr/lib/syslinux/efi32/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/syslinux/efi32/syslinux.efi $(1)/usr/lib/syslinux/efi32/
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/syslinux/efi64/*.c32 $(1)/usr/lib/syslinux/efi64/
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/syslinux/efi64/ldlinux.e64 $(1)/usr/lib/syslinux/efi64/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/syslinux/efi64/syslinux.efi $(1)/usr/lib/syslinux/efi64/
endef

$(eval $(call BuildPackage,syslinux))
