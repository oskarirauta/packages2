include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=ax206
PKG_RELEASE:=1

PKG_SOURCE_DATE:=2026-06-28
PKG_SOURCE_VERSION:=1594560d6e39f44ff277a4b92d89fc9091228084

PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>
PKG_LICENSE:=GPL-2.0-only
PKG_LICENSE_FILES:=LICENSE

include $(INCLUDE_DIR)/package.mk

define KernelPackage/ax206
  SUBMENU:=$(VIDEO_MENU)
  TITLE:=AX206/DPF driver
  DEPENDS:=@DISPLAY_SUPPORT +kmod-backlight +kmod-drm +kmod-drm-dma-helper +kmod-drm-client-lib +kmod-drm-display-helper
  FILES:=$(PKG_BUILD_DIR)/ax206.ko $(PKG_BUILD_DIR)/drm/ax206-drm.ko $(PKG_BUILD_DIR)/bl/ax206-bl.ko
  AUTOLOAD:=$(call AutoProbe,ax206 ax206-drm ax206-bl)
endef

include $(INCLUDE_DIR)/kernel-defaults.mk

define KernelPackage/ax206/description
  Kernel modules for AX206 Digital Photo Frame USB display
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

$(eval $(call KernelPackage,ax206))
