include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=boot-variant
PKG_VERSION:=0.99
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/oskarirauta/boot-variant.git
PKG_SOURCE_DATE:=2023-01-20
PKG_SOURCE_VERSION:=aa5c439e69ba9d7c899def942eec8602c70b9167
PKG_SOURCE_HASH:=skip

PKG_LICENSE:=GPL
PKG_LICENSE_FILES:=LICENSE
PKG_BUILD_PARALLEL:=1

PKG_BUILD_DIR:=$(KERNEL_BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

BOOT_VARIANT_AUTOLOAD:= \
	boot_variant

define KernelPackage/boot-variant
  SECTION:=kernel
  CATEGORY:=Kernel modules
  SUBMENU:=Other modules
  TITLE:=boot variant cmdline parser
  URL:=https://github.com/oskarirauta/boot_variant
  VERSION:=$(LINUX_VERSION)+$(PKG_VERSION)-$(BOARD)-$(PKG_RELEASE)
  FILES:=$(PKG_BUILD_DIR)/boot_variant.$(LINUX_KMOD_SUFFIX)
  AUTOLOAD:=$(call AutoLoad,88,$(BOOT_VARIANT_AUTOLOAD))
endef

define Package/boot-variant/description
  Kernel module that exposes /proc/boot_variant
  which contains boot_variant.name kernel cmdline
  parameter
endef

MAKE_OPTS:= \
	ARCH="$(LINUX_KARCH)" \
	CROSS_COMPILE="$(TARGET_CROSS)" \
	M="$(PKG_BUILD_DIR)"

define Build/Compile
	$(MAKE) -C "$(LINUX_DIR)" \
		$(MAKE_OPTS) \
		CONFIG_BOOT_ENTRY=m \
		modules
endef

$(eval $(call KernelPackage,boot-variant))
