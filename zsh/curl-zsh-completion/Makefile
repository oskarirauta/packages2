include $(TOPDIR)/rules.mk

PKG_NAME:=curl-zsh-completion
PKG_VERSION:=8.20.0
PKG_RELEASE:=1

PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>
PKG_LICENSE:=curl

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=ZSH extensions
  TITLE:=Zsh completion for curl
  URL:=https://curl.se/
  DEPENDS:=+zsh
endef

define Package/$(PKG_NAME)/description
  Zsh completion function for curl.
endef

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/share/zsh/site-functions
	$(INSTALL_DATA) ./files/_curl $(1)/usr/share/zsh/site-functions/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
