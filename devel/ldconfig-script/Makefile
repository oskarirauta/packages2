include $(TOPDIR)/rules.mk

PKG_NAME:=ldconfig-script
PKG_VERSION:=20230308
PKG_RELEASE:=1

PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>

include $(INCLUDE_DIR)/package.mk

define Package/ldconfig-script/Default
endef

define Package/ldconfig-script
  SECTION:=devel
  CATEGORY:=Development
  TITLE:=MUSL ldconfig script
  PKGARCH:=all
endef

define Package/ldconfig-script/description
  Basicly ldconfig is missing from MUSL.
  This script is a copy from alpine linux's musl-utils
  packages provided ldconfig.
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/ldconfig-script/install
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_BIN) ./files/ldconfig $(1)/sbin/
endef

$(eval $(call BuildPackage,ldconfig-script))
