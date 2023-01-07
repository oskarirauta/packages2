#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=efivar
PKG_VERSION:=38
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=https://github.com/rhboot/efivar/releases/download/$(PKG_VERSION)
PKG_HASH:=f018ed6e49c5f1c16d336d9fd7687ce87023276591921db1e49a314ad6515349

PKG_LICENSE:=LGPL-2.1-only
PKG_LICENSE_FILES:=COPYING
PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/efivar
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Tools and libraries to work with EFI variables
  DEPENDS:=@TARGET_x86_64
  URL:=https://github.com/rhboot/efibootmgr
endef

define Package/efivar/description
  Tools and libraries to work with EFI variables
endef

MAKE_VARS += ERRORS=

define Build/Prepare
	$(Build/Prepare/Default)
	$(CP) ./files/qsort_r_missing.* $(PKG_BUILD_DIR)/src/
	$(CP) ./files/guids.lds $(PKG_BUILD_DIR)/src/
	$(CP) ./files/efivar-guids.h $(PKG_BUILD_DIR)/src/include/efivar/
	$(CP) ./files/guid-symbols.c $(PKG_BUILD_DIR)/src/
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/efivar $(1)/usr/lib $(1)/usr/lib/pkgconfig
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/{libefiboot.so.1.38,libefisec.so.1.38,libefivar.so.1.38} $(1)/usr/lib/
	$(LN) libefiboot.so.1.38 $(1)/usr/lib/libefiboot.so
	$(LN) libefiboot.so.1.38 $(1)/usr/lib/libefiboot.so.1
	$(LN) libefisec.so.1.38 $(1)/usr/lib/libefisec.so
	$(LN) libefisec.so.1.38 $(1)/usr/lib/libefisec.so.1
	$(LN) libefivar.so.1.38 $(1)/usr/lib/libefivar.so
	$(LN) libefivar.so.1.38 $(1)/usr/lib/libefivar.so.1
	$(CP) $(PKG_INSTALL_DIR)/usr/include/efivar/*.h $(1)/usr/include/efivar/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/*.pc $(1)/usr/lib/pkgconfig/	
endef

define Package/efivar/install
	$(INSTALL_DIR) $(1)/usr/bin $(1)/usr/lib/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/{efisecdb,efivar} $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/{libefiboot.so.1.38,libefisec.so.1.38,libefivar.so.1.38} $(1)/usr/lib/
	$(LN) libefiboot.so.1.38 $(1)/usr/lib/libefiboot.so
	$(LN) libefiboot.so.1.38 $(1)/usr/lib/libefiboot.so.1
	$(LN) libefisec.so.1.38 $(1)/usr/lib/libefisec.so
	$(LN) libefisec.so.1.38 $(1)/usr/lib/libefisec.so.1
	$(LN) libefivar.so.1.38 $(1)/usr/lib/libefivar.so
	$(LN) libefivar.so.1.38 $(1)/usr/lib/libefivar.so.1
endef

$(eval $(call BuildPackage,efivar))
