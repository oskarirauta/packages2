include $(TOPDIR)/rules.mk

PKG_NAME:=splash-drm
PKG_VERSION:=3.5.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/oskarirauta/$(PKG_NAME)/releases/download/$(PKG_VERSION)/
PKG_HASH:=757de5c0a0083a2de922e1ac5f6ba473d87acdfc17710cb66dc8a2c07b69a987

PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>
PKG_BUILD_DEPENDS:=libdrm
PKG_RUNTIME_DEPENDS:=

include $(INCLUDE_DIR)/package.mk

define Package/splash-drm
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=DRM/KMS bootsplash
  DEPENDS:=
endef

define Package/splash-drm/description
  Splash-DRM is a visual boot process representer that survives from
  initramfs -> rootfs change in memory. Static build to avoid depends.
endef

TARGET_CFLAGS += -I$(PKG_BUILD_DIR) -I$(PKG_BUILD_DIR)/include -I$(PKG_BUILD_DIR)/qrcodegen/c -I$(STAGING_DIR)/usr/include/libdrm \
		 -O2 -Wall -Wextra -std=c99 -D_GNU_SOURCE -Wno-unused-function -static

TARGET_LDFLAGS += -lm -static

define Build/Configure
endef

define Package/splash-drm/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/splash-drm $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/splash-ctl $(1)/usr/bin/
endef

$(eval $(call BuildPackage,splash-drm))
