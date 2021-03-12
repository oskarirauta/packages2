include $(TOPDIR)/rules.mk

PKG_NAME:=libnetwork
PKG_VERSION:=0.3
PKG_RELEASE:=1
PKG_LICENSE:=Apache-2.0
PKG_LICENSE_FILES:=LICENSE

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/moby/libnetwork.git
PKG_SOURCE_VERSION:=b3507428be5b458cb0e2b4086b13531fb0706e46
PKG_SOURCE_DATE:=2021-03-11
PKG_MIRROR_HASH:=e0e71ba6a6816b4dc9866d87abec955ee25de7f3f2c1241432ed38a3de74319a

PKG_BUILD_DEPENDS:=golang/host
PKG_BUILD_PARALLEL:=0
PKG_USE_MIPS16:=0

GO_PKG:=github.com/docker/libnetwork
GO_PKG_BUILD_PKG:= \
  github.com/docker/libnetwork/cmd/proxy \
  github.com/docker/libnetwork/cmd/dnet

include $(INCLUDE_DIR)/package.mk
include ../../../packages/lang/golang/golang-package.mk

define Package/libnetwork
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=networking for containers
  URL:=https://github.com/moby/libnetwork
  DEPENDS:=$(GO_ARCH_DEPENDS)
endef

define Package/libnetwork/description
Libnetwork provides a native Go implementation for connecting containers.
The goal of libnetwork is to deliver a robust Container Network Model that provides a consistent programming interface and the required network abstractions for applications.
endef

GO_PKG_BUILD_VARS += GO111MODULE=auto
GO_PKG_INSTALL_ALL:=1
MAKE_PATH=$(GO_PKG_WORK_DIR_NAME)/build/src/$(GO_PKG)
MAKE_VARS += $(GO_PKG_VARS)
MAKE_FLAGS += COMMIT=$(PKG_SOURCE_VERSION)
MAKE_FLAGS += all-local

define Package/libnetwork/install
	$(call GoPackage/Package/Install/Bin,$(PKG_INSTALL_DIR))

	$(INSTALL_DIR) $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/dnet $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/proxy $(1)/usr/bin/docker-proxy
endef

$(eval $(call GoBinPackage,libnetwork))
$(eval $(call BuildPackage,libnetwork))
