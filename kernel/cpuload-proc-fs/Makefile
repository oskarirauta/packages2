include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=cpuload-proc-fs
PKG_RELEASE:=1

PKG_SRC_VERSION:=0.9.0
PKG_BUILD_DIR:=$(KERNEL_BUILD_DIR)/$(PKG_NAME)-$(PKG_SRC_VERSION)-$(PKG_RELEASE)

include $(INCLUDE_DIR)/package.mk

define KernelPackage/cpuload-proc-fs
  TITLE:=cpuloads from procfs
  SECTION:=kernel
  CATEGORY:=Kernel modules
  SUBMENU:=Other modules
  KCONFIG:=CONFIG_PROC_FS_CPULOAD
  FILES:=$(PKG_BUILD_DIR)/cpuload.ko
  AUTOLOAD:=$(call AutoLoad,01,cpuload)
  DEPENDS:=
endef

define KernelPackage/cpuload-proc-fs/description
  Kernel module for retrieving cpu loads from procfs
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

$(eval $(call KernelPackage,cpuload-proc-fs))
