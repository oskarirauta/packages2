
include $(TOPDIR)/rules.mk

PKG_NAME:=cmake
PKG_VERSION:=3.20.0
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-x86_64-precompiled.tar.gz
PKG_SOURCE_URL:=https://github.com/oskarirauta/owrt-packages2/raw/main/devel/cmake/files
PKG_HASH:=c596094a439cc28e595e89276fd6d77e268d815cb921abf99ab1bbdeb7a54663

PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>
PKG_LICENSE:=LGPL-2.1-or-later
PKG_LICENSE_FILES:= Copyright.txt LGPLv2.1.txt LGPLv3.txt
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
	$(INSTALL_DIR) $(1)/usr/bin $(1)/usr/lib/cmake
	$(INSTALL_DIR) $(1)/usr/lib/cmake/{Help,Modules,Templates,include}
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/** $(1)/usr/bin/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/cmake/Help/** $(1)/usr/lib/cmake/Help/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/cmake/Modules/** $(1)/usr/lib/cmake/Modules/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/cmake/Templates/** $(1)/usr/lib/cmake/Templates/
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/cmake/include/** $(1)/usr/lib/cmake/include/
	[ "$(CONFIG_NO_STRIP)" = "y" ] && \
		STRIP=$(STAGING_DIR_HOST)/bin/sstrip $(SCRIPT_DIR)/rstrip.sh $(1)/usr/bin
endef

$(eval $(call BuildPackage,cmake))
