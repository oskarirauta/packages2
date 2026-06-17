include $(TOPDIR)/rules.mk

PKG_NAME:=patchelf-zsh-completion
PKG_VERSION:=0.18.0
PKG_RELEASE:=1

PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>
PKG_LICENSE:=GPL-3.0-or-later

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=ZSH extensions
  TITLE:=Zsh completion for patchelf
  URL:=https://nixos.org/patchelf.html
  DEPENDS:=+zsh
endef

define Package/$(PKG_NAME)/description
  Zsh completion function for patchelf, a utility to modify ELF executables.
endef

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/share/zsh/site-functions
	$(INSTALL_DATA) ./files/_patchelf $(1)/usr/share/zsh/site-functions/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
