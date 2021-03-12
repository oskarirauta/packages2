include $(TOPDIR)/rules.mk

PKG_NAME:=cni-plugins
PKG_VERSION:=0.9.1
PKG_RELEASE:=2
PKG_LICENSE:=Apache-2.0
PKG_LICENSE_FILES:=LICENSE

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/containernetworking/plugins.git
PKG_SOURCE_DATE:=2021-03-11
PKG_SOURCE_VERSION:=2989aba9dc2f2e9652a102f43bf5973ee3982352
PKG_HASH:=ead715a94eff61712b409033fb307035c29759c13fc98faebbbbe68298c83759

PKG_BUILD_DEPENDS:=golang/host
PKG_BUILD_PARALLEL:=1
PKG_USE_MIPS16:=0

GO_PKG:=github.com/containernetworking/plugins/
GO_PKG_BUILD_PKG:=github.com/containernetworking/plugins/plugins/main/... \
		  github.com/containernetworking/plugins/plugins/meta/... \
		  github.com/containernetworking/plugins/plugins/ipam/...

include $(INCLUDE_DIR)/package.mk
include ../../../packages/lang/golang/golang-package.mk

define Package/cni-plugins
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=cni-plugins
  URL:=https://github.com/containernetworking/cni-plugins
  DEPENDS:=$(GO_ARCH_DEPENDS) +ip-full
endef

define Package/cni-plugins/description
  Some CNI network plugins, maintained by the containernetworking team. For
  more information, see the individual READMEs.
endef

define Package/cni-plugins/install
	$(call GoPackage/Package/Install/Bin,$(PKG_INSTALL_DIR))
	$(INSTALL_DIR) $(1)/usr/lib/cni
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/lib/cni
endef

$(eval $(call GoBinPackage,cni-plugins))
$(eval $(call BuildPackage,cni-plugins))
