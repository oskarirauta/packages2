#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=ghostscript
PKG_VERSION:=9.53.3
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/ghostpdl-$(PKG_VERSION)
PKG_SOURCE:=ghostpdl-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/ArtifexSoftware/ghostpdl-downloads/releases/download/gs9533/
PKG_HASH:=96d04e4e464bddb062c1774ea895c4f1c1c94e6c4b62f5d32218ebd44dd65ba1

include $(INCLUDE_DIR)/package.mk

PKG_INSTALL:=1

define Package/ghostscript
  SECTION:=print
  CATEGORY:=Printing
  TITLE:=ghostscript
  DEPENDS:=+libexpat +fontconfig +libjpeg-turbo +libpaper +libpng +libtiff +zlib
  URL:=https://www.ghostscript.com/
endef

define Package/ghostscript/description
  An interpreter for the PostScript language
endef

TARGET_CFLAGS += $(FPIC)
CONFIGURE_ARGS += \
	--datarootdir=/usr/lib \
	--datadir=/usr/lib \
	--enable-dynamic \
	--with-ijs \
	--with-jbig2dec \
	--without-x \
	--with-drivers=ALL \
	--with-openprinting \
	--with-fontpath=/usr/share/fonts/ \
	--enable-fontconfig \
	--enable-freetype \
	--enable-openjpeg \
	--without-luratech \
	--with-system-libtiff \
	--with-libpaper \
	--disable-compile-inits \
	--disable-gtk \
	--without-versioned-path \
	--with-cups-serverbin=/usr/lib/cups \
	--with-cups-serverroot=/etc/cups \
	--with-cups-datadir=/usr/share/cups

define Build/Prepare
	$(Build/Prepare/Default)
	rm -rf $(PKG_BUILD_DIR)/cups/libs $(PKG_BUILD_DIR)/jpeg \
		$(PKG_BUILD_DIR)/libpng $(PKG_BUILD_DIR)/zlib \
		$(PKG_BUILD_DIR)/tiff $(PKG_BUILD_DIR)/expat \
		$(PKG_BUILD_DIR)/gpdl
endef

define Build/Compile
	+$(MAKE_VARS) DESTDIR="$(PKG_INSTALL_DIR)" \
	$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR)/$(MAKE_PATH) \
		$(MAKE_FLAGS) DESTDIR="$(PKG_INSTALL_DIR)" so-only
	+$(MAKE_VARS) DESTDIR="$(PKG_INSTALL_DIR)" \
	$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR)/$(MAKE_PATH) \
		$(MAKE_FLAGS) DESTDIR="$(PKG_INSTALL_DIR)" all
endef

define Build/Install
	+$(MAKE_VARS) DESTDIR="$(PKG_INSTALL_DIR)" \
	$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR)/$(MAKE_PATH) \
		$(MAKE_FLAGS) DESTDIR="$(PKG_INSTALL_DIR)" \
		soinstall
endef

define Package/ghostscript/install
	$(INSTALL_DIR) $(1)/usr/bin $(1)/usr/lib/ghostscript \
		$(1)/usr/lib \
		$(1)/usr/lib/ghostscript/{Resource,iccprofiles,lib}
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/libgs.so** \
		$(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/ghostscript/Resource/** \
		$(1)/usr/lib/ghostscript/Resource/
	$(INSTALL_CONF) $(PKG_INSTALL_DIR)/usr/lib/ghostscript/iccprofiles/*.icc \
		$(1)/usr/lib/ghostscript/iccprofiles/
	$(INSTALL_CONF) $(PKG_INSTALL_DIR)/usr/lib/ghostscript/lib/** \
		$(1)/usr/lib/ghostscript/lib/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/** \
		$(1)/usr/bin/
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/ghostscript $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/libgs*.so** \
		$(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/include/*.h $(1)/usr/include/
endef

$(eval $(call BuildPackage,ghostscript))
