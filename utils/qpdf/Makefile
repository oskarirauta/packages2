#
# Copyright (C) 2007-2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=qpdf
PKG_VERSION:=10.3.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@SF/$(PKG_NAME)
PKG_HASH:=4649c66ddb3a4fc4b18927667fd3ae20a9edfbbdc1017ee1ed1e548d3abe00d1

PKG_INSTALL:=1

include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/package.mk

HOST_BUILD_DEPENDS:=libjpeg-turbo/host

define Package/qpdf
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=QPDF is a command-line program for transformations on PDF files.
  DEPENDS:= +libstdcpp +libqpdf
  URL:=http://qpdf.sourceforge.net/
  SUBMENU:=Printing
endef

define Package/qpdf/description
  QPDF is a command-line program that does structural, content-preserving
  transformations on PDF files. It could have been called something like pdf-to-pdf.
  It also provides  many useful capabilities to developers of PDF-producing software
  or for people who just want to look at the innards of a PDF file to learn more
  about how they work.
endef

define Package/libqpdf
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=QPDF library
  DEPENDS:= +libpcre +zlib +libstdcpp +libjpeg +libatomic
  SUBMENU:=Printing
endef

define Package/libqpdf/description
  QPDF Library
endef

CONFIGURE_ARGS+= \
	--disable-doc-maintenance \
	--disable-html-doc \
	--disable-pdf-doc \
	--disable-validate-doc \
	--with-random=/dev/urandom \
	--disable-check-autofiles \
	--disable-crypto-gnutls \
	--disable-crypto-openssl

HOST_CONFIGURE_ARGS+= \
	--disable-doc-maintenance \
	--disable-html-doc \
	--disable-pdf-doc \
	--disable-validate-doc \
	--with-random=/dev/urandom \
	--disable-check-autofiles \
	--disable-crypto-gnutls \
	--disable-crypto-openssl

#HOST_LDFLAGS+= -L$(BUILD_DIR_HOST)/libjpeg-turbo-2.0.6/lib
# Instead copy those files directly to staging_dir/hostpkg/lib

define Build/Compile
	$(call Build/Compile/Default, LIBTOOL="./libtool --tag=CXX")
endef

define Package/qpdf/install
	$(INSTALL_DIR) $(1)/usr/bin $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib/
endef

define Package/libqpdf/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libqpdf*.so* $(1)/usr/lib/
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/bin $(1)/usr/lib $(1)/usr/include/qpdf $(1)/usr/lib/pkgconfig
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin/
	$(CP) -r $(PKG_INSTALL_DIR)/usr/include/qpdf/** $(1)/usr/include/qpdf/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libqpdf*.so* $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libqpdf*.la $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libqpdf*.a $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/libqpdf.pc $(1)/usr/lib/pkgconfig/
endef

$(eval $(call BuildPackage,qpdf))
$(eval $(call BuildPackage,libqpdf))
$(eval $(call HostBuild))
