# 
# Copyright (C) 2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=samsung-unified-driver
PKG_VERSION:=1.00.39
PKG_RELEASE:=1

PKG_SOURCE:=uld_V$(PKG_VERSION)_01.17.tar.gz
PKG_SOURCE_URL:=http://downloadcenter.samsung.com/content/DR/201704/20170407143829533/
PKG_MD5SUM:=09c166f9b1e11ff312965886387ee31c6980867c67c3d6206680cbae76ae6746
PKG_BUILD_DIR:=$(BUILD_DIR)/uld

include $(INCLUDE_DIR)/package.mk

define Package/samsung-unified-printer-driver
  SECTION:=print
  CATEGORY:=Printing
  SUBMENU:=Drivers
  TITLE:=Samsung Unified Printer Driver
  URL:=https://www.bchemnet.com/suldr/index.html
  DEPENDS:=@TARGET_x86_64 +libc +libcups +libmath +libstdcpp +libpthread
endef

#libc

define Package/samsung-unified-printer-driver/description
  Unified Linux Driver for Samsung printers.
endef

define Package/samsung-unified-scanner-driver
  SECTION:=print
  CATEGORY:=Printing
  SUBMENU:=Drivers
  TITLE:=Samsung Unified Scanner Driver
  URL:=https://www.bchemnet.com/suldr/index.html
  DEPENDS:=@TARGET_x86_64
endef

define Package/samsung-unified-scanner-driver/description
  Unified Linux Driver for Samsung scanners.
endef

define Build/Compile
endef

define Package/samsung-unified-printer-driver/install
	$(INSTALL_DIR) \
		$(1)/usr/lib \
		$(1)/usr/lib/cups/backend \
		$(1)/usr/lib/cups/filter

	$(INSTALL_BIN) $(PKG_BUILD_DIR)/x86_64/libscmssc.so $(1)/usr/lib/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/x86_64/smfpnetdiscovery $(1)/usr/lib/cups/backend/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/x86_64/{pstosecps,rastertospl} $(1)/usr/lib/cups/filter/
	$(LN) /usr/lib/cups/filter/rastertospl $(1)/usr/lib/cups/filter/rastertosplc

	$(INSTALL_DIR) $(1)/usr/share/ppd/suld	$(1)/usr/share/ppd/suld/cms

	$(INSTALL_CONF) $(PKG_BUILD_DIR)/noarch/share/ppd/*.ppd $(1)/usr/share/ppd/suld/
	$(INSTALL_CONF) $(PKG_BUILD_DIR)/noarch/share/ppd/cms/*.cts $(1)/usr/share/ppd/suld/cms/

endef

define Package/samsung-unified-scanner-driver/install
	$(INSTALL_DIR) $(1)/etc/sane.d/dll.d $(1)/usr/lib/sane $(1)/usr/lib/udev/rules.d
	$(INSTALL_CONF) $(PKG_BUILD_DIR)/noarch/etc/smfp.conf $(1)/etc/sane.d/
	$(INSTALL_CONF) ./files/xerox_mfp-smfp.conf $(1)/etc/sane.d/
	$(INSTALL_CONF) ./files/smfp-scanner $(1)/etc/sane.d/dll.d/
	$(INSTALL_CONF) ./files/smfp-scanner-fix $(1)/etc/sane.d/dll.d/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/x86_64/libsane-smfp.so.1.0.1 $(1)/usr/lib/sane/
	$(LN) libsane-smfp.so.1.0.1 $(1)/usr/lib/sane/libsane-smfp.so.1
	$(LN) libsane-smfp.so.1 $(1)/usr/lib/sane/libsane-smfp.so
endef

$(eval $(call BuildPackage,samsung-unified-printer-driver))
$(eval $(call BuildPackage,samsung-unified-scanner-driver))
