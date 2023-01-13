#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=refind
PKG_VERSION:=0.13.3.1
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=refind-$(if $(CONFIG_REFIND_TIANOCORE),bin,bin-gnuefi)-$(PKG_VERSION).zip
PKG_SOURCE_URL:=https://downloads.sourceforge.net/project/refind/$(PKG_VERSION)
PKG_HASH:=$(if $(CONFIG_REFIND_TIANOCORE),44a981538cf06e683bc095e11694ce8508826fcc6ca6229e116938575e7c4c54,e0e82f53f58e6c0ef769c6563aabe9a984da6482edffc895e5d1bd56aca5f523)
PKG_BUILD_DIR:=$(BUILD_DIR)/refind-bin-$(PKG_VERSION)

PKG_LICENSE:=GPL-3.0-only
PKG_LICENSE_FILES:=LICENSE.txt
PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=0

PKG_CONFIG_DEPENDS := \
	REFIND_TIANOCORE \
	REFIND_GNUEFI \
	REFIND_X64 \
	REFIND_IA32

include $(INCLUDE_DIR)/package.mk

define Package/refind/config
  source "$(SOURCE)/Config.in"
endef

define Package/refind
  SECTION:=boot
  CATEGORY:=Boot Loaders
  TITLE:=rEFInd EFI Boot Manager
  DEPENDS:=@TARGET_x86_64
  URL:=https://www.rodsbooks.com/refind
endef

define Package/refind/description
  rEFInd is a boot manager, meaning that it presents a menu of options to the
  user when the computer first starts up.

  This package contains necessary files for EFI
endef

define Package/refind-scripts
  SECTION:=boot
  CATEGORY:=Boot Loaders
  TITLE:=rEFInd EFI Boot Manager script
  DEPENDS:=@TARGET_x86_64 refind +efibootmgr
  URL:=https://www.rodsbooks.com/refind
endef

define Package/refind-scripts/description
  rEFInd is a boot manager, meaning that it presents a menu of options to the
  user when the computer first starts up.

  This package contains scripts for installing rEFInd
endef

TIANO_ARCH=$(if $(CONFIG_REFIND_X64),x64,ia32)

define Build/Configure
endef

define Build/Compile
endef

define Build/Install
endef

define Package/refind/install
	$(INSTALL_DIR) \
		$(1)/usr/share/refind \
		$(1)/usr/share/refind/drivers_$(TIANO_ARCH) \
		$(1)/usr/share/refind/tools_$(TIANO_ARCH) \
		$(1)/usr/share/refind/fonts \
		$(1)/usr/share/refind/icons \
		$(1)/usr/share/refind/banners \
		$(1)/usr/share/refind/keys		

	$(INSTALL_BIN) $(PKG_BUILD_DIR)/refind/refind_$(TIANO_ARCH).efi $(1)/usr/share/refind/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/refind/drivers_$(TIANO_ARCH)/*.efi $(1)/usr/share/refind/drivers_$(TIANO_ARCH)/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/refind/tools_$(TIANO_ARCH)/*.efi $(1)/usr/share/refind/tools_$(TIANO_ARCH)/

	$(INSTALL_DATA) $(PKG_BUILD_DIR)/refind/refind.conf-sample $(1)/usr/share/refind/
	$(INSTALL_DATA) ./files/refind_linux.conf $(1)/usr/share/refind/

	$(INSTALL_DATA) $(PKG_BUILD_DIR)/fonts/*.png $(1)/usr/share/refind/fonts/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/refind/icons/*.png $(1)/usr/share/refind/icons/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/banners/*.{png,svg} $(1)/usr/share/refind/banners/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/keys/*.{cer,crt} $(1)/usr/share/refind/keys/
endef

define Package/refind-scripts/install
	$(INSTALL_DIR) $(1)/usr/bin

	$(INSTALL_BIN) $(PKG_BUILD_DIR)/refind-install $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/refind-mkdefault $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/mkrlconf $(1)/usr/bin/refind-mkrlconf
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/mvrefind $(1)/usr/bin/refind-mvrefind
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/fonts/mkfont.sh $(1)/usr/bin/refind-mkfont

	$(SED) 's|#!/bin/bash|#!/bin/sh|g' -i $(1)/usr/bin/*
	$(SED) 's|#!/usr/bin/env bash|#!/bin/sh|g' -i $(1)/usr/bin/*
	$(SED) 's|#!/usr/bin/env python3|#!/usr/bin/python3|g' -i $(1)/usr/bin/*
	$(SED) 's|RefindDir=\"\$ThisDir/refind\"|RefindDir=\"/usr/share/refind/\"|g' -i $(1)/usr/bin/refind-install
endef

$(eval $(call BuildPackage,refind))
$(eval $(call BuildPackage,refind-scripts))
