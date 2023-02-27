include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/version.mk

PKG_NAME:=altbanner
PKG_VERSION:=20230227
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/altbanner
	SECTION:=base
	CATEGORY:=Base system
	TITLE:=banner for passwordless root
	PKGARCH:=all
endef

define Package/altbanner/description
 Unsafe banner - or banner for passwordless root logon.
 This package contains alternative banner.unsafe for
 that is displayed during logon if root's password is
 unset. Made for private needs, requires modified
 /etc/profile file.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
endef

define Build/Compile/Default

endef
Build/Compile = $(Build/Compile/Default)

define Package/altbanner/install
	$(INSTALL_DIR) $(1)/etc
	$(CP) ./files/banner.unsafe $(1)/etc/
endef

$(eval $(call BuildPackage,altbanner))
