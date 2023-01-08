include $(TOPDIR)/rules.mk

PKG_NAME:=ztdns
PKG_VERSION:=0.0.2
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/palonsoro/ztdns.git
PKG_SOURCE_DATE:=2023-01-08
PKG_SOURCE_VERSION:=1510cb47083989549ce2ed53d01b56a79df69ad0
PKG_SOURCE_HASH:=skip

PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>

PKG_BUILD_DEPENDS:=golang/host
PKG_BUILD_PARALLEL:=1
PKG_USE_MIPS16:=0

GO_PKG:=github.com/uxbh/ztdns

include $(INCLUDE_DIR)/package.mk
#include ../../lang/golang/golang-package.mk
include ../../../packages/lang/golang/golang-package.mk

define Package/ztdns
  SECTION:=net
  CATEGORY:=Network
  TITLE:=ZerotierDNS
  URL:=https://github.com/uxbh/ztdns
  DEPENDS:=$(GO_ARCH_DEPENDS)
endef

define Package/ztdns/description
  ztDNS is a dedicated DNS server for a ZeroTier virtual network.
endef

define Package/ztdns/install
	$(call GoPackage/Package/Install/Bin,$(PKG_INSTALL_DIR))
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin/
endef

$(eval $(call GoBinPackage,ztdns))
$(eval $(call BuildPackage,ztdns))
