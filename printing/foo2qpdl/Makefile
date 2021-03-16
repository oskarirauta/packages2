#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=foo2qpdl
PKG_VERSION:=0.99
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/foo2zjs
PKG_SOURCE:=foo2zjs.tar.gz
PKG_SOURCE_URL:=http://foo2zjs.rkkda.com/
PKG_HASH:=skip

include $(INCLUDE_DIR)/package.mk

PKG_BUILD_DEPENDS:=ghostscript/host cups/host cups-filters/host
PKG_INSTALL:=1

define Package/foo2qpdl
  SECTION:=print
  CATEGORY:=Printing
  TITLE:=foo2qpdl
  DEPENDS:=+libcups +ghostscript
  URL:=https://foo2zjs.rkkda.com/
endef

define Package/foo2qpdl/description
  With the foo2qpdl driver, you can print to some Samsung and Xerox printers.
endef

TARGET_CFLAGS += $(FPIC)

define Build/Prepare
	$(Build/Prepare/Default)
	touch $(PKG_BUILD_DIR)/manual.pdf
endef

define Build/Compile
	+$(MAKE_VARS) DESTDIR="$(PKG_INSTALL_DIR)" \
	$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR)/$(MAKE_PATH) \
		$(MAKE_FLAGS) DESTDIR="$(PKG_INSTALL_DIR)"
	+$(MAKE_VARS) DESTDIR="$(PKG_INSTALL_DIR)" \
	$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR)/$(MAKE_PATH) \
		$(MAKE_FLAGS) DESTDIR="$(PKG_INSTALL_DIR)" command2foo2lava-pjl
	(cd $(PKG_BUILD_DIR) && ./getweb 300)
	(cd $(PKG_BUILD_DIR) && ./getweb 310)
	(cd $(PKG_BUILD_DIR) && ./getweb 315)
	(cd $(PKG_BUILD_DIR) && ./getweb 325)
	(cd $(PKG_BUILD_DIR) && ./getweb 360)
	(cd $(PKG_BUILD_DIR) && ./getweb 365)
	(cd $(PKG_BUILD_DIR) && ./getweb 2160)
	(cd $(PKG_BUILD_DIR) && ./getweb 3160)
	(cd $(PKG_BUILD_DIR) && ./getweb 3175)
	(cd $(PKG_BUILD_DIR) && ./getweb 3185)
	(cd $(PKG_BUILD_DIR) && ./getweb 6110)
	(cd $(PKG_BUILD_DIR) && ./getweb 600)
	(cd $(PKG_BUILD_DIR) && ./getweb 610)
endef

define Package/foo2qpdl/install
	$(INSTALL_DIR) $(1)/bin $(1)/usr/bin \
		$(1)/usr/lib/cups/filter \
		$(1)/usr/share

	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/bin/usb_printerid $(1)/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/** $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/command2foo2lava-pjl $(1)/usr/bin/
	$(CP) $(PKG_INSTALL_DIR)/usr/share/** $(1)/usr/share/
	rm -rf $(1)/usr/share/doc $(1)/usr/share/man
	$(LN) /usr/bin/command2foo2lava-pjl $(1)/usr/lib/cups/filter/command2foo2lava-pjl
endef

$(eval $(call BuildPackage,foo2qpdl))
