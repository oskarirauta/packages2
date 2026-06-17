include $(TOPDIR)/rules.mk

PKG_NAME:=zsh-autosuggestions
PKG_VERSION:=0.7.1
PKG_RELEASE:=1

PKG_SOURCE:=v$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/zsh-users/zsh-autosuggestions/archive
PKG_HASH:=0df7affff21cd87ed298e6a3970ed08a1dd66a6efa676454ee5b091ad503badf

PKG_BUILD_DIR:=$(BUILD_DIR)/zsh-autosuggestions-$(PKG_VERSION)

PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=ZSH extensions
  TITLE:=Fish-like autosuggestions for Zsh
  URL:=https://github.com/zsh-users/zsh-autosuggestions
  DEPENDS:=+zsh
endef

define Package/$(PKG_NAME)/description
  Fish-like fast/unobtrusive autosuggestions for zsh. It suggests commands
  as you type based on history and completions.
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/share/zsh/plugins/$(PKG_NAME)
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/$(PKG_NAME).zsh $(1)/usr/share/zsh/plugins/$(PKG_NAME)/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/$(PKG_NAME).plugin.zsh $(1)/usr/share/zsh/plugins/$(PKG_NAME)/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
