include $(TOPDIR)/rules.mk

PKG_NAME:=zsh-completions
PKG_VERSION:=0.35.0
PKG_RELEASE:=1

PKG_SOURCE:=zsh-completions-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/zsh-users/zsh-completions/archive/$(PKG_VERSION)
PKG_HASH:=811bb4213622720872e08d6e0857f1dd7bc12ff7aa2099a170b76301a53f4fbe

PKG_BUILD_DIR:=$(BUILD_DIR)/zsh-completions-$(PKG_VERSION)

PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>
PKG_LICENSE:=BSD-3-Clause AND Apache-2.0 AND MIT
PKG_LICENSE_FILES:=LICENSE

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=ZSH extensions
  TITLE:=Additional completion definitions for Zsh
  URL:=https://github.com/zsh-users/zsh-completions
  DEPENDS:=+zsh
endef

define Package/$(PKG_NAME)/description
  Additional completion definitions for Zsh, packaged as a plugin.
  Provides completion for many tools not covered by zsh itself.
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/share/zsh/plugins/$(PKG_NAME)/src
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/$(PKG_NAME).plugin.zsh $(1)/usr/share/zsh/plugins/$(PKG_NAME)/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/src/* $(1)/usr/share/zsh/plugins/$(PKG_NAME)/src/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
