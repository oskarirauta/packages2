include $(TOPDIR)/rules.mk

PKG_NAME:=lsd-zsh-completion
PKG_VERSION:=1.2.0
PKG_RELEASE:=1

PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>
PKG_LICENSE:=Apache-2.0

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=ZSH extensions
  TITLE:=Zsh completion for lsd
  URL:=https://github.com/lsd-rs/lsd
  DEPENDS:=+zsh
endef

define Package/$(PKG_NAME)/description
  Zsh completion function for lsd, the next-gen ls command.
endef

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/share/zsh/site-functions
	$(INSTALL_DATA) ./files/_lsd $(1)/usr/share/zsh/site-functions/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
