include $(TOPDIR)/rules.mk

PKG_NAME:=runc
PKG_VERSION:=0.18
PKG_RELEASE:=1
PKG_LICENSE:=GPL-2.0

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/containers/crun.git
PKG_SOURCE_DATE:=2021-03-18
PKG_SOURCE_VERSION:=dc3b474d4329090496f0f99a00fc60596d623ab5
PKG_MIRROR_HASH:=ae048b3117ef937b8ac90d14cbe5b41df392b45b5ab00840248bd43a39cdac1c

PKG_BUILD_DEPENDS:=golang/host
PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1
PKG_USE_MIPS16:=0

GO_PKG:=github.com/containers/crun

include $(INCLUDE_DIR)/package.mk
include ../../../packages/lang/golang/golang-package.mk

define Package/crun
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=crun container runtime
  URL:=https://github.com/containers/crun
  DEPENDS:=$(GO_ARCH_DEPENDS) +KERNEL_SECCOMP_FILTER:libseccomp +yajl +libcap
endef

define Package/crun/description
  A fast and low-memory footprint OCI Container Runtime fully written in C.
endef

GO_PKG_INSTALL_ALL:=1
CONFIGURE_ARGS+= --disable-systemd
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

define Package/crun/install
	$(INSTALL_DIR) $(1)/usr/sbin/
	$(INSTALL_BIN) $(GO_PKG_BUILD_DIR)/src/$(GO_PKG)/crun $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,crun))
