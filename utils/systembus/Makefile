include $(TOPDIR)/rules.mk

PKG_NAME:=systembus
PKG_VERSION:=2.0.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/oskarirauta/$(PKG_NAME)/tar.gz/$(PKG_VERSION)?
PKG_HASH:=a1aa2b47de626ecc083ba02487b690deadd98305e24196ff714bfd37c652b846

PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

include $(INCLUDE_DIR)/package.mk

define Package/systembus
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=System statistics via ubus
  DEPENDS:=+libubus +libubox +libblobmsg-json +libuci +libstdcpp +jsoncpp +SYSTEMBUS_SELINUX:libselinux
  MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>
endef

define Package/systembus/description
  systembus provides statistics of running system via
  ubus. systembus is written in C++.
endef

define Package/systembus/config
	source "$(SOURCE)/Config.in"
endef

define Build/Configure
endef

ifeq ($(CONFIG_SYSTEMBUS_SELINUX),y)
MAKE_VARS += selinux=1
endif

TARGET_CXXFLAGS += -Wno-builtin-declaration-mismatch

define Package/systembus/install
	$(INSTALL_DIR) $(1)/usr/bin $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/systembus $(1)/usr/bin/
	$(INSTALL_BIN) ./files/systembus.init $(1)/etc/init.d/systembus
endef

$(eval $(call BuildPackage,systembus))
