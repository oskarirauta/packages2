include $(TOPDIR)/rules.mk

PKG_NAME:=fzf-zsh-completion
PKG_VERSION:=0.54.3
PKG_RELEASE:=1

PKG_SOURCE:=fzf-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/junegunn/fzf/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=skip
PKG_BUILD_DIR:=$(BUILD_DIR)/fzf-$(PKG_VERSION)

PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=ZSH extensions
  TITLE:=fzf zsh integration - key bindings and completion
  URL:=https://github.com/junegunn/fzf
  DEPENDS:=+fzf +zsh
endef

define Package/$(PKG_NAME)/description
  Zsh integration for fzf: key bindings (Ctrl+R history search,
  Ctrl+T file finder, Alt+C directory changer) and fuzzy tab
  completion. Installs as a zsh plugin under
  /usr/share/zsh/plugins/fzf/.
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/share/zsh/plugins/fzf
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/shell/key-bindings.zsh $(1)/usr/share/zsh/plugins/fzf/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/shell/completion.zsh $(1)/usr/share/zsh/plugins/fzf/
	$(INSTALL_DATA) ./files/fzf.plugin.zsh $(1)/usr/share/zsh/plugins/fzf/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
