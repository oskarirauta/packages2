#
# Copyright (C) 2007-2019 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=glib2
PKG_VERSION:=2.66.7
PKG_RELEASE:=2

PKG_SOURCE:=glib-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=@GNOME/glib/2.66
PKG_HASH:=09f158769f6f26b31074e15b1ac80ec39b13b53102dfae66cfe826fb2cc65502

PKG_LICENSE:=LGPL-2.1-or-later
PKG_LICENSE_FILES:=COPYING
PKG_CPE_ID:=cpe:/a:gnome:glib

PKG_BUILD_DIR:=$(BUILD_DIR)/glib-$(PKG_VERSION)
HOST_BUILD_DIR:=$(BUILD_DIR_HOST)/glib-$(PKG_VERSION)
PKG_BUILD_DEPENDS:=meson/host gettext-full libiconv/host
HOST_BUILD_DEPENDS:=meson/host gettext-full/host libiconv/host libffi/host host-util-linux/host
PKG_CONFIG_DEPENDS:=CONFIG_BUILD_NLS libiconv/host
PKG_INSTALL:=1

include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk
include ../../../packages/devel/meson/meson.mk

define Package/glib2
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:=$(ICONV_DEPENDS) $(INTL_DEPENDS) +zlib +libpthread +libffi +libattr +libselinux +libmount +libelf +libasm +libdw
  TITLE:=glib 2.0
  URL:=http://www.gtk.org/
endef

define Package/glib2/description
  The GLib library of C routines
endef

HOST_LDFLAGS += -liconv -Wl,-rpath,$(STAGING_DIR_HOSTPKG)/lib
TARGET_CFLAGS += -ffunction-sections -fdata-sections
TARGET_LDFLAGS += -Wall -Wl,--gc-sections -lintl

COMP_ARGS= \
	-Ddefault_library=both \
	-Diconv=external \
	-Dselinux=enabled \
	-Dlibmount=enabled \
	-Dinternal_pcre=true \
	-Dman=false \
	-Ddtrace=false \
	-Dsystemtap=false \
	-Dgtk_doc=false \
	-Dbsymbolic_functions=true \
	-Dforce_posix_threads=true \
	-Dfam=false \
	-Dinstalled_tests=false \
	-Dnls=$(if $(CONFIG_BUILD_NLS),en,dis)abled \
	-Doss_fuzz=disabled \
	-Dglib_assert=true \
	-Dglib_checks=false \
	-Dglib_debug=true

MESON_HOST_ARGS += $(COMP_ARGS) -Dxattr=false -Diconv=external -Dselinux=disabled -Dinternal_pcre=true -Dnls=disabled -Doss_fuzz=disabled -Dsysprof=disabled -Dman=false -Dlibmount=disabled
MESON_ARGS += $(COMP_ARGS) -Dxattr=true -Db_lto=true --datadir=/usr/lib

CONFIGURE_ARGS += \
	datadir="lib/glib-2.0" \
	gio_module_dir="lib/glib-2.0"

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/{bin,include,lib} $(2)/share/aclocal $(1)/usr/share/glib-2.0
	$(INSTALL_DIR) $(1)/usr/lib/glib-2.0 $(1)/usr/share/glib-2.0 $(1)/usr/lib/pkgconfig

	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin/

	$(CP) $(PKG_INSTALL_DIR)/usr/include/glib-2.0 $(1)/usr/include/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/glib-2.0/include/*.h $(1)/usr/include/glib-2.0/
	$(CP) $(PKG_INSTALL_DIR)/usr/include/gio-unix-2.0 $(1)/usr/include/

	$(CP) $(PKG_INSTALL_DIR)/usr/lib/glib-2.0 $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.{so*,a} $(1)/usr/lib/

	$(SED) '/^Libs:/s/-lintl \(.*\)/\1 -lintl/' \
		$(PKG_INSTALL_DIR)/usr/lib/pkgconfig/glib-2.0.pc

	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/*.pc $(1)/usr/lib/pkgconfig

	$(INSTALL_DATA) \
		$(PKG_INSTALL_DIR)/usr/lib/aclocal/*.m4  \
		$(2)/share/aclocal/

	$(CP) $(PKG_INSTALL_DIR)/usr/lib/glib-2.0/codegen $(1)/usr/lib/glib-2.0/
	$(LN) ../../lib/glib-2.0/codegen $(1)/usr/share/glib-2.0/codegen

endef

define Package/glib2/install
	$(INSTALL_DIR) $(1)/usr/bin $(1)/usr/lib

	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.so* $(1)/usr/lib/

	$(INSTALL_DIR) $(1)/usr/lib/gettext/its $(1)/usr/lib/glib-2.0/schemas
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/gettext/its/gschema.* $(1)/usr/lib/gettext/its/
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/glib-2.0/schemas/gschema.dtd $(1)/usr/lib/glib-2.0/schemas/
endef

define Package/glib2/postinst
#!/bin/sh
/usr/bin/glib-compile-schemas /usr/lib/glib-2.0/schemas
endef

$(eval $(call HostBuild))
$(eval $(call BuildPackage,glib2))
