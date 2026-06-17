include $(TOPDIR)/rules.mk

PKG_NAME:=zsh-fast-syntax-highlighting
PKG_VERSION:=1.56
PKG_RELEASE:=1

PKG_SOURCE:=v$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/zdharma-continuum/fast-syntax-highlighting/archive/refs/tags
PKG_HASH:=9e5045510ef86beae658b5fcf58e7b6c76f5b63788498b956d54bc1038faa806

PKG_BUILD_DIR:=$(BUILD_DIR)/fast-syntax-highlighting-$(PKG_VERSION)

PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>
PKG_LICENSE:=BSD-3-Clause
PKG_LICENSE_FILES:=LICENSE

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=ZSH extensions
  TITLE:=Feature-rich syntax highlighting for Zsh
  URL:=https://github.com/zdharma-continuum/fast-syntax-highlighting
  DEPENDS:=+zsh
endef

define Package/$(PKG_NAME)/description
  Feature-rich syntax highlighting for Zsh. An alternative to
  zsh-syntax-highlighting offering better performance and more features.
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) \
		$(1)/usr/share/zsh/plugins/fast-syntax-highlighting \
		$(1)/usr/share/zsh/plugins/fast-syntax-highlighting/share \
		$(1)/usr/share/zsh/plugins/fast-syntax-highlighting/themes

	$(CP) $(PKG_BUILD_DIR)/_fast-theme $(1)/usr/share/zsh/plugins/fast-syntax-highlighting/
	$(CP) $(PKG_BUILD_DIR)/fast-highlight $(1)/usr/share/zsh/plugins/fast-syntax-highlighting/
	$(CP) $(PKG_BUILD_DIR)/fast-string-highlight $(1)/usr/share/zsh/plugins/fast-syntax-highlighting/
	$(CP) $(PKG_BUILD_DIR)/fast-syntax-highlighting.plugin.zsh $(1)/usr/share/zsh/plugins/fast-syntax-highlighting/
	$(CP) $(PKG_BUILD_DIR)/fast-theme $(1)/usr/share/zsh/plugins/fast-syntax-highlighting/

	$(CP) $(PKG_BUILD_DIR)/share/** $(1)/usr/share/zsh/plugins/fast-syntax-highlighting/share/
	$(CP) $(PKG_BUILD_DIR)/themes/** $(1)/usr/share/zsh/plugins/fast-syntax-highlighting/themes/
	$(CP) $(PKG_BUILD_DIR)/*chroma $(1)/usr/share/zsh/plugins/fast-syntax-highlighting/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
