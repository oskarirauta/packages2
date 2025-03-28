include $(TOPDIR)/rules.mk

PKG_NAME:=meson
PKG_VERSION:=1.6.1
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/mesonbuild/meson/releases/download/$(PKG_VERSION)
PKG_HASH:=1eca49eb6c26d58bbee67fd3337d8ef557c0804e30a6d16bfdf269db997464de

PKG_LICENSE:=Apache-2.0
PKG_LICENSE_FILES:=COPYING

include $(INCLUDE_DIR)/package.mk

define Package/meson
  SECTION:=devel
  CATEGORY:=Development
  TITLE:=meson
  URL:=https://github.com/mesonbuild/meson
  DEPENDS:=+python3
endef

define Package/meson/description
  Meson is a project to create the best possible next-generation build system.
endef


define Build/Configure
endef

define Build/Compile
endef

define Package/meson/install
	$(INSTALL_DIR) $(1)/bin
	$(STAGING_DIR_HOST)/bin/$(PYTHON) $(PKG_BUILD_DIR)/packaging/create_zipapp.py $(PKG_BUILD_DIR) --outfile $(1)/bin/meson.py
	$(INSTALL_DIR) $(1)/lib/meson
	$(INSTALL_CONF) files/openwrt-cross.txt.in $(1)/lib/meson/
	$(INSTALL_CONF) files/openwrt-native.txt.in $(1)/lib/meson/
endef

$(eval $(call BuildPackage,meson))
