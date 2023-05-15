include $(TOPDIR)/rules.mk

PKG_NAME:=upodman
PKG_VERSION:=0.98.2
PKG_RELEASE:=2

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/oskarirauta/upodman.git
PKG_SOURCE_DATE:=2023-05-15
PKG_SOURCE_VERSION:=175aaee4af2737de048490bb6428bd511a884374
PKG_MIRROR_HASH:=skip

PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk

define Package/upodman
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Podman manager via Ubus
  URL:=https://github.com/oskarirauta/upodman
  DEPENDS:=+libjson-c +libstdcpp +libubox +libuci +libblobmsg-json +libubus +libcurl +podman
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
endef

$(eval $(call BuildPackage,upodman))
