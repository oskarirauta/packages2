#
# Copyright (C) 2006-2016 OpenWrt.org
# Copyright (C) 2016 lede-project.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=cups-filters
PKG_VERSION:=1.28.7
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=https://www.openprinting.org/download/cups-filters
PKG_HASH:=e4150902809c58dfff7089c9345f196ecd88e38bce2be4800fa4811a0902057d

include $(INCLUDE_DIR)/package.mk

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=0

define Package/cups-filters/Default
  URL:=http://www.cups.org/
  SECTION:=print
  CATEGORY:=Printing
endef

define Package/cups-filters
$(call Package/cups-filters/Default)
  DEPENDS:=+libcups +libcups-filters +glib2
  TITLE:=OpenPrinting CUPS Filters
endef

define Package/cups-filters/description
  OpenPrinting CUPS Filters
endef

define Package/libcups-filters
$(call Package/cups/Default)
  SUBMENU:=Libraries
  DEPENDS:=+libcups +libiconv-full +lcms2 +dejavu-fonts-ttf-DejaVuSans +libqpdf +poppler +fontconfig
  TITLE:=OpenPrinting CUPS Filters library
endef

define Package/libcups-filters/description
  OpenPrinting CUPS Filters library
endef

define Package/cups/conffiles
/etc/cups/cups-browsed.conf
endef

TARGET_CFLAGS += -std=gnu11 -D_GNU_SOURCE
TARGET_CXXFLAGS += -D_GNU_SOURCE
TARGET_LDFLAGS += -Wl,--as-needed,-rpath-link=$(STAGING_DIR)/usr/lib -L$(STAGING_DIR)/usr/lib/libiconv-full/lib -lcups -liconv

CONFIGURE_ARGS += \
		--build=$(GNU_HOST_NAME) \
		--host=$(GNU_TARGET_NAME) \
		--localstatedir=/var \
		--sharedstatedir=/usr/share \
		--bindir=/usr/lib/cups/bin \
		--sysconfdir=/etc \
		--enable-static \
		--with-pdftops=pdftops \
		--with-shell=/bin/sh \
		--with-rcdir=no \
		--without-rcdir \
		--without-rclevels \
		--disable-avahi \
		--disable-ldap \
		--enable-pclm \
		--disable-dbus \
		--enable-saving-created-queues \
		--with-cups-config=/usr/lib/cups/bin/cups-config \
		--with-cups-rundir=/var/run/cups \
		--with-cups-domainsocket=/var/run/cups/cups.sock \
		--without-tiff \
		--without-php \
		--with-browseremoteprotocols=DNSSD,CUPS \
		--enable-driverless \
		--with-test-font-path=/usr/share/fonts/ttf-dejavu/DejaVuSans.ttf \
		UNAME="Linux" \
		LIBS="$(TARGET_LDFLAGS) -lz -lpng -ljpeg"

define Package/libcups-filters/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libcups*.so* $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libfontembed*.so* $(1)/usr/lib/
endef

define Package/cups-filters/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/cups-browsed $(1)/usr/sbin/

	$(INSTALL_DIR) $(1)/usr/lib/cups/{backend,bin,driver,filter}
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/cups/backend/** $(1)/usr/lib/cups/backend/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/cups/bin/** $(1)/usr/lib/cups/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/cups/driver/** $(1)/usr/lib/cups/driver/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/cups/filter/** $(1)/usr/lib/cups/filter/

	$(INSTALL_DIR) $(1)/usr/share/cups $(1)/usr/share/ppd/cupsfilters
	$(INSTALL_DIR) $(1)/usr/share/cups/{banners,braille,charsets,data,drv,mime,ppdc}
	$(CP) $(PKG_INSTALL_DIR)/usr/share/cups/banners/** $(1)/usr/share/cups/banners/
	$(CP) $(PKG_INSTALL_DIR)/usr/share/cups/braille/** $(1)/usr/share/cups/braille/
	$(CP) $(PKG_INSTALL_DIR)/usr/share/cups/charsets/** $(1)/usr/share/cups/charsets/
	$(CP) $(PKG_INSTALL_DIR)/usr/share/cups/data/** $(1)/usr/share/cups/data/
	$(CP) $(PKG_INSTALL_DIR)/usr/share/cups/drv/** $(1)/usr/share/cups/drv/
	$(CP) $(PKG_INSTALL_DIR)/usr/share/cups/mime/** $(1)/usr/share/cups/mime/
	$(CP) $(PKG_INSTALL_DIR)/usr/share/cups/ppdc/** $(1)/usr/share/cups/ppdc/
	$(CP) $(PKG_INSTALL_DIR)/usr/share/ppd/cupsfilters/** $(1)/usr/share/ppd/cupsfilters/
endef

define Build/InstallDev
	$(INSTALL_DIR) \
		$(1)/etc/cups \
		$(1)/usr/include/cupsfilters \
		$(1)/usr/include/fontembed \
		$(1)/usr/lib $(1)/usr/lib/cups $(1)/usr/lib/pkgconfig \
		$(1)/usr/sbin \
		$(1)/usr/share $(1)/usr/share/cups \
		$(1)/usr/share/ppd/cupsfilters

	$(INSTALL_CONF) $(PKG_INSTALL_DIR)/etc/cups/cups-browsed.conf $(1)/etc/cups/
	$(CP) $(PKG_INSTALL_DIR)/usr/include/cupsfilters/** $(1)/usr/include/cupsfilters/
	$(CP) $(PKG_INSTALL_DIR)/usr/include/fontembed/** $(1)/usr/include/fontembed/

	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libcupsfilters*.so* $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libfontembed*.so $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libcupsfilters.a $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libfontembed.a $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libcupsfilters.la $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libfontembed.la $(1)/usr/lib/

	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/*.pc $(1)/usr/lib/pkgconfig/

	$(INSTALL_DIR) $(1)/usr/lib/cups/{backend,bin,driver,filter}

	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/cups/backend/** $(1)/usr/lib/cups/backend/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/cups/bin/** $(1)/usr/lib/cups/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/cups/driver/** $(1)/usr/lib/cups/driver/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/cups/filter/** $(1)/usr/lib/cups/filter/

	$(CP) -aR $(PKG_INSTALL_DIR)/usr/share/cups/** $(1)/usr/share/cups/
	$(INSTALL_CONF) $(PKG_INSTALL_DIR)/usr/share/ppd/cupsfilters/** $(1)/usr/share/ppd/cupsfilters/
endef

$(eval $(call BuildPackage,cups-filters))
$(eval $(call BuildPackage,libcups-filters))
