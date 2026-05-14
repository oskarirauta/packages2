include $(TOPDIR)/rules.mk

PKG_NAME:=uchardet
PKG_VERSION:=0.0.8
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=https://www.freedesktop.org/software/uchardet/releases
PKG_HASH:=e97a60cfc00a1c147a674b097bb1422abd9fa78a2d9ce3f3fdcc2e78a34ac5f0

PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>
PKG_LICENSE:=MPL-1.1

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/libuchardet
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Encoding detector library ported from Mozilla
  URL:=https://www.freedesktop.org/wiki/Software/uchardet
  DEPENDS:=+libstdcpp
endef

define Package/libuchardet/description
  uchardet is an encoding detector library, which
  takes a sequence of bytes in an unknown character
  encoding without any additional information, and
  attempts to determine the encoding of the text.
  Returned encoding names are iconv-compatible.
endef

define Package/uchardet
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Encoder detector binary
  URL:=https://www.freedesktop.org/wiki/Software/uchardet
  DEPENDS:=+libstdcpp +libuchardet
endef

define Package/uchardet/description
  uchardet binary for libuchardet
endef

CMAKE_OPTIONS += \
	-DCMAKE_POLICY_VERSION_MINIMUM=3.5

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/uchardet $(1)/usr/lib $(1)/usr/lib/cmake/uchardet $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/include/uchardet/*.h $(1)/usr/include/uchardet/
	$(LN) -s uchardet/uchardet.h $(1)/usr/include/uchardet.h
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.so* $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.a $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/cmake/uchardet/*.cmake $(1)/usr/lib/cmake/uchardet/
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/*.pc $(1)/usr/lib/pkgconfig/
endef

define Package/libuchardet/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.so* $(1)/usr/lib/
endef

define Package/uchardet/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/uchardet $(1)/usr/bin/
endef

$(eval $(call BuildPackage,libuchardet))
$(eval $(call BuildPackage,uchardet))
