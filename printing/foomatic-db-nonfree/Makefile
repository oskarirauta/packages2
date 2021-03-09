#
# Copyright (C) 2006-2016 OpenWrt.org
# Copyright (C) 2016 lede-project.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=foomatic-db-nonfree
PKG_VERSION:=4.0
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/OpenPrinting/$(PKG_NAME).git
PKG_SOURCE_DATE:=20210308
PKG_SOURCE_VERSION:=6ddae02ac89240c019f8b5026cfe70e30fd2b3db
PKG_MIRROR_HASH:=7a261006d6cf7c39d602a528d8a27882a670a8e275ad53659841c5feb16dfa7a

include $(INCLUDE_DIR)/package.mk

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=0
PKG_LICENSE:=GPL
PKG_FIXUP:=autoreconf

define Package/foomatic-db-nonfree
  SECTION:print
  CATEGORY:=Printing
  SUBMENU:=Drivers
  URL:=http://www.linuxprinting.org/foomatic.html
  DEPENDS:=
  TITLE:=foomatic-db-nonfree
endef

define Package/foomatic-db-nonfree/description
  Foomatic database - Non-free drivers
endef

define Package/foomatic-db-nonfree-ppds
  SECTION:=print
  CATEGORY:=Printing
  SUBMENU:=Drivers
  URL:=http://www.linuxprinting.org/foomatic.html
  DEPENDS:=
  TITLE:=foomatic-db-nonfree-ppds
endef

define Package/foomatic-db-nonfree-ppds/description
  foomatic database non-free ppds files
endef

TARGET_LDFLAGS += -Wl,-rpath-link=$(STAGING_DIR)/usr/lib
Hooks/Configure/Pre += autoreconf_target

define Package/foomatic-db-nonfree/install
	$(INSTALL_DIR) $(1)/usr/share/foomatic/db/source/driver
	$(INSTALL_CONF) $(PKG_INSTALL_DIR)/usr/share/foomatic/db/source/driver/*.xml \
		$(1)/usr/share/foomatic/db/source/driver/
endef

define Package/foomatic-db-nonfree-ppds/install
	$(INSTALL_DIR) $(1)/usr/share/foomatic/db/source/PPD/{Dell,Genicom,Lexmark,Oce,Xerox}
	$(INSTALL_DIR) \
		$(1)/usr/share/cups/model/foomatic-db-ppds \
		$(1)/usr/share/foomatic/db/source/PPD/Lexmark/PS \
		$(1)/usr/share/foomatic/db/source/PPD/Oce/Others
	$(INSTALL_CONF) $(PKG_INSTALL_DIR)/usr/share/foomatic/db/source/PPD/Dell/*.ppd \
		$(1)/usr/share/foomatic/db/source/PPD/Dell/
	$(INSTALL_CONF) $(PKG_INSTALL_DIR)/usr/share/foomatic/db/source/PPD/Genicom/*.PPD \
		$(1)/usr/share/foomatic/db/source/PPD/Genicom/
	$(INSTALL_CONF) $(PKG_INSTALL_DIR)/usr/share/foomatic/db/source/PPD/Lexmark/PS/*.ppd \
		$(1)/usr/share/foomatic/db/source/PPD/Lexmark/PS/
	$(INSTALL_CONF) $(PKG_INSTALL_DIR)/usr/share/foomatic/db/source/PPD/Oce/Others/*.ppd \
		$(1)/usr/share/foomatic/db/source/PPD/Oce/Others/
	$(INSTALL_CONF) $(PKG_INSTALL_DIR)/usr/share/foomatic/db/source/PPD/Xerox/*.ppd \
		$(1)/usr/share/foomatic/db/source/PPD/Xerox/
endef

$(eval $(call BuildPackage,foomatic-db-nonfree))
$(eval $(call BuildPackage,foomatic-db-nonfree-ppds))
