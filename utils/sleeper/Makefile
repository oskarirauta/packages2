include $(TOPDIR)/rules.mk

PKG_NAME:=sleeper
PKG_VERSION:=1.0.0
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/sleeper
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Simple program to wait given seconds without depencies
endef

define Package/sleeper/description
 Sleeper is built statically without any depencies
endef

define Build/Configure
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CC)" \
		CFLAGS="$(TARGET_CFLAGS) -Wall" \
		LDFLAGS="$(TARGET_LDFLAGS)"
endef

define Package/sleeper/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/sleeper $(1)/usr/bin/
endef

$(eval $(call BuildPackage,sleeper))
