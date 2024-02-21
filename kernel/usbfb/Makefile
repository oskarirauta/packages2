include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=usbfb
PKG_RELEASE:=1

PKG_SRC_VERSION:=v5
PKG_BUILD_DIR:=$(KERNEL_BUILD_DIR)/$(PKG_NAME)-$(PKG_SRC_VERSION)-$(PKG_RELEASE)

include $(INCLUDE_DIR)/package.mk

define KernelPackage/usb-framebuffer
  TITLE:=USB Framebuffer Support
  SUBMENU:=USB Support
  FILES:=$(PKG_BUILD_DIR)/usbfb.ko
  AUTOLOAD:=$(call AutoLoad,75,usbfb)
  DEPENDS:=+kmod-usb-core +kmod-usb-hid +kmod-fb +kmod-fb-tft
endef

include $(INCLUDE_DIR)/kernel-defaults.mk

define KernelPackage/usb-framebuffer/description
  Kernel module for VoCore Screen, USB framebuffer (/dev/fb1)
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) src/Makefile src/usbfb.c $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) $(KERNEL_MAKEOPTS) \
		M="$(PKG_BUILD_DIR)" \
		modules
endef

$(eval $(call KernelPackage,usb-framebuffer))
