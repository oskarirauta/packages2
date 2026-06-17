include $(TOPDIR)/rules.mk

PKG_NAME:=powerline-extra-symbols
PKG_VERSION:=0_git20191017
PKG_RELEASE:=1

PKG_COMMIT:=ae05de7c51f6609479f4f1a4a0f6f65631731c1b

PKG_SOURCE:=$(PKG_COMMIT).tar.gz
PKG_SOURCE_URL:=https://github.com/ryanoasis/powerline-extra-symbols/archive
PKG_HASH:=febdf9677635777f7277ff7cf058428c618950cc9dff033800a9bf5e7e1536a3

PKG_BUILD_DIR:=$(BUILD_DIR)/powerline-extra-symbols-$(PKG_COMMIT)

PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=fonts
  CATEGORY:=Fonts
  TITLE:=Extra glyphs for powerline separators
  URL:=https://github.com/ryanoasis/powerline-extra-symbols
  DEPENDS:=+fontconfig
endef

define Package/$(PKG_NAME)/description
  Extra powerline glyphs for Powerline-compatible fonts.
  Required by oh-my-zsh for some themes.
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/share/fonts/$(PKG_NAME)
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/PowerlineExtraSymbols.otf $(1)/usr/share/fonts/$(PKG_NAME)/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
