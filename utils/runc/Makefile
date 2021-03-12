include $(TOPDIR)/rules.mk

PKG_NAME:=runc
PKG_VERSION:=1.0.0-rc93
PKG_RELEASE:=2
PKG_LICENSE:=Apache-2.0
PKG_LICENSE_FILES:=LICENSE

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/opencontainers/runc.git
PKG_SOURCE_DATE:=2021-03-11
PKG_SOURCE_VERSION:=249bca0a1316129dcd5bd38b5d75572274181cb5
PKG_MIRROR_HASH:=ae048b3117ef937b8ac90d14cbe5b41df392b45b5ab00840248bd43a39cdac1c

PKG_BUILD_DEPENDS:=golang/host
PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1
PKG_USE_MIPS16:=0

GO_PKG:=github.com/opencontainers/runc

include $(INCLUDE_DIR)/package.mk
include ../../../packages/lang/golang/golang-package.mk

define Package/runc
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=runc container runtime
  URL:=https://www.opencontainers.org/
  DEPENDS:=$(GO_ARCH_DEPENDS) +KERNEL_SECCOMP_FILTER:libseccomp
endef

define Package/runc/description
runc is a CLI tool for spawning and running containers according to the OCI specification.
endef

GO_PKG_INSTALL_ALL:=1
MAKE_PATH:=$(GO_PKG_WORK_DIR_NAME)/build/src/$(GO_PKG)
MAKE_VARS += $(GO_PKG_VARS)
MAKE_FLAGS += COMMIT=$(PKG_SOURCE_VERSION)

ifeq ($(ARCH),mips)
MAKE_FLAGS += EXTRA_FLAGS='-buildmode=default'
endif

BUILDTAGS += selinux seccomp
MAKE_FLAGS += BUILDTAGS="selinux seccomp"

# Reset golang-package.mk overrides so we can use the Makefile
Build/Compile=$(call Build/Compile/Default)

define Package/runc/install
	$(INSTALL_DIR) $(1)/usr/sbin/
	$(INSTALL_BIN) $(GO_PKG_BUILD_DIR)/src/$(GO_PKG)/runc $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,runc))
