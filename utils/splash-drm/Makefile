include $(TOPDIR)/rules.mk

PKG_NAME:=splash-drm
PKG_VERSION:=5.0.2
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/oskarirauta/$(PKG_NAME)/releases/download/$(PKG_VERSION)/
PKG_HASH:=ea0aa408d54bc078f408d6c46d1e73cd95b6ab5f7ecad090d3f374a46a7a8d65

PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>

include $(INCLUDE_DIR)/package.mk

define Package/splash-drm
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=DRM/KMS bootsplash
  DEPENDS:=
endef

define Package/splash-drm/description
  Splash-DRM is a visual boot process representer that survives the
  initramfs -> rootfs switch in memory. The daemon (splash-drm) and its
  control client (splash-ctl) are linked statically so they have no runtime
  dependencies and can live in a minimal initramfs.
endef

define Package/splash-drm-progress
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=DRM bootsplash procd progress bridge
  DEPENDS:=+splash-drm +libubox +libuci +libubus
endef

define Package/splash-drm-progress/description
  Optional OpenWrt companion that subscribes to procd's service ubus object
  and drives the splash from real service.start events: it advances an
  N-of-total progress bar, shows the current service name, and runs a finished
  or fail sequence. Unlike the daemon it is a normal dynamically linked rootfs
  program (libubus/libubox/libuci), not part of the initramfs. Configured
  through UCI in /etc/config/splash.
endef

# /etc/config/splash is preserved across sysupgrade.
define Package/splash-drm-progress/conffiles
/etc/config/splash
endef

define Build/Configure
endef

# Two distinct artifacts from one source tree, built with different linkage:
#
#   * splash-drm + splash-ctl  -> `make static` (own obj-static/ dir, -static)
#     Self-contained; links only -lm. For the initramfs.
#   * ubus-progress            -> `make ubus-progress` (default obj/ dir)
#     Dynamic; links libubus/libubox/libuci. For the rootfs only.
#
# The two targets use separate object directories, so a single build tree
# produces both without clobbering each other.
define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CC)" \
		CFLAGS="$(TARGET_CFLAGS) -Wall -Wextra -std=c99 -Wno-unused-function" \
		LDFLAGS="$(TARGET_LDFLAGS) -lm" \
		static

	$(MAKE) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CC)" \
		CFLAGS="$(TARGET_CFLAGS) $(TARGET_CPPFLAGS) -Wall -Wextra -std=c99 -Wno-unused-function" \
		LDFLAGS="$(TARGET_LDFLAGS) -lm" \
		ubus-progress
endef

define Package/splash-drm/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/splash-drm $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/splash-ctl $(1)/usr/bin/
endef

define Package/splash-drm-progress/install
	$(INSTALL_DIR) $(1)/usr/bin $(1)/etc/init.d $(1)/etc/config
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ubus-progress/ubus-progress $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ubus-progress/splash-progress.init $(1)/etc/init.d/splash-progress
	$(INSTALL_CONF) $(PKG_BUILD_DIR)/ubus-progress/splash.config $(1)/etc/config/splash
endef

$(eval $(call BuildPackage,splash-drm))
$(eval $(call BuildPackage,splash-drm-progress))
