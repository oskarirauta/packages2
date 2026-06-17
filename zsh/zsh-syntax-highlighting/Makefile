include $(TOPDIR)/rules.mk

PKG_NAME:=zsh-syntax-highlighting
PKG_VERSION:=0.8.0
PKG_RELEASE:=1

PKG_SOURCE:=zsh-syntax-highlighting-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/zsh-users/zsh-syntax-highlighting/archive/$(PKG_VERSION)
PKG_HASH:=5981c19ebaab027e356fe1ee5284f7a021b89d4405cc53dc84b476c3aee9cc32

PKG_BUILD_DIR:=$(BUILD_DIR)/zsh-syntax-highlighting-$(PKG_VERSION)

PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>
PKG_LICENSE:=BSD-3-Clause
PKG_LICENSE_FILES:=COPYING.md

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=ZSH extensions
  TITLE:=Fish shell like syntax highlighting for Zsh
  URL:=https://github.com/zsh-users/zsh-syntax-highlighting
  DEPENDS:=+zsh
endef

define Package/$(PKG_NAME)/description
  Fish shell-like syntax highlighting for Zsh. Highlights commands as you
  type, giving visual feedback before you run the command.
endef

define Build/Configure
endef

define Build/Compile
  $(MAKE) -C $(PKG_BUILD_DIR)
endef

define Package/$(PKG_NAME)/install
	$(MAKE) -C $(PKG_BUILD_DIR) install \
		DESTDIR=$(1) \
		PREFIX=/usr \
		SHARE_DIR=$(1)/usr/share/zsh/plugins/$(PKG_NAME)
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
