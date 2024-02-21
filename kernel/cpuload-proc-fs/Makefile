include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=cpuload-proc-fs
PKG_RELEASE:=1

PKG_SRC_VERSION:=0.9.0
PKG_BUILD_DIR:=$(KERNEL_BUILD_DIR)/$(PKG_NAME)-$(PKG_SRC_VERSION)-$(PKG_RELEASE)

include $(INCLUDE_DIR)/package.mk

define KernelPackage/cpuload-proc-fs
  TITLE:=cpuloads from procfs
  SUBMENU:=Other modules
  FILES:=$(PKG_BUILD_DIR)/cpuload.ko
  AUTOLOAD:=$(call AutoLoad,90,cpuload)
endef

include $(INCLUDE_DIR)/kernel-defaults.mk

define KernelPackage/cpuload-proc-fs/description
  Kernel module for retrieving cpu loads from procfs
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) files/Makefile files/cpuload.c $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) $(KERNEL_MAKEOPTS) \
		M="$(PKG_BUILD_DIR)" \
		modules
endef

$(eval $(call KernelPackage,cpuload-proc-fs))
