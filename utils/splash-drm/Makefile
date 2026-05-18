include $(TOPDIR)/rules.mk

PKG_NAME:=splash-drm
PKG_VERSION:=3.2.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/oskarirauta/$(PKG_NAME)/releases/download/$(PKG_VERSION)/
PKG_HASH:=3cb6c2112dfd9bd716e425d2d9b3411c77801404b8ad974effc4a585204a7e6a

PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>
PKG_BUILD_DEPENDS:=libdrm
PKG_RUNTIME_DEPENDS:=

include $(INCLUDE_DIR)/package.mk

define Package/splash-drm/config
  source "$(SOURCE)/Config.in"
endef

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

TARGET_CFLAGS += -I$(PKG_BUILD_DIR) -I$(PKG_BUILD_DIR)/include -I$(STAGING_DIR)/usr/include/libdrm \
		 -O2 -Wall -Wextra -std=c99 -D_GNU_SOURCE -Wno-unused-function -static

TARGET_LDFLAGS += -lm -static

define Build/Configure
endef

define Package/splash-drm/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/splash-drm $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/splash-ctl $(1)/usr/bin/
ifeq ($(CONFIG_SPLASH_DRM_FONTS),y)
	$(INSTALL_DIR) $(1)/usr/share/splash/fonts
	$(INSTALL_DATA) ./files/fonts/** $(1)/usr/share/splash/fonts/
endif
ifeq ($(CONFIG_SPLASH_DRM_SAMPLE_IMAGE),y)
	$(INSTALL_DIR) $(1)/usr/share/splash/images
	$(INSTALL_DATA) ./files/openwrt.png $(1)/usr/share/splash/images/splash.png
endif
endef

$(eval $(call BuildPackage,splash-drm))
