#
# Copyright (C) 2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=cmake
PKG_VERSION:=3.19.6
PKG_RELEASE:=1

PKG_SOURCE:=cmake-bin-$(PKG_VERSION).tar.gz
PKG_HASH:=8ba4a3d33845b77d452310b2c50135070b53b3f1fe0f090535c68963f12a1152
PKG_CPE_ID:=cpe:/a:kitware:cmake

include $(INCLUDE_DIR)/package.mk

define Package/cmake
  SECTION:=devel
  CATEGORY:=Development
  TITLE:=cmake
  DEPENDS:=@TARGET_x86_64 +libstdcpp
  URL:=https://github.com/Kitware/CMake
endef

define Package/cmake/description
  CMake is a cross-platform, open-source build system generator.
endef

define Build/Prepare
endef

define Build/Compile
	tar xvfz ./files/cmake-bin-$(PKG_VERSION).tar.gz -C $(BUILD_DIR)
endef

define Package/cmake/install
	$(INSTALL_DIR) $(1)/usr/bin $(1)/usr/lib/cmake
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/cmake $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/cpack $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/ctest $(1)/usr/bin/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/cmake/** $(1)/usr/lib/cmake/
endef

$(eval $(call BuildPackage,cmake))
