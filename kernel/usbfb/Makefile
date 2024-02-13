include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=usbfb
#PKG_VERSION:=v5
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(KERNEL_BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

define KernelPackage/usb-framebuffer
  TITLE:=USB Framebuffer Support
  SECTION:=kernel
  CATEGORY:=Kernel modules
  SUBMENU:=USB Support
  KCONFIG:=CONFIG_USBFB_VOCORE_SCREEN
  FILES:=$(PKG_BUILD_DIR)/usbfb.ko
  AUTOLOAD:=$(call AutoLoad,01,usbfb)
  DEPENDS:=+kmod-usb-core +kmod-usb-hid +kmod-fb +kmod-fb-tft
endef

define KernelPackage/usb-framebuffer/description
  Kernel module for VoCore Screen, USB framebuffer (/dev/fb1)
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./files/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	+$(KERNEL_MAKE) $(PKG_JOBS) \
		M="$(PKG_BUILD_DIR)" \
		modules
endef

$(eval $(call KernelPackage,usb-framebuffer))
