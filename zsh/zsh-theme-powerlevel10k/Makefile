include $(TOPDIR)/rules.mk

PKG_NAME:=zsh-theme-powerlevel10k
PKG_VERSION:=1.20.0
PKG_RELEASE:=1

PKG_SOURCE:=zsh-theme-powerlevel10k-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/romkatv/powerlevel10k/archive/refs/tags/v$(PKG_VERSION)
PKG_HASH:=d8187d44b697b3a37a8c4896678b4380e717cbf2850179529358348780a2d3d7

PKG_BUILD_DIR:=$(BUILD_DIR)/powerlevel10k-$(PKG_VERSION)

PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=ZSH extensions
  TITLE:=Powerlevel10k theme for Zsh
  URL:=https://github.com/romkatv/powerlevel10k
  DEPENDS:=+zsh
endef

define Package/$(PKG_NAME)/description
  Powerlevel10k is a theme for Zsh emphasizing speed, flexibility, and
  out-of-the-box experience. Note: gitstatus integration requires the
  gitstatus binary to be separately installed.
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/share/zsh/plugins/powerlevel10k
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/powerlevel10k.zsh-theme $(1)/usr/share/zsh/plugins/powerlevel10k/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/powerlevel9k.zsh-theme $(1)/usr/share/zsh/plugins/powerlevel10k/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/prompt_powerlevel10k_setup $(1)/usr/share/zsh/plugins/powerlevel10k/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/prompt_powerlevel9k_setup $(1)/usr/share/zsh/plugins/powerlevel10k/
	$(INSTALL_DATA) ./files/powerlevel10k.plugin.zsh $(1)/usr/share/zsh/plugins/powerlevel10k/
	$(CP) $(PKG_BUILD_DIR)/config $(1)/usr/share/zsh/plugins/powerlevel10k/
	$(CP) $(PKG_BUILD_DIR)/internal $(1)/usr/share/zsh/plugins/powerlevel10k/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
