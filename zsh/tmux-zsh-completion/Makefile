include $(TOPDIR)/rules.mk

PKG_NAME:=tmux-zsh-completion
PKG_VERSION:=5.9
PKG_RELEASE:=1

PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>
PKG_LICENSE:=MIT-Modern-Variant AND GPL-2.0-only

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=ZSH extensions
  TITLE:=Zsh completion for tmux
  URL:=https://www.zsh.org/
  DEPENDS:=+zsh
endef

define Package/$(PKG_NAME)/description
  Zsh completion function for tmux, provided by the zsh project.
endef

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/share/zsh/site-functions
	$(INSTALL_DATA) ./files/_tmux $(1)/usr/share/zsh/site-functions/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
