include $(TOPDIR)/rules.mk

PKG_NAME:=netbird-zsh-completion
PKG_VERSION:=0.70.5
PKG_RELEASE:=1

PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>
PKG_LICENSE:=BSD-3-Clause

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=ZSH extensions
  TITLE:=Zsh completions for NetBird
  URL:=https://netbird.io/
  DEPENDS:=+zsh
endef

define Package/$(PKG_NAME)/description
  Zsh completion functions for NetBird WireGuard-based mesh VPN
  (netbird, netbird-mgmt, netbird-signal).
endef

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/share/zsh/site-functions
	$(INSTALL_DATA) ./files/_netbird $(1)/usr/share/zsh/site-functions/
	$(INSTALL_DATA) ./files/_netbird-mgmt $(1)/usr/share/zsh/site-functions/
	$(INSTALL_DATA) ./files/_netbird-signal $(1)/usr/share/zsh/site-functions/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
