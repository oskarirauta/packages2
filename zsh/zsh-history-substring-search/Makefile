include $(TOPDIR)/rules.mk

PKG_NAME:=zsh-history-substring-search
PKG_VERSION:=1.1.0
PKG_RELEASE:=1

PKG_SOURCE:=zsh-history-substring-search-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/zsh-users/zsh-history-substring-search/archive/v$(PKG_VERSION)
PKG_HASH:=9b52eca6c894dd98caa5f07160199f3f3179ff017575d5acc9fdc467b1ac70f8

PKG_BUILD_DIR:=$(BUILD_DIR)/zsh-history-substring-search-$(PKG_VERSION)

PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>
PKG_LICENSE:=BSD-3-Clause
PKG_LICENSE_FILES:=LICENSE

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=ZSH extensions
  TITLE:=Zsh port of the Fish shell's history search
  URL:=https://github.com/zsh-users/zsh-history-substring-search
  DEPENDS:=+zsh
endef

define Package/$(PKG_NAME)/description
  Zsh port of the Fish shell's history search (bound to up/down arrow keys),
  allows you to type in any part of any command from history and press the
  up and down arrow keys to cycle through matching commands.
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/share/zsh/plugins/$(PKG_NAME)
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/*.zsh $(1)/usr/share/zsh/plugins/$(PKG_NAME)/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
