include $(TOPDIR)/rules.mk

PKG_NAME:=zsh-vcs
PKG_VERSION:=5.9
PKG_RELEASE:=1

PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>
PKG_LICENSE:=MIT-Modern-Variant

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=ZSH extensions
  TITLE:=Zsh VCS_Info module
  URL:=https://www.zsh.org/
  DEPENDS:=+zsh
endef

define Package/$(PKG_NAME)/description
  Version Control Information module for Zsh (vcs_info), provided by the
  zsh project.
endef

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/share/zsh/functions
	$(CP) ./files/VCS_Info $(1)/usr/share/zsh/functions/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
