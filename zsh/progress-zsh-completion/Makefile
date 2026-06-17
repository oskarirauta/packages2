include $(TOPDIR)/rules.mk

PKG_NAME:=progress-zsh-completion
PKG_VERSION:=0.17
PKG_RELEASE:=1

PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>
PKG_LICENSE:=GPL-3.0-or-later

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=ZSH extensions
  TITLE:=Zsh completion for progress
  URL:=https://github.com/Xfennec/progress
  DEPENDS:=+zsh
endef

define Package/$(PKG_NAME)/description
  Zsh completion function for progress, the Coreutils progress viewer.
endef

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/share/zsh/site-functions
	$(INSTALL_DATA) ./files/_progress $(1)/usr/share/zsh/site-functions/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
