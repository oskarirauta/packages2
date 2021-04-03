#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libapparmor
PKG_VERSION:=2.13.4
PKG_RELEASE:=1

#PKG_SOURCE:=$(PKG_NAME)-v$(PKG_VERSION).tar.gz
#PKG_SOURCE_URL:=https://gitlab.com/apparmor/apparmor/-/archive/v$$(PKG_VERSION)/
#PKG_HASH:=1cc7861fc5d913abe170685ad46e1f1ad5258b77a68418f6f7346936e95bba2e
PKG_BUILD_DIR:=$(BUILD_DIR)/apparmor-v2.13.4-df0ac742f7a1146181d8734d03334494f2015134/libraries/libapparmor
#HOST_BUILD_DEPENDS:=libsepol/host pcre/host

#PKG_LICENSE:=libselinux-1.0
PKG_BUILD_DEPEND:=apparmor
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>
PKG_FIXUP:=autoreconf

#CONFIGURE_PATH:=libraries/libapparmor

include $(INCLUDE_DIR)/package.mk

define Package/libapparmor
  TITLE:=libapparmor
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:=
endef

#MAKE_FLAGS+= --without-python --with-perl

define Package/libapparmor/description
	libapparmor
endef

define Build/Prepare
	# package app armor has done this already..
endef

#define Build/Configure
#	cd $(PKG_BUILD_DIR)/libraries
#endef

#define Build/Compile
#	$(call Build/Compile/Default,all)
#endef

#define Build/Install
#	$(call Build/Install/Default,install)
#endef

#define Build/InstallDev
#	$(INSTALL_DIR) $(1)/usr/include
#	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include/
#	$(INSTALL_DIR) $(1)/usr/lib
#	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib/
#	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
#	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/libselinux.pc $(1)/usr/lib/pkgconfig/
#	$(SED) 's,/usr/include,$$$${prefix}/include,g' $(1)/usr/lib/pkgconfig/libselinux.pc
#	$(SED) 's,/usr/lib,$$$${exec_prefix}/lib,g' $(1)/usr/lib/pkgconfig/libselinux.pc
#endef

define Package/libapparmor/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libapparmor.so.* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libapparmor))
