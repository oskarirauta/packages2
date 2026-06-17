include $(TOPDIR)/rules.mk

PKG_NAME:=adwaita-fonts
PKG_VERSION:=50.0
PKG_RELEASE:=1

PKG_COMMIT:=bd828c71418f7ae3af76687d6ca2a809111770f2

PKG_SOURCE:=$(PKG_COMMIT).tar.gz
PKG_SOURCE_URL:=https://github.com/GNOME/adwaita-fonts/archive
PKG_HASH:=c7f059630bd096bdd25116b2ec33c4596aeaeef1126541a8473261b374c8170f

PKG_BUILD_DIR:=$(BUILD_DIR)/adwaita-fonts-$(PKG_COMMIT)

PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

include $(INCLUDE_DIR)/package.mk

define Package/adwaita-fonts
  SECTION:=fonts
  CATEGORY:=Fonts
  TITLE:=Adwaita ttf fonts
  URL:=https://github.com/GNOME/adwaita-fonts
  DEPENDS:=+fontconfig
endef

define Package/adwaita-fonts/description
  Adwaita fonts with Mono and Sans families.
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/adwaita-fonts/install
	$(INSTALL_DIR) $(1)/usr/share/fonts/adwaita
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/mono/AdwaitaMono-Bold.ttf $(1)/usr/share/fonts/adwaita/adwaitamono-bold.ttf
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/mono/AdwaitaMono-BoldItalic.ttf $(1)/usr/share/fonts/adwaita/adwaitamono-bolditalic.ttf
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/mono/AdwaitaMono-Italic.ttf $(1)/usr/share/fonts/adwaita/adwaitamono-italic.ttf
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/mono/AdwaitaMono-Regular.ttf $(1)/usr/share/fonts/adwaita/adwaitamono-regular.ttf
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/sans/AdwaitaSans-Italic.ttf $(1)/usr/share/fonts/adwaita/adwaitasans-italic.ttf
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/sans/AdwaitaSans-Regular.ttf $(1)/usr/share/fonts/adwaita/adwaitasans-regular.ttf
endef

$(eval $(call BuildPackage,adwaita-fonts))
