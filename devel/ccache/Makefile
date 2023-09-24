include $(TOPDIR)/rules.mk

PKG_NAME:=ccache
PKG_VERSION:=4.8.2
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=https://github.com/ccache/ccache/releases/download/v$(PKG_VERSION)
PKG_HASH:=3d3fb3f888a5b16c4fa7ee5214cca76348afd6130e8443de5f6f2424f2076a49

PKG_LICENSE:=LGPL-3.0-only GPL-3.0-only
PKG_LICENSE_FILES:=LICENSE.adoc GPL-3.0.txt LGPL-3.0.txt

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/ccache
  SECTION:=devel
  CATEGORY:=Development
  TITLE:=ccache
  URL:=https://github.com/ccache/ccache
  DEPENDS:=
endef

define Package/ccache/description
  Ccache is a compiler cache. It speeds up recompilation by caching previous
  compilations and detecting when the same compilation is being done again.
endef

CMAKE_OPTIONS += \
	-DCMAKE_SKIP_RPATH=FALSE \
	-DENABLE_DOCUMENTATION=OFF \
	-DREDIS_STORAGE_BACKEND=OFF

define Package/ccache/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/ccache $(1)/usr/bin/
endef

$(eval $(call BuildPackage,ccache))
