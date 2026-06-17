include $(TOPDIR)/rules.mk

PKG_NAME:=apk-tools-zsh-completion
PKG_VERSION:=3.0.6
PKG_RELEASE:=1

PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>
PKG_LICENSE:=GPL-2.0-only

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=ZSH extensions
  TITLE:=Zsh completion for apk
  URL:=https://gitlab.alpinelinux.org/alpine/apk-tools
  DEPENDS:=+zsh
endef

define Package/$(PKG_NAME)/description
  Zsh completion function for the apk package manager.
endef

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/share/zsh/site-functions
	$(INSTALL_DATA) ./files/_apk $(1)/usr/share/zsh/site-functions/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
