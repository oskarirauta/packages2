include $(TOPDIR)/rules.mk

PKG_NAME:=libplacebo
PKG_VERSION:=7.360.1
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-v$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://code.videolan.org/videolan/libplacebo/-/archive/v$(PKG_VERSION)
PKG_HASH:=14c0a99f4b01557ec9826ce6b1d52f6de21be274ba03fd5aab7307f18766dc39
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-v$(PKG_VERSION)

PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>
PKG_LICENSE:=GPL-2.0-or-later

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/meson.mk

define Package/libplacebo
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Reusable library for GPU-accelerated video/image rendering
  URL:=https://code.videolan.org/videolan/libplacebo
  DEPENDS:=+libstdcpp
endef

define Package/libplacebo/description
  libplacebo is, in a nutshell, the core rendering algorithms and ideas of
  mpv rewritten as an independent library. As of today,
  libplacebo contains a large assortment of video processing shaders, focusing
  on both quality and performance.
endef

MESON_ARGS += \
	-Db_lto=true \
	-Dbackend_max_links=4 \
	-Dvulkan=disabled \
	-Dd3d11=disabled \
	-Ddemos=false \
	-Dtests=false \
	-Dbench=false \
	-Dunwind=disabled

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/libplacebo $(1)/usr/lib $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/include/libplacebo/*.h $(1)/usr/include/libplacebo/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.so* $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.a $(1)/usr/lib/
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/*.pc $(1)/usr/lib/pkgconfig/
endef

define Package/libplacebo/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.so* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libplacebo))
