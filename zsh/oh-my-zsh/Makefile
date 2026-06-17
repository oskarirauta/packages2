include $(TOPDIR)/rules.mk

PKG_NAME:=oh-my-zsh
PKG_VERSION:=0_git20260329
PKG_RELEASE:=1

PKG_COMMIT:=cb13cc539fa87f1c8f57dbbaefa3d3dcbaa85099

PKG_SOURCE:=$(PKG_COMMIT).tar.gz
PKG_SOURCE_URL:=https://github.com/ohmyzsh/ohmyzsh/archive
PKG_HASH:=b636309a2b082a379a9e1602f8fe1d70525d5b24f8ec6c86febdd29230b3a013

PKG_BUILD_DIR:=$(BUILD_DIR)/ohmyzsh-$(PKG_COMMIT)

PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE.txt

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=ZSH extensions
  TITLE:=Delightful framework for managing zsh configurations
  URL:=https://ohmyz.sh/
  DEPENDS:=+zsh +powerline-extra-symbols
endef

define Package/$(PKG_NAME)/description
  Oh My Zsh is a delightful, open source, community-driven framework for
  managing your Zsh configuration. It comes bundled with thousands of helpful
  functions, helpers, plugins, themes, and a few things that make you shout.
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/share/oh-my-zsh
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/oh-my-zsh.sh $(1)/usr/share/oh-my-zsh/
	$(CP) -R $(PKG_BUILD_DIR)/cache $(1)/usr/share/oh-my-zsh/
	$(CP) -R $(PKG_BUILD_DIR)/custom $(1)/usr/share/oh-my-zsh/
	$(CP) -R $(PKG_BUILD_DIR)/lib $(1)/usr/share/oh-my-zsh/
	$(CP) -R $(PKG_BUILD_DIR)/log $(1)/usr/share/oh-my-zsh/
	$(CP) -R $(PKG_BUILD_DIR)/plugins $(1)/usr/share/oh-my-zsh/
	$(CP) -R $(PKG_BUILD_DIR)/templates $(1)/usr/share/oh-my-zsh/
	$(CP) -R $(PKG_BUILD_DIR)/themes $(1)/usr/share/oh-my-zsh/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
