include $(TOPDIR)/rules.mk

PKG_NAME:=zsh-history-search-multi-word
PKG_VERSION:=0_git20230705
PKG_RELEASE:=1

PKG_COMMIT:=c4dcddc1cd17e7e0909471703f3526170db0f475

PKG_SOURCE:=$(PKG_COMMIT).tar.gz
PKG_SOURCE_URL:=https://github.com/zdharma-continuum/history-search-multi-word/archive
PKG_HASH:=94a9a3b131aafa4109cd0193bd0b96dbfcf3a0b03b41c27e301902dc01cd1277

PKG_BUILD_DIR:=$(BUILD_DIR)/history-search-multi-word-$(PKG_COMMIT)

PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>
PKG_LICENSE:=MIT AND GPL-3.0-or-later
PKG_LICENSE_FILES:=LICENSE

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=ZSH extensions
  TITLE:=Multi-word history search for Zsh (Ctrl-R)
  URL:=https://github.com/zdharma-continuum/history-search-multi-word
  DEPENDS:=+zsh
endef

define Package/$(PKG_NAME)/description
  Binds Ctrl-R to a widget that searches for multiple keywords in an AND
  fashion in the Zsh command history.
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/share/zsh/plugins/history-search-multi-word
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/history-search-multi-word $(1)/usr/share/zsh/plugins/history-search-multi-word/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/history-search-multi-word.plugin.zsh $(1)/usr/share/zsh/plugins/history-search-multi-word/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/hsmw-context-main $(1)/usr/share/zsh/plugins/history-search-multi-word/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/hsmw-highlight $(1)/usr/share/zsh/plugins/history-search-multi-word/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
