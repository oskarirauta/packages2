include $(TOPDIR)/rules.mk

PKG_NAME:=openwrt-zsh-config
PKG_VERSION:=0.5.0
PKG_RELEASE:=1

PKG_SOURCE:=alpine-zsh-config-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/jirutka/alpine-zsh-config/archive/v$(PKG_VERSION)
PKG_HASH:=68f09efdf66660d0cdacae6f5a23de9f4294c07f646eb0e45d6e726c60988f1a

PKG_BUILD_DIR:=$(BUILD_DIR)/alpine-zsh-config-$(PKG_VERSION)

PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=ZSH extensions
  TITLE:=A sensible default configuration for Zsh
  URL:=https://github.com/jirutka/alpine-zsh-config
  DEPENDS:=+zsh
endef

define Package/$(PKG_NAME)/description
  A sensible default configuration for Zsh, providing a good out-of-the-box
  experience with completions, history, key bindings, and more.
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/etc/zsh/zshrc.d
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/zshrc.d/10-base-dirs.zsh $(1)/etc/zsh/zshrc.d/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/zshrc.d/50-aliases.zsh $(1)/etc/zsh/zshrc.d/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/zshrc.d/50-command-not-found.zsh $(1)/etc/zsh/zshrc.d/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/zshrc.d/50-history.zsh $(1)/etc/zsh/zshrc.d/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/zshrc.d/50-key-bindings.zsh $(1)/etc/zsh/zshrc.d/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/zshrc.d/50-misc.zsh $(1)/etc/zsh/zshrc.d/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/zshrc.d/50-semantic-prompt.zsh $(1)/etc/zsh/zshrc.d/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/zshrc.d/70-completion.zsh $(1)/etc/zsh/zshrc.d/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
