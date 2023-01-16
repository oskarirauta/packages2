include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=boot-entry
PKG_VERSION:=0.9.1
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/oskarirauta/boot-entry/archive/refs/tags/
PKG_HASH:=c90493f986e5bd407132c5a3e174378c02cb80fa4eaee29875e06b4bba6afcc3

PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE
PKG_BUILD_PARALLEL:=1

PKG_BUILD_DIR:=$(KERNEL_BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

BOOT_ENTRY_AUTOLOAD:= \
	boot_entry

define KernelPackage/boot-entry
  SECTION:=kernel
  CATEGORY:=Kernel modules
  SUBMENU:=Other modules
  TITLE:=boot_entry cmdline parser
  DEPENDS:=@TARGET_x86_64
  URL:=https://github.com/oskarirauta/boot_entry
  VERSION:=$(LINUX_VERSION)+$(PKG_VERSION)-$(BOARD)-$(PKG_RELEASE)
  FILES:=$(PKG_BUILD_DIR)/boot_entry.$(LINUX_KMOD_SUFFIX)
  AUTOLOAD:=$(call AutoLoad,88,$(BOOT_ENTRY_AUTOLOAD))
endef

define Package/boot-entry/description
  Kernel module that exposes /proc/boot_entry
  which contains boot_entry kernel cmdline
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

$(eval $(call KernelPackage,boot-entry))
