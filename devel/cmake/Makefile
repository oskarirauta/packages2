
include $(TOPDIR)/rules.mk

PKG_NAME:=cmake
PKG_VERSION:=3.27.4
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-x86_64-precompiled.tar.xz
PKG_SOURCE_URL:=https://github.com/oskarirauta/owrt-packages2/raw/main/devel/cmake/files
PKG_HASH:=b96edcd26287dfab62ea3851ebe4ad8daa48f60be27f83cc2c79cd8c6eb0e281

PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>
PKG_LICENSE:=LGPL-2.1-or-later
PKG_LICENSE_FILES:= Copyright.txt Licenses/LGPLv2.1.txt Licenses/LGPLv3.txt
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/cmake
  SECTION:=devel
  CATEGORY:=Development
  TITLE:=cmake
  DEPENDS:=@TARGET_x86_64 +libstdcpp
endef

define Package/cmake/description
  CMake is an open-source, cross-platform family of
  tools designed to build, test and package software.
endef

PKG_SHORT_VERSION:=3.27

define Build/Configure
	# Nothing to configure
endef

define Build/Compile
	# Nothing to build
endef

define Build/Install
	# Nothing to install
endef

define Package/cmake/install
	$(INSTALL_DIR) $(1)/usr/bin $(1)/usr/share/aclocal $(1)/usr/share/cmake-$(PKG_SHORT_VERSION)
	$(INSTALL_DIR) $(1)/usr/lib/cmake-$(PKG_SHORT_VERSION)/{Help,Modules,Templates,include}
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/** $(1)/usr/bin/
	$(CP) $(PKG_INSTALL_DIR)/usr/share/cmake-$(PKG_SHORT_VERSION)/Help/** $(1)/usr/share/cmake-$(PKG_SHORT_VERSION)/Help/
	$(CP) $(PKG_INSTALL_DIR)/usr/share/cmake-$(PKG_SHORT_VERSION)/Modules/** $(1)/usr/share/cmake-$(PKG_SHORT_VERSION)/Modules/
	$(CP) $(PKG_INSTALL_DIR)/usr/share/cmake-$(PKG_SHORT_VERSION)/Templates/** $(1)/usr/share/cmake-$(PKG_SHORT_VERSION)/Templates/
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/share/cmake-$(PKG_SHORT_VERSION)/include/** $(1)/usr/share/cmake-$(PKG_SHORT_VERSION)/include/
endef

$(eval $(call BuildPackage,cmake))
