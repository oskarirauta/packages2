include $(TOPDIR)/rules.mk

PKG_NAME:=upodman
PKG_VERSION:=0.98.1
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/oskarirauta/upodman/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=c05eddfe584fcb4e7e1a8893d676356b6332fd3ee6d292793097d16dcc33e6b9

PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

include $(INCLUDE_DIR)/package.mk

define Package/upodman
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Podman manager via Ubus
  URL:=https://github.com/oskarirauta/upodman
  DEPENDS:=+libjson-c +libstdcpp +libubox +libuci +libblobmsg-json +libubus +podman
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

MAKE_VARS += \
	BUILD_DIR="$(PKG_BUILD_DIR)"

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
