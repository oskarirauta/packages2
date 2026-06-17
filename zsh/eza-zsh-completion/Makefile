include $(TOPDIR)/rules.mk

PKG_NAME:=eza-zsh-completion
PKG_VERSION:=0.23.4
PKG_RELEASE:=1

PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>
PKG_LICENSE:=EUPL-1.2

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=ZSH extensions
  TITLE:=Zsh completion for eza
  URL:=https://github.com/eza-community/eza
  DEPENDS:=+zsh
endef

define Package/$(PKG_NAME)/description
  Zsh completion function for eza, a modern replacement for ls.
endef

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/share/zsh/site-functions
	$(INSTALL_DATA) ./files/_eza $(1)/usr/share/zsh/site-functions/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
