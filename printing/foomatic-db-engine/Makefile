#
# Copyright (C) 2006-2016 OpenWrt.org
# Copyright (C) 2016 lede-project.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=foomatic-db-engine
PKG_VERSION:=4.0
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/OpenPrinting/foomatic-db-engine-4.git
PKG_SOURCE_DATE:=20210308
PKG_SOURCE_VERSION:=bd265b77a9f66f672bf1e3f0803145f2eccabf06
PKG_MIRROR_HASH:=0a6d88912c2d5734b8a9be139dc50322b4a43592baaddbf71992d21103f4c62e

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1

define Package/foomatic-db-engine
  SECTION:=print
  CATEGORY:=Printing
  DEPENDS:=$(ICONV_DEPENDS) +zlib +libxml2
  URL:=https://github.com/OpenPrinting/foomatic-db-engine-4
  TITLE:=foomatic-db-engine
endef

define Package/foomatic-db-engine/description
  Foomatic's database engine generates PPD files from the data in
  Foomatic's XML database. It also contains scripts to directly generate
  print queues and handle jobs.
endef

define Package/foomatic-db-engine/conffiles
/etc/foomatic/defaultspooler
endef

TARGET_LDFLAGS+=-Wl,-rpath-link=$(STAGING_DIR)/usr/lib -L$(STAGING_DIR)/usr/lib/libiconv-full/lib
Hooks/Configure/Pre += autoreconf_target

CONFIGURE_ARGS += \
		--bindir=/usr/bin \
		--sbindir=/usr/bin \
		--libexecdir=/usr/lib \
		--sysconfdir=/etc \
		--sharedstatedir=/usr/share \
		--localstatedir=/var \
		--libdir=/usr/lib \
		--datarootdir=/usr/share
		
define Package/foomatic-db-engine/install

	$(INSTALL_DIR) \
		$(1)/etc/foomatic \
		$(1)/usr/lib/cups/driver \
		$(1)/usr/lib/perl5 \
		$(1)/usr/bin \
		$(1)/usr/share/foomatic/templates \
		$(1)/usr/share/perl5/site_perl/Foomatic

	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/foomatic-* $(1)/usr/bin/
	$(LN) /usr/bin/foomatic-ppdfile $(1)/usr/lib/cups/driver/foomatic
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/perl5/** $(1)/usr/lib/perl5/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/foomatic-* $(1)/usr/bin/
	$(INSTALL_CONF) $(PKG_INSTALL_DIR)/usr/share/foomatic/templates/*.xml \
		$(1)/usr/share/foomatic/templates/
	$(INSTALL_CONF) $(PKG_INSTALL_DIR)/usr/share/perl5/site_perl/Foomatic/*.pm \
		$(1)/usr/share/perl5/site_perl/Foomatic/
	$(INSTALL_CONF) ./files/defaultspooler $(1)/etc/foomatic/	
endef

$(eval $(call BuildPackage,foomatic-db-engine))
