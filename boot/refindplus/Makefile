#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=refindplus
PKG_VERSION:=0.13.3.AC
PKG_RELEASE:=1

PKG_SOURCE:=RefindPlus_x64-001303_AC.zip
PKG_SOURCE_URL:=https://github.com/dakanji/RefindPlus/releases/download/v0.13.3.AC
PKG_HASH:=skip
PKG_BUILD_DIR:=$(BUILD_DIR)/RefindPlus_x64-001303_AC

PKG_LICENSE:=GPL-3.0-only
PKG_LICENSE_FILES:=LICENSE.txt
PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=0

PKG_CONFIG_DEPENDS := \
	REFINDPLUS_RELEASE \
	REFINDPLUS_DEBUG

include $(INCLUDE_DIR)/package.mk

define Package/refindplus/config
  source "$(SOURCE)/Config.in"
endef

define Package/refindplus
  SECTION:=boot
  CATEGORY:=Boot Loaders
  TITLE:=A variant of the rEFInd boot manager
  DEPENDS:=@TARGET_x86_64
  URL:=https://github.com/dakanji/RefindPlus
endef

define Package/refindplus/description
  RefindPlus is a variant of the rEFInd Boot Manager incorporating various fixes and additional features.

  The main development focus is on the following units:

    - MacPro3,1: Early 2008 Mac Pro
    - MacPro4,1: Early 2009 Mac Pro
    - MacPro5,1: Mid 2010 and Mid 2012 Mac Pros
    - Xserve2,1: Early 2008 Xserve
    - Xserve3,1: Early 2009 Xserve

  However, the enhancements added to RefindPlus are not limited in scope to those units and include
  several UEFI-PC and other Apple Mac related enhancements that may be of interest to anyone
  requiring a capable and flexible boot manager.

  NOTE: This package does not come with install scripts.
endef

define Download/refind-conf
  URL_FILE:=refind-bin-0.13.3.1.zip
  URL:=https://downloads.sourceforge.net/project/refind/0.13.3.1/
  FILE:=refind-conf-0.13.3.1.zip
  HASH:=44a981538cf06e683bc095e11694ce8508826fcc6ca6229e116938575e7c4c54
endef

define Build/Prepare
	$(call Build/Prepare/Default)
	$(eval $(call Download,refind-conf))
	unzip -q -d $(PKG_BUILD_DIR) $(DL_DIR)/refind-conf-0.13.3.1.zip
endef

define Build/Configure
endef

define Build/Compile
endef

define Build/Install
endef

define Package/refindplus/install
	$(INSTALL_DIR) \
		$(1)/usr/share/refindplus \
		$(1)/usr/share/refindplus/drivers_x64 \
		$(1)/usr/share/refindplus/tools_x64 \
		$(1)/usr/share/refindplus/fonts \
		$(1)/usr/share/refindplus/icons \
		$(1)/usr/share/refindplus/banners \
		$(1)/usr/share/refindplus/keys		

	$(INSTALL_BIN) $(PKG_BUILD_DIR)/RefindPlus_x64-$(if $(CONFIG_REFINDPLUS_DEBUG),DBG,REL).efi $(1)/usr/share/refindplus/refindplus_x64.efi
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/OtherBinaries/*.efi $(1)/usr/share/refindplus/drivers_x64/

	$(INSTALL_DATA) $(PKG_BUILD_DIR)/config.conf $(1)/usr/share/refindplus/refind.conf-sample
	$(INSTALL_DATA) ./files/refind_linux.conf $(1)/usr/share/refindplus/

	$(INSTALL_DATA) $(PKG_BUILD_DIR)/refind-bin-0.13.3.1/fonts/*.png $(1)/usr/share/refindplus/fonts/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/refind-bin-0.13.3.1/refind/icons/*.png $(1)/usr/share/refindplus/icons/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/refind-bin-0.13.3.1/banners/*.{png,svg} $(1)/usr/share/refindplus/banners/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/refind-bin-0.13.3.1/keys/*.{cer,crt} $(1)/usr/share/refindplus/keys/
endef

$(eval $(call BuildPackage,refindplus))
