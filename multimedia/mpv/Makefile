include $(TOPDIR)/rules.mk

PKG_NAME:=mpv
PKG_VERSION:=0.41.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-v$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/mpv-player/mpv/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=ee21092a5ee427353392360929dc64645c54479aefdb5babc5cfbb5fad626209

PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>
PKG_LICENSE:=GPL-2.0-or-later

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/meson.mk

define Package/mpv
  SECTION:=multimedia
  CATEGORY:=Multimedia
  TITLE:=Video player based on MPlayer/mplayer2
  URL:=https://mpv.io/
  DEPENDS:=+libarchive +libass +libdrm +libjpeg +libplacebo +libuchardet +libva +libwayland +libxkbcommon +alsa-lib +libffmpeg +libdisplay-info +zlib
endef

define Package/mpv/description
  mpv is a free (as in freedom) media player
  for the command line. It supports a wide
  variety of media file formats, audio and
  video codecs, and subtitle types.
endef

MESON_ARGS += \
	-Dcplayer=true \
	-Dalsa=enabled \
	-Db_lto=true \
	-Dbuild-date=false \
	-Dcaca=disabled \
	-Dcdda=disabled \
	-Diconv=enabled \
	-Duchardet=enabled \
	-Duwp=disabled \
	-Ddrm=enabled \
	-Ddvdnav=disabled \
	-Degl-drm=enabled \
	-Degl=enabled \
	-Degl-wayland=enabled \
	-Degl-x11=disabled \
	-Dgbm=enabled \
	-Dgl=enabled \
	-Djack=disabled \
	-Dlibmpv=true \
	-Dpulse=disabled \
	-Dsixel=enabled \
	-Dsndio=disabled \
	-Dtests=false \
	-Duchardet=enabled \
	-Dvdpau=disabled \
	-Dvulkan=disabled \
	-Dx11=disabled \
	-Dxv=disabled \
	-Dlua=disabled \
	-Dlibbluray=disabled \
	-Djavascript=disabled \
	-Ddvbin=disabled \
	-Dsixel=disabled \
	-Dmanpage-build=disabled

define Build/InstallDev
        $(INSTALL_DIR) $(1)/usr/include/mpv $(1)/usr/lib $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/include/mpv/*.h $(1)/usr/include/mpv/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.so* $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.a $(1)/usr/lib/
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/mpv.pc $(1)/usr/lib/pkgconfig/
endef

define Package/mpv/install
	$(INSTALL_DIR) $(1)/usr/bin $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/mpv $(1)/usr/bin/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.so* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,mpv))
