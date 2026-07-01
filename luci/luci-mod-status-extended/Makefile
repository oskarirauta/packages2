include $(TOPDIR)/rules.mk

PKG_NAME:=luci-mod-status-extended
PKG_VERSION:=1.0
PKG_RELEASE:=1
PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>

PKG_BUILD_DEPENDS:=luci-mod-status

LUCI_MOD_STATUS_DIR:=$(BUILD_DIR)/luci-mod-status/.pkgdir/luci-mod-status

include $(INCLUDE_DIR)/package.mk

define Package/luci-mod-status-extended
  SECTION:=luci
  CATEGORY:=LuCI
  SUBMENU:=2. Modules
  TITLE:=Extended system status view for LuCI
  DEPENDS:=+luci-base +libiwinfo +rpcd-mod-iwinfo
  PROVIDES:=luci-mod-status
  CONFLICTS:=luci-mod-status
  PKGARCH:=all
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) $(LUCI_MOD_STATUS_DIR)/. $(PKG_BUILD_DIR)/
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/luci-mod-status-extended/install
	$(CP) $(PKG_BUILD_DIR)/. $(1)/
	$(INSTALL_DIR) $(1)/www/luci-static/resources/svg
	$(INSTALL_DATA) ./htdocs/luci-static/resources/svg/feature-enabled.svg $(1)/www/luci-static/resources/svg/
	$(INSTALL_DATA) ./htdocs/luci-static/resources/svg/feature-disabled.svg $(1)/www/luci-static/resources/svg/
	$(INSTALL_DIR) $(1)/www/luci-static/resources/view/status/include
	$(INSTALL_DATA) ./htdocs/luci-static/resources/view/status/include/10_system.js $(1)/www/luci-static/resources/view/status/include/
	$(INSTALL_DIR) $(1)/usr/share/rpcd/acl.d
	$(INSTALL_DATA) ./root/usr/share/rpcd/acl.d/luci-app-status-extended.json $(1)/usr/share/rpcd/acl.d/
endef

$(eval $(call BuildPackage,luci-mod-status-extended))
