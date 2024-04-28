include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=mpro
PKG_RELEASE:=1

PKG_SRC_VERSION:=v5
PKG_BUILD_DIR:=$(KERNEL_BUILD_DIR)/$(PKG_NAME)-$(PKG_SRC_VERSION)-$(PKG_RELEASE)

include $(INCLUDE_DIR)/package.mk

define KernelPackage/drm-mpro
  SUBMENU:=$(VIDEO_MENU)
  TITLE:=MPro DRM driver
  DEPENDS:=@DISPLAY_SUPPORT +kmod-backlight +kmod-drm-ttm \
	+kmod-drm-ttm-helper +kmod-drm-kms-helper +kmod-drm-display-helper \
	+kmod-drm-buddy \
	+LINUX_6_6:kmod-drm-exec +LINUX_6_6:kmod-drm-suballoc-helper
  FILES:=$(PKG_BUILD_DIR)/mpro.ko
  AUTOLOAD:=$(call AutoProbe,mpro)
endef

include $(INCLUDE_DIR)/kernel-defaults.mk

define KernelPackage/drm-mpro/description
  DRM driver for VoCore Screen's with MPRO
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

$(eval $(call KernelPackage,drm-mpro))
