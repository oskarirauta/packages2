#
# Copyright (C) 2006-2016 OpenWrt.org
# Copyright (C) 2016 lede-project.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=cups
PKG_VERSION:=2.3.3
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-source.tar.gz
PKG_SOURCE_URL:=https://github.com/apple/cups/releases/download/v$(PKG_VERSION)/
PKG_HASH:=261fd948bce8647b6d5cb2a1784f0c24cc52b5c4e827b71d726020bcc502f3ee

include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/package.mk

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1

define Package/cups
  SECTION:=print
  CATEGORY:=Printing
  DEPENDS:=+libcups +libpaper +libacl +libstdcpp
  TITLE:=Common UNIX Printing System (daemon)
  URL:=http://www.cups.org/
endef

define Package/cups/description
  Common UNIX Printing System (daemon)
endef

define Package/cups/conffiles
/etc/cups/cupsd.conf
/etc/cups/printers.conf
/etc/cups/classes.conf
/etc/cups/cups-files.conf
/etc/cups/snmp.conf
endef

define Package/libcups
  SECTION:=print
  CATEGORY:=Printing
  SUBMENU:=Libraries
  DEPENDS:=+zlib +libpthread +mdnsd +libjpeg +libpng +libusb-1.0 +libavahi-client +libavahi-dbus-support
  TITLE:=Common UNIX Printing System - Core library
  URL:=http://www.cups.org/
endef

define Package/libcups/description
  Common UNIX Printing System - Core library
endef

TARGET_LDFLAGS+=-Wl,-rpath-link=$(STAGING_DIR)/usr/lib

CONFIGURE_ARGS+= \
		--build=$(GNU_HOST_NAME) \
		--host=$(GNU_TARGET_NAME) \
		--localstatedir=/var \
		--sharedstatedir=/usr/share \
		--bindir=/usr/lib/cups/bin \
		--with-menudir=/usr/share/cups/applications \
		--with-icondir=/usr/share/cups/icons \
		--with-logdir=/var/log/cups \
		--with-rundir=/var/run/cups \
		--with-cupsd-file-perm=0775 \
		--with-exe-file-perm=0775 \
		--with-cups-user=65534 \
		--with-cups-group=65534 \
		--with-system-groups=0 \
		--with-domainsocket=/var/run/cups/cups.sock \
		--enable-libusb \
		--without-rcdir \
		--disable-pam \
		--enable-raw-printing \
		--disable-dbus \
		--enable-libpaper \
		--disable-ssl \
		--disable-gnutls \
		--disable-gssapi \
		--disable-launchd \
		--enable-threads \
		--enable-avahi \
		--disable-systemd \
		--enable-relro \
		--enable-webif \
		--enable-static \
		UNAME="Linux" \
		LIBS="$(TARGET_LDFLAGS) -lz -lpng -ljpeg"

HOST_CONFIGURE_ARGS += \
	--oldincludedir=$(HOST_BUILD_PREFIX)/include \
	--datarootdir=$(HOST_BUILD_PREFIX)/share \
	--datadir=$(HOST_BUILD_PREFIX)/share \
	--with-icondir=$(HOST_BUILD_PREFIX)/share/icons \
	--with-menudir=$(HOST_BUILD_PREFIX)/share/applications \
	--disable-libusb \
	--without-rcdir \
	--without-xinetd \
	--disable-pam \
	--disable-libpaper \
	--disable-ssl \
	--disable-gnutls \
	--disable-gssapi \
	--disable-launchd \
	--disable-avahi \
	--disable-systemd \
	--disable-webif \
	--disable-tcp-wrappers \
	--disable-acl \
	--disable-dbus \
	--disable-dnssd \
	--disable-cdsassl \
	--disable-browsing \
	UNAME="Linux" \
	LIBS="$(HOST_LDFLAGS)"

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		$(TARGET_CONFIGURE_OPTS) \
		DESTROOT="$(PKG_INSTALL_DIR)" \
		DSTROOT="$(PKG_INSTALL_DIR)" \
		STRIP="/bin/true" \
		all install
	$(RM) -rf $(PKG_BUILD_DIR)/ipkg-install/etc/cups/ppd $(PKG_BUILD_DIR)/ipkg-install/etc/cups/ssl
	$(INSTALL_DIR) $(PKG_INSTALL_DIR)/usr/lib64
	touch $(PKG_INSTALL_DIR)/usr/lib64/dummy
	$(CP) $(PKG_INSTALL_DIR)/usr/lib64/** $(PKG_INSTALL_DIR)/usr/lib/
	$(RM) $(PKG_INSTALL_DIR)/usr/lib/dummy
endef

define Package/cups/install

	rm -rf $(PKG_INSTALL_DIR)/etc/cups/ppd $(PKG_INSTALL_DIR)/etc/cups/ssl

	$(INSTALL_DIR) $(1)/etc/cups $(1)/etc/cups/ppd $(1)/etc/cups/ssl
	$(INSTALL_CONF) ./files/etc/cups/** $(1)/etc/cups
	$(INSTALL_CONF) $(PKG_INSTALL_DIR)/etc/cups/{cups-files.conf,snmp.conf} $(1)/etc/cups/
	
	$(INSTALL_DIR) $(1)/usr/lib/cups/{backend,bin,cgi-bin,command,daemon,driver,filter,monitor,notifier}
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/cups/backend/{dnssd,http,ipp,lpd,snmp,socket,usb} \
		$(1)/usr/lib/cups/backend/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/cups/bin/{cancel,cups-config,cupstestppd,ippeveprinter,ipptool,lp,lpoptions,lpq,lpr,lprm,lpstat,ppdc,ppdhtml,ppdi,ppdmerge,ppdpo} \
		$(1)/usr/lib/cups/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/cups/cgi-bin/{admin.cgi,classes.cgi,help.cgi,jobs.cgi,printers.cgi} \
		$(1)/usr/lib/cups/cgi-bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/cups/command/{ippevepcl,ippeveps} \
		$(1)/usr/lib/cups/command/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/cups/daemon/{cups-deviced,cups-driverd,cups-exec,cups-lpd} \
		$(1)/usr/lib/cups/daemon/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/cups/filter/{commandtops,pstops,rastertohp,rastertopwg,gziptoany,rastertoepson,rastertolabel} \
		$(1)/usr/lib/cups/filter/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/cups/monitor/{bcp,tbcp} \
		$(1)/usr/lib/cups/monitor/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/cups/notifier/{mailto,rss} \
		$(1)/usr/lib/cups/notifier/

	$(INSTALL_DIR) $(1)/usr/share/cups
	$(CP) -aR $(PKG_INSTALL_DIR)/usr/share/cups/** $(1)/usr/share/cups/
	rm -rf $(1)/usr/share/cups/desktop

	$(INSTALL_DIR) $(1)/usr/sbin

	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/{cupsaccept,cupsctl,cupsd,cupsfilter,lpadmin,lpc,lpinfo,lpmove} \
		$(1)/usr/sbin

	$(LN) /usr/sbin/cupsaccept $(1)/usr/sbin/cupsdisable
	$(LN) /usr/sbin/cupsaccept $(1)/usr/sbin/cupsenable
	$(LN) /usr/sbin/cupsaccept $(1)/usr/sbin/cupsreject
	
	# install initscript with priority 60
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/cupsd.init $(1)/etc/init.d/cupsd

	# needed for cups to find usb printers per http://wiki.openwrt.org/doc/howto/cups.server
	chmod 700 $(1)/usr/lib/cups/backend/usb
endef

define Package/libcups/install
	rm -rf $(PKG_INSTALL_DIR)/etc/cups/ppd $(PKG_INSTALL_DIR)/etc/cups/ssl
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libcups*.so* $(1)/usr/lib/
endef

define Build/InstallDev
	rm -rf $(PKG_INSTALL_DIR)/etc/cups/ppd $(PKG_INSTALL_DIR)/etc/cups/ssl
	$(INSTALL_DIR) $(1)/etc/cups
	$(INSTALL_CONF) ./files/etc/cups/** $(1)/etc/cups/
	$(INSTALL_CONF) $(PKG_INSTALL_DIR)/etc/cups/** $(1)/etc/cups/

	$(INSTALL_DIR) $(1)/bin $(1)/usr/bin $(1)/usr/lib/cups/bin $(2)/bin $(2)/usr/bin

	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/cups/bin/cups-config \
		$(1)/usr/lib/cups/bin/

	$(LN) $(1)/usr/lib/cups/bin/cups-config $(1)/bin/cups-config
	$(LN) $(1)/usr/lib/cups/bin/cups-config $(1)/usr/bin/cups-config
	$(LN) $(1)/usr/lib/cups/bin/cups-config $(2)/bin/cups-config
	$(LN) $(1)/usr/lib/cups/bin/cups-config $(2)/usr/bin/cups-config

	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/cups/bin/{cancel,cupstestppd,ippeveprinter,ipptool,lp,lpoptions,lpq,lpr,lprm,lpstat,ppdc,ppdhtml,ppdi,ppdmerge,ppdpo} $(1)/usr/lib/cups/bin/

	$(INSTALL_DIR) $(1)/usr/lib/cups/{backend,cgi-bin,command,daemon,driver,filter,monitor,notifier}
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/cups/backend/** $(1)/usr/lib/cups/backend/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/cups/cgi-bin/** $(1)/usr/lib/cups/cgi-bin/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/cups/command/** $(1)/usr/lib/cups/command/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/cups/daemon/** $(1)/usr/lib/cups/daemon/
	#$(CP) $(PKG_INSTALL_DIR)/usr/lib/cups/driver/** $(1)/usr/lib/cups/driver/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/cups/filter/** $(1)/usr/lib/cups/filter/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/cups/monitor/** $(1)/usr/lib/cups/monitor/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/cups/notifier/** $(1)/usr/lib/cups/notifier/

	$(INSTALL_DIR) $(1)/usr/sbin $(1)/usr/share/cups
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/{cupsaccept,cupsctl,cupsd,cupsfilter,lpadmin,lpc,lpinfo,lpmove} $(1)/usr/sbin/

	$(LN) /usr/sbin/cupsaccept $(1)/usr/sbin/cupsdisable
	$(LN) /usr/sbin/cupsaccept $(1)/usr/sbin/cupsenable
	$(LN) /usr/sbin/cupsaccept $(1)/usr/sbin/cupsreject

	$(CP) -aR $(PKG_INSTALL_DIR)/usr/share/cups/** $(1)/usr/share/cups/
	rm -rf $(1)/usr/share/cups/desktop

	$(INSTALL_DIR) $(1)/usr/include $(1)/usr/include/cups $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/include/cups/** $(1)/usr/include/
	$(CP) $(PKG_INSTALL_DIR)/usr/include/cups/** $(1)/usr/include/cups/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libcups*.so* $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libcups*.a $(1)/usr/lib/

	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(INSTALL_CONF) ./files/libcups.pc $(1)/usr/lib/pkgconfig/

endef

$(eval $(call BuildPackage,libcups))
$(eval $(call BuildPackage,cups))
$(eval $(call HostBuild))
