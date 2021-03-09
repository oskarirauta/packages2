#
# Copyright (C) 2006-2016 OpenWrt.org
# Copyright (C) 2016 lede-project.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=foomatic-db
PKG_VERSION:=4.0
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/OpenPrinting/$(PKG_NAME).git
PKG_SOURCE_DATE:=20210308
PKG_SOURCE_VERSION:=f9ad1f0a98cd7e71a31744464dffb9563ffe3e97
PKG_MIRROR_HASH:=8151c6ae998bbd98c50c8b9d489e33c7319893fb1095b92f71c328cf43fdfa10

include $(INCLUDE_DIR)/package.mk

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=0
PKG_LICENSE:=GPL
PKG_FIXUP:=autoreconf

define Package/foomatic-db
  SECTION:=print
  CATEGORY:=Printing
  SUBMENU:=Drivers
  URL:=http://www.linuxprinting.org/foomatic.html
  DEPENDS:=
  TITLE:=foomatic-db
endef

define Package/foomatic-db/description
  Foomatic database
endef

define Package/foomatic-db-ppds
  SECTION:=print
  CATEGORY:=Printing
  SUBMENU:=Drivers
  URL:=http://www.linuxprinting.org/foomatic.html
  DEPENDS:=
  TITLE:=foomatic-db-ppds
endef

define Package/foomatic-db-ppds/description
  foomatic database ppds files
endef

TARGET_LDFLAGS += -Wl,-rpath-link=$(STAGING_DIR)/usr/lib
Hooks/Configure/Pre += autoreconf_target

define Package/foomatic-db/install
	$(INSTALL_DIR) $(1)/usr/share/foomatic/db/source
	$(INSTALL_CONF) $(PKG_INSTALL_DIR)/usr/share/foomatic/db/oldprinterids $(1)/usr/share/foomatic/db/

	$(INSTALL_DIR) $(1)/usr/share/foomatic/db/source/{driver,opt,printer}
	$(INSTALL_CONF) $(PKG_INSTALL_DIR)/usr/share/foomatic/db/source/driver/*.xml \
		$(1)/usr/share/foomatic/db/source/driver/
	$(INSTALL_CONF) $(PKG_INSTALL_DIR)/usr/share/foomatic/db/source/opt/*.xml \
		$(1)/usr/share/foomatic/db/source/opt/
	$(INSTALL_CONF) $(PKG_INSTALL_DIR)/usr/share/foomatic/db/source/printer/*.xml \
		$(1)/usr/share/foomatic/db/source/printer/
	
	$(INSTALL_DIR) $(1)/usr/share/foomatic/xmlschema
	$(INSTALL_CONF) $(PKG_INSTALL_DIR)/usr/share/foomatic/xmlschema/*.xsd \
		$(1)/usr/share/foomatic/xmlschema/
endef

define Package/foomatic-db-ppds/install
	$(INSTALL_DIR) $(1)/usr/share/foomatic/db/source/PPD $(1)/usr/share/cups/model/foomatic-db-ppds
	$(CP) -r $(PKG_INSTALL_DIR)/usr/share/foomatic/db/source/PPD/** \
		$(1)/usr/share/foomatic/db/source/PPD/
endef

$(eval $(call BuildPackage,foomatic-db))
$(eval $(call BuildPackage,foomatic-db-ppds))
