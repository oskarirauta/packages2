include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=beadapanel
PKG_RELEASE:=1

PKG_SOURCE_DATE:=2026-05-13
PKG_SOURCE_VERSION:=3497204792b0ed82bec5236681fbcaa8868dd492

PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>
PKG_LICENSE:=GPL-2.0-only
PKG_LICENSE_FILES:=LICENSE

include $(INCLUDE_DIR)/package.mk

define KernelPackage/beadapanel
  SUBMENU:=$(VIDEO_MENU)
  TITLE:=NXElec BeadaPanel driver
  DEPENDS:=@DISPLAY_SUPPORT +kmod-backlight +kmod-drm +kmod-drm-dma-helper +kmod-drm-client-lib +kmod-drm-display-helper
  FILES:=$(PKG_BUILD_DIR)/beadapanel.ko $(PKG_BUILD_DIR)/drm/beada-drm.ko $(PKG_BUILD_DIR)/bl/beada-bl.ko
  AUTOLOAD:=$(call AutoProbe,beadapanel beada-drm beada-bl)
endef

include $(INCLUDE_DIR)/kernel-defaults.mk

define KernelPackage/beadapanel/description
  Kernel modules for NXElec's BeadaPanel USB screen
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

$(eval $(call KernelPackage,beadapanel))
