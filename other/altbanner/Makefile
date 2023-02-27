include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/version.mk

PKG_NAME:=altbanner
PKG_VERSION:=20230227
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/altbanner
	SECTION:=base
	CATEGORY:=Base system
	TITLE:=Unsafe banner
	PKGARCH:=all
endef

define Package/altbanner/description
 This package contains alternative banner.unsafe for private needs.
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
