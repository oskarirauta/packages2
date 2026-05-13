include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=mpro
PKG_RELEASE:=1

PKG_SOURCE_DATE:=2026-05-13
PKG_SOURCE_VERSION:=90f990ef19772def776268a82ff593b25132c5e6

PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>
PKG_LICENSE:=GPL-2.0-only
PKG_LICENSE_FILES:=LICENSE

include $(INCLUDE_DIR)/package.mk

define KernelPackage/mpro
  SUBMENU:=$(VIDEO_MENU)
  TITLE:=VoCore MPro driver
  DEPENDS:=@DISPLAY_SUPPORT +kmod-backlight +kmod-drm +kmod-drm-dma-helper +kmod-drm-client-lib +kmod-drm-display-helper +kmod-lib-lz4 +kmod-lib-lz4hc
  FILES:=$(PKG_BUILD_DIR)/mpro.ko $(PKG_BUILD_DIR)/drm/mpro_drm.ko $(PKG_BUILD_DIR)/backlight/mpro_backlight.ko $(PKG_BUILD_DIR)/touchscreen/mpro_touchscreen.ko
  AUTOLOAD:=$(call AutoProbe,mpro mpro_drm mpro_backlight mpro_touchscreen)
endef

include $(INCLUDE_DIR)/kernel-defaults.mk

define KernelPackage/mpro/description
  Kernel modules for VoCore Screen's with MPRO
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

$(eval $(call KernelPackage,mpro))
