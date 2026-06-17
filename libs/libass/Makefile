include $(TOPDIR)/rules.mk

PKG_NAME:=libass
PKG_VERSION:=0.17.4
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/libass/libass/releases/download/$(PKG_VERSION)
PKG_HASH:=a886b3b80867f437bc55cff3280a652bfa0d37b43d2aff39ddf3c4f288b8c5a8

PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>
PKG_LICENSE:=GPL-2.0-or-later

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/meson.mk

define Package/libass
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Portable library for SSA/ASS subtitle rendering
  URL:=https://github.com/libass/libass
  DEPENDS:=+fontconfig +libfreetype +fribidi +harfbuzz
endef

define Package/libass/description
  libass is a portable subtitle renderer for the ASS/SSA
  (Advanced Substation Alpha/Substation Alpha) subtitle format.
  It is mostly compatible with VSFilter.
endef

MESON_ARGS += \
	-Dtest=disabled \
	-Dcompare=disabled \
	-Dprofile=disabled \
	-Dfuzz=disabled \
	-Dcheckasm=disabled \
	-Dcoretext=disabled \
	-Ddirectwrite=disabled \
	-Dlibunibreak=disabled \
	-Db_lto=true

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/ass $(1)/usr/lib $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/include/ass/*.h $(1)/usr/include/ass/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.so* $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.a $(1)/usr/lib/
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/*.pc $(1)/usr/lib/pkgconfig/
endef

define Package/libass/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.so* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libass))
