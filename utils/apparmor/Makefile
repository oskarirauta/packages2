#
# Copyright (C) 2007-2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=apparmor
PKG_VERSION:=2.13.4
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://gitlab.com/apparmor/apparmor/-/archive/v$(PKG_VERSION)/
PKG_HASH:=skip
PKG_COMMIT:=df0ac742f7a1146181d8734d03334494f2015134
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-v$(PKG_VERSION)-$(PKG_COMMIT)

PKG_BUILD_DEPENDS:=python3
PKG_FIXUP:=autoreconf

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk

define Package/apparmor
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=AppArmor
  DEPENDS:=+libapparmor +python3
  URL:=https://gitlab.com/apparmor/apparmor/wikis/home
endef

define Package/apparmor/description
  Linux application security framework - mandatory access control for programs
endef

define Package/libapparmor
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=libapparmor
  DEPENDS:=$(INTL_DEPENDS)
endef

define Package/libapparmor/description
  apparmor library
endef

CONFIGURE_PATH=libraries/libapparmor
CONFIGURE_VARS+= PYTHON_VERSION=3 PYTHON_VERSIONS=python3 PYTHON=/usr/bin/python3
CONFIGURE_ARGS+= \
	--enable-man-pages=no \
	--datarootdir=/usr/lib \
	--datadir=/usr/lib \
	--enable-python \
	--enable-perl

TARGET_LDFLAGS+= -L$(STAGING_DIR)/usr/lib/libintl-full/lib

MAKE_VARS+= PYTHON_VERSION=3 PYTHON_VERSIONS=python3 PYTHON=/usr/bin/python3

define Build/Prepare
	$(call Build/Prepare/Default)
	$(CP) ./files/list_capabilities.sh $(PKG_BUILD_DIR)/common/
	chmod a+x $(PKG_BUILD_DIR)/common/list_capabilities.sh
	$(SED) "s/cpp/$(TARGET_CROSS)cpp/" $(PKG_BUILD_DIR)/common/list_capabilities.sh
	$(CP) ./files/list_af_names.sh $(PKG_BUILD_DIR)/common/
	chmod a+x $(PKG_BUILD_DIR)/common/list_af_names.sh
	$(SED) "s/cpp/$(TARGET_CROSS)cpp/" $(PKG_BUILD_DIR)/common/list_af_names.sh
endef

define Build/Compile
	+$(MAKE_VARS) \
	$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR)/libraries/libapparmor \
		$(MAKE_FLAGS);
	+$(MAKE_VARS) \
	$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR)/parser \
		$(MAKE_FLAGS);
	+$(MAKE_VARS) \
	$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR)/binutils \
		$(MAKE_FLAGS);
	+$(MAKE_VARS) \
	$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR)/utils \
		$(MAKE_FLAGS);
	+$(MAKE_VARS) \
	$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR)/profiles \
		$(MAKE_FLAGS);
	+$(MAKE_VARS) \
	$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR)/changehat/pam_apparmor \
		$(MAKE_FLAGS);

	mkdir -p \
		$(PKG_INSTALL_DIR)/lib \
		$(PKG_INSTALL_DIR)/utils \
		$(PKG_INSTALL_DIR)/pam

	+$(MAKE_VARS) \
	$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR)/libraries/libapparmor \
		$(MAKE_FLAGS) DESTDIR="$(PKG_INSTALL_DIR)/lib" install;

	+$(MAKE_VARS) \
	$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR)/utils \
		$(MAKE_FLAGS) DESTDIR="$(PKG_INSTALL_DIR)/utils" \
		BINDIR="$(PKG_INSTALL_DIR)/utils/usr/bin" install;

	+$(MAKE_VARS) \
	$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR)/binutils \
		$(MAKE_FLAGS) DESTDIR="$(PKG_INSTALL_DIR)/utils" \
		BINDIR="$(PKG_INSTALL_DIR)/utils/usr/bin" install;

	+$(MAKE_VARS) \
	$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR)/profiles \
		$(MAKE_FLAGS) DESTDIR="$(PKG_INSTALL_DIR)/utils" install

	+$(MAKE_VARS) \
	$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR)/changehat/pam_apparmor \
		$(MAKE_FLAGS) DESTDIR="$(PKG_INSTALL_DIR)/pam" install	
endef

define Package/apparmor/install
	$(INSTALL_DIR) \
		$(1)/usr/bin $(1)/usr/sbin \
		$(1)/etc/apparmor $(1)/etc/apparmor.d \
		$(1)/usr/lib/apparmor/easyprof \
		$(1)/usr/lib/apparmor/extra-profiles \
		$(1)/usr/lib/python3.9/site-packages
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/utils/usr/bin/** $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/parser/apparmor_parser $(1)/usr/sbin/
	$(RM) $(1)/usr/bin/apparmor_status
	$(LN) aa-status $(1)/usr/bin/apparmor_status
	$(CP) $(PKG_INSTALL_DIR)/utils/usr/share/apparmor/easyprof/** \
		$(1)/usr/lib/apparmor/easyprof/
	$(CP) $(PKG_INSTALL_DIR)/utils/usr/share/apparmor/extra-profiles/** \
		$(1)/usr/lib/apparmor/extra-profiles/
	$(CP) $(PKG_INSTALL_DIR)/utils/etc/apparmor.d/** $(1)/etc/apparmor.d/
	$(CP) ./etc/apparmor/** $(1)/etc/apparmor/
	$(CP) $(PKG_INSTALL_DIR)/utils/usr/lib/python3.9/site-packages/apparmor/** \
		$(1)/usr/lib/python3.9/site-packages/
	$(CP) ./files/rc.apparmor.functions $(1)/usr/lib/apparmor
endef

define Package/libapparmor/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/lib/usr/lib/libapparmor.so** $(1)/usr/lib/
endef

define Build/InstallDev
	$(INSTALL_DIR) \
		$(1)/usr/lib $(1)/usr/lib/pkgconfig \
		$(1)/usr/include/sys $(1)/usr/include/aalogparse
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/lib/usr/lib/libapparmor.** $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/lib/usr/include/sys/*.h $(1)/usr/include/sys/
	$(CP) $(PKG_INSTALL_DIR)/lib/usr/include/aalogparse/*.h $(1)/usr/include/aalogparse/
	$(INSTALL_CONF) $(PKG_INSTALL_DIR)/lib/usr/lib/pkgconfig/libapparmor.pc \
		$(1)/usr/lib/pkgconfig/
endef

$(eval $(call BuildPackage,apparmor))
$(eval $(call BuildPackage,libapparmor))
