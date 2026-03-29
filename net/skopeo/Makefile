include $(TOPDIR)/rules.mk

PKG_NAME:=skopeo
PKG_VERSION:=1.20.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/containers/skopeo/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=0c19fe51b2cd8d1bd5e38c03b97421e318fc08153bdf5ef2f816a29889eacdef

PKG_LICENSE:=Apache-2.0
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>

PKG_BUILD_DEPENDS:=golang/host
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk
include ../../../packages/lang/golang/golang-package.mk

define Package/skopeo
  SECTION:=net
  CATEGORY:=Network
  TITLE:=command line utility that performs various operations on container images
  URL:=https://github.com/containers/skopeo
  DEPENDS:=$(GO_ARCH_DEPENDS) +libgpgme +libsqlite3
endef

define Package/skopeo/description
  Work with remote images registries - retrieving information, images, signing content
endef

MAKE_VARS += \
	DISABLE_DOCS=1

MAKE_ARGS += bin/skopeo

define Package/skopeo/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/skopeo $(1)/usr/bin/
endef

$(eval $(call BuildPackage,skopeo))
