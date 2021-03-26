include $(TOPDIR)/rules.mk

PKG_NAME:=containerd
PKG_VERSION:=1.5.0
PKG_RELEASE:=1
PKG_LICENSE:=Apache-2.0
PKG_LICENSE_FILES:=LICENSE

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/containerd/containerd.git
PKG_SOURCE_DATE:=2021-03-11
PKG_SOURCE_VERSION:=ce8e8e89077f3b2ffbbc4b77944d92bd5b96e703
PKG_MIRROR_HASH:=skip

PKG_BUILD_DEPENDS:=golang/host
PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1
PKG_USE_MIPS16:=0

GO_PKG:=github.com/containerd/containerd

include $(INCLUDE_DIR)/package.mk
include ../../../packages/lang/golang/golang-package.mk

define Package/containerd
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=containerd container runtime
  URL:=https://containerd.io/
  DEPENDS:=$(GO_ARCH_DEPENDS) +btrfs-progs +crun
  MENU:=1
endef

define Package/containerd/description
An industry-standard container runtime with an emphasis on simplicity, robustness and portability
endef

GO_PKG_BUILD_VARS += GO111MODULE=auto
GO_PKG_INSTALL_ALL:=1
MAKE_PATH:=$(GO_PKG_WORK_DIR_NAME)/build/src/$(GO_PKG)
MAKE_VARS += $(GO_PKG_VARS)
MAKE_FLAGS += \
	VERSION=$(PKG_VERSION) \
	REVISION=$(PKG_SOURCE_VERSION)

ifeq ($(CONFIG_SELINUX),y)
MAKE_FLAGS += BUILDTAGS='selinux'
else
MAKE_FLAGS += BUILDTAGS=''
endif

# Reset golang-package.mk overrides so we can use the Makefile
Build/Compile=$(call Build/Compile/Default)

#define Build/Compile
#	( \
#		cd $(PKG_BUILD_DIR); \
#		ln -sf $(PKG_BUILD_DIR)/vendor/github.com/urfave $(PKG_BUILD_DIR)/.go_work/build/src/github.com/urfave; \
#		ln -sf $(PKG_BUILD_DIR)/vendor/github.com/cpuguy83 $(PKG_BUILD_DIR)/.go_work/build/src/github.com/cpuguy83; \
#		ln -sf $(PKG_BUILD_DIR)/vendor/github.com/russross $(PKG_BUILD_DIR)/.go_work/build/src/github.com/russross; \
#		$(MAKE_VARS) $(APP_TAGS) BUILDTAGS="$(APP_TAGS) $(BUILD_TAGS)" \
#		GO111MODULE=auto \
#		$(GO_PKG_VARS) \
#		$(MAKE) $(MAKE_FLAGS) $(APP_TAGS) BUILDTAGS="$(APP_TAGS) $(BUILD_TAGS)"; \
#	)
#endef

define Package/containerd/install
	$(INSTALL_DIR) $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/bin/{ctr,containerd,containerd-stress,containerd-shim,containerd-shim-runc-v1,containerd-shim-runc-v2} $(1)/usr/bin/
endef

$(eval $(call BuildPackage,containerd))
