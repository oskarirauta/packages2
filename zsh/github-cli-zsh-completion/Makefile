include $(TOPDIR)/rules.mk

PKG_NAME:=github-cli-zsh-completion
PKG_VERSION:=2.83.2
PKG_RELEASE:=1

PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>
PKG_LICENSE:=MIT

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=ZSH extensions
  TITLE:=Zsh completion for gh (GitHub CLI)
  URL:=https://cli.github.com
  DEPENDS:=+zsh
endef

define Package/$(PKG_NAME)/description
  Zsh completion function for gh, the GitHub CLI tool.
endef

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/share/zsh/site-functions
	$(INSTALL_DATA) ./files/_gh $(1)/usr/share/zsh/site-functions/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
