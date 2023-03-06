include $(TOPDIR)/rules.mk

PKG_NAME:=upodman
PKG_VERSION:=0.98.2
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/oskarirauta/upodman/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=4662f233f2c2e0400981299d09086e36d64ffec541233923e019702d378ff418

PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE
PKG_INSTALL:=0

include $(INCLUDE_DIR)/package.mk

define Package/upodman
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Podman manager via Ubus
  URL:=https://github.com/oskarirauta/upodman
  DEPENDS:=+libjson-c +libstdcpp +libubox +libuci +libblobmsg-json +libubus +podman @LIBCURL_UNIX_SOCKETS
endef

define Package/upodman/description
  upodman extends ubus with bindings to podman and allows
  controlling and statistics over pods and containers through
  ubus.

  upodman is written in C++ except for ubus parts which are
  externed from C. It uses staticly linked libcurl for unix
  socket operations, because system's default libcurl might
  not support unix sockets.
endef

define Build/Configure
endef

define Package/upodman/install
	$(INSTALL_DIR) $(1)/usr/bin $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/upodman $(1)/usr/bin/
	$(INSTALL_BIN) ./files/upodman.init $(1)/etc/init.d/upodman
	[ "$(CONFIG_NO_STRIP)" = "y" ] && \
		STRIP=$(STAGING_DIR_HOST)/bin/sstrip $(SCRIPT_DIR)/rstrip.sh $(1)/usr/bin
endef

$(eval $(call BuildPackage,upodman))
