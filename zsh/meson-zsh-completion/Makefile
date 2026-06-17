include $(TOPDIR)/rules.mk

PKG_NAME:=meson-zsh-completion
PKG_VERSION:=1.11.1
PKG_RELEASE:=1

PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>
PKG_LICENSE:=Apache-2.0

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=ZSH extensions
  TITLE:=Zsh completion for meson
  URL:=https://mesonbuild.com
  DEPENDS:=+zsh
endef

define Package/$(PKG_NAME)/description
  Zsh completion function for the meson build system.
endef

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/share/zsh/site-functions
	$(INSTALL_DATA) ./files/_meson $(1)/usr/share/zsh/site-functions/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
