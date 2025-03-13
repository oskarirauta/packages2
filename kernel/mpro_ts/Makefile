include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=mpro_ts
PKG_RELEASE:=1

PKG_SRC_VERSION:=v5
PKG_BUILD_DIR:=$(KERNEL_BUILD_DIR)/$(PKG_NAME)-$(PKG_SRC_VERSION)-$(PKG_RELEASE)

include $(INCLUDE_DIR)/package.mk

define KernelPackage/mpro_ts
  SUBMENU:=$(VIDEO_MENU)
  TITLE:=MPro Touchscreen driver
  DEPENDS:=@DISPLAY_SUPPORT +kmod-backlight +kmod-drm-ttm \
	+kmod-drm-ttm-helper +kmod-drm-kms-helper +kmod-drm-display-helper \
	+kmod-drm-buddy \
	+LINUX_6_6:kmod-drm-exec +LINUX_6_6:kmod-drm-suballoc-helper
  FILES:=$(PKG_BUILD_DIR)/mpro_ts.ko
  AUTOLOAD:=$(call AutoProbe,mpro_ts)
endef

include $(INCLUDE_DIR)/kernel-defaults.mk

define KernelPackage/mpro_ts/description
  Touch screen driver for VoCore's Screen
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) src/** $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) $(KERNEL_MAKEOPTS) \
		M="$(PKG_BUILD_DIR)" \
		modules
endef

$(eval $(call KernelPackage,mpro_ts))
