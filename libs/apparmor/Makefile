#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libapparmor
PKG_VERSION:=2.13.4
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-v$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://gitlab.com/apparmor/apparmor/-/archive/v$$(PKG_VERSION)/
PKG_HASH:=1cc7861fc5d913abe170685ad46e1f1ad5258b77a68418f6f7346936e95bba2e
PKG_BUILD_DIR:=$(BUILD_DIR)/apparmor-v2.13.4-df0ac742f7a1146181d8734d03334494f2015134

PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>
#BUILDONLY:=1

include $(INCLUDE_DIR)/package.mk

define Package/apparmor
  TITLE:=apparmor
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:=
endef

define Package/libapparmor/description
	libapparmor
endef

define Build/Configure
	# nothing to do
endef

define Build/Compile
	# nothing to do
endef

define Build/Install
	# nothing to do
endef

define Package/apparmor/install
	# nothign to do
endef

$(eval $(call BuildPackage,apparmor))
