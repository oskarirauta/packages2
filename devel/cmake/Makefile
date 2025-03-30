#
# Copyright (C) 2006-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=cmake
PKG_VERSION:=3.31.4
PKG_VERSION_MAJOR:=$(word 1,$(subst ., ,$(PKG_VERSION))).$(word 2,$(subst ., ,$(PKG_VERSION)))
PKG_RELEASE:=1
PKG_CPE_ID:=cpe:/a:kitware:cmake

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/Kitware/CMake/releases/download/v$(PKG_VERSION)/ \
		https://cmake.org/files/v$(PKG_VERSION_MAJOR)/
PKG_HASH:=a6130bfe75f5ba5c73e672e34359f7c0a1931521957e8393a5c2922c8b0f7f25

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/cmake
  SECTION:=devel
  CATEGORY:=Development
  TITLE:=cmake
  DEPENDS:=@TARGET_x86_64 +libstdcpp +libncurses +libopenssl +libidn2
endef

define Package/cmake/description
  CMake is an open-source, cross-platform family of
  tools designed to build, test and package software.
endef

CONFIGURE_ARGS := \
	--no-debugger \
	$(if $(MAKE_JOBSERVER),--parallel="$(MAKE_JOBSERVER)") \
	--prefix="$(STAGING_DIR_HOST)" \
	--system-expat \
	--system-liblzma \
	--system-zlib \
	--system-zstd \
	--generator=Ninja

define Package/cmake/install
	$(INSTALL_DIR) $(1)/usr/bin $(1)/usr/share/aclocal $(1)/usr/share/cmake-$(PKG_VERSION_MAJOR)
	$(INSTALL_DIR) $(1)/usr/share/cmake-$(PKG_VERSION_MAJOR)/{Help,Modules,Templates,include}
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/{ccmake,cmake,cpack,ctest} $(1)/usr/bin/
	$(CP) $(PKG_INSTALL_DIR)/usr/share/aclocal/cmake.m4 $(1)/usr/share/aclocal/
	$(CP) $(PKG_INSTALL_DIR)/usr/share/cmake-$(PKG_VERSION_MAJOR)/Help/** $(1)/usr/share/cmake-$(PKG_VERSION_MAJOR)/Help/
	$(CP) $(PKG_INSTALL_DIR)/usr/share/cmake-$(PKG_VERSION_MAJOR)/Modules/** $(1)/usr/share/cmake-$(PKG_VERSION_MAJOR)/Modules/
	$(CP) $(PKG_INSTALL_DIR)/usr/share/cmake-$(PKG_VERSION_MAJOR)/Templates/** $(1)/usr/share/cmake-$(PKG_VERSION_MAJOR)/Templates/
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/share/cmake-$(PKG_VERSION_MAJOR)/include/** $(1)/usr/share/cmake-$(PKG_VERSION_MAJOR)/include/
endef

$(eval $(call BuildPackage,cmake))
