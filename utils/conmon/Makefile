include $(TOPDIR)/rules.mk

PKG_NAME:=conmon
PKG_VERSION:=2.0.27
PKG_RELEASE:=1
PKG_LICENSE:=Apache-2.0
PKG_LICENSE_FILES:=LICENSE

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/containers/conmon.git
PKG_SOURCE_DATE:=2021-03-11
PKG_SOURCE_VERSION:=551605f2f3dfe8e1c812568cd667a6190addc18d
PKG_MIRROR_HASH:=a5a5ca893520b38c208d1d37bf48d32b641e19eb0bfec25e738896b4972aedd7

PKG_BUILD_DEPENDS:=golang/host
PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk

MAKE_VARS += PREFIX=/usr

define Package/conmon
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Podman conmon
  URL:=https://podman.io
  DEPENDS:=+glib2 $(INTL_DEPENDS) $(ICONV_DEPENDS)
endef

define Package/conmon/description
  Podman: A tool for managing OCI containers and pods 
endef

define Package/conmon/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/conmon $(1)/usr/bin
endef

$(eval $(call BuildPackage,conmon))
