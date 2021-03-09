include $(TOPDIR)/rules.mk

PKG_NAME:=docker
PKG_VERSION:=20.10.4
PKG_RELEASE:=1
PKG_LICENSE:=Apache-2.0
PKG_LICENSE_FILES:=LICENSE

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/docker/cli/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=2757ce724b80289d834618f338faa49c512240f3006d18119677f63ec12f0631
PKG_SOURCE_VERSION:=d3cb89e # SHA1 used within the docker executable

PKG_MAINTAINER:=Gerard Ryan <G.M0N3Y.2503@gmail.com>

PKG_BUILD_DEPENDS:=golang/host
PKG_BUILD_PARALLEL:=1
PKG_USE_MIPS16:=0

GO_PKG:=github.com/docker/cli

include $(INCLUDE_DIR)/package.mk
include ../../../packages/lang/golang/golang-package.mk

define Package/docker
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Docker Community Edition CLI
  URL:=https://www.docker.com/
  DEPENDS:=$(GO_ARCH_DEPENDS)
endef

define Package/docker/description
The CLI used in the Docker CE and Docker EE products.
endef

GOPATH=$(PKG_BUILD_DIR)/.go_work/build/src/
TAR_OPTIONS:=--strip-components 1 $(TAR_OPTIONS)
TAR_CMD=$(HOST_TAR) -C $(1) $(TAR_OPTIONS)
#GOFLAGS += ' -buildmode=pie -trimpath -mod=readonly -modcacherw -ldflags=-linkmode=external'

define Build/Compile
	( \
		cd $(PKG_BUILD_DIR); \
		ln -sf $(PKG_BUILD_DIR)/cli $(GOPATH)/github.com/cli; \
		DOCKER_BUILDTAGS="seccomp" DISABLE_WARN_OUTSIDE_CONTAINER=1 \
		AUTO_GOPATH=1 GOPATH=$(GOPATH) GO111MODULE=auto \
		VERSION="$(PKG_VERSION)" GITCOMMIT="$(PKG_SOURCE_VERSION)" \
		$(GO_PKG_VARS) \
		$(MAKE) VERSION="$(PKG_VERSION)" GITCOMMIT="$(PKG_SOURCE_VERSION)" DOCKER_BUILDTAGS="seccomp" dynbinary; \
	)
endef

define Package/docker/install
	$(INSTALL_DIR) $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/build/docker $(1)/usr/bin/docker
endef
$(eval $(call BuildPackage,docker))
