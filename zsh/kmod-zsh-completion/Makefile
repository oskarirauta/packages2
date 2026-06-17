include $(TOPDIR)/rules.mk

PKG_NAME:=kmod-zsh-completion
PKG_VERSION:=34.2
PKG_RELEASE:=1

PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>
PKG_LICENSE:=GPL-2.0-or-later

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=ZSH extensions
  TITLE:=Zsh completions for kmod utilities
  URL:=http://git.kernel.org/?p=utils/kernel/kmod/kmod.git;a=summary
  DEPENDS:=+zsh
endef

define Package/$(PKG_NAME)/description
  Zsh completion functions for kmod utilities (insmod, lsmod, rmmod).
endef

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/share/zsh/site-functions
	$(INSTALL_DATA) ./files/_insmod $(1)/usr/share/zsh/site-functions/
	$(INSTALL_DATA) ./files/_lsmod $(1)/usr/share/zsh/site-functions/
	$(INSTALL_DATA) ./files/_rmmod $(1)/usr/share/zsh/site-functions/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
