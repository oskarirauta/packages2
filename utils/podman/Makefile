include $(TOPDIR)/rules.mk

PKG_NAME:=podman
PKG_VERSION:=3.1.0
PKG_RELEASE:=2
PKG_LICENSE:=Apache-2.0
PKG_LICENSE_FILES:=LICENSE

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/containers/podman.git
PKG_SOURCE_DATE:=2021-03-18
PKG_SOURCE_VERSION:=629183bd7f0073dfcbfa4d611abc62a9c5711dab
PKG_MIRROR_HASH:=skip

#PKG_BUILD_DEPENDS:=golang/host protobuf/host
PKG_BUILD_PARALLEL:=1
#PKG_USE_MIPS16:=0
#PKG_INSTALL:=1

#GO_PKG:=github.com/containers/podman/
#GO_PKG_BUILD_PKG:=github.com/containers/podman/v3/cmd/podman/

#GO_PKG_TAGS:=seccomp,selinux,exclude_graphdriver_devicemapper,exclude_graphdriver_btrfs,exclude_graphdriver_overlay,apparmor

include $(INCLUDE_DIR)/package.mk
#include ../../../packages/lang/golang/golang-package.mk

define Download/default-registries
  URL:=https://raw.githubusercontent.com/projectatomic/registries/da9a9c87781823f45401ca49da04e269c9e3100e
  URL_FILE:=registries.fedora
  FILE:=registries.fedora-da9a9c8778
  HASH:=bc2b58c209aa8ca35b6814ec9a3c64716d4970b884ade460b65000e56024dfee
endef

define Download/default-policy
  URL:=https://raw.githubusercontent.com/containers/skopeo/362f70b056a1f5d2bd4184527a0ae0d20c4d35d3
  URL_FILE:=default-policy.json
  FILE:=default-policy.json-362f70b056
  HASH:=cddfaa8e6a7e5497b67cc0dd8e8517058d0c97de91bf46fff867528415f2d946
endef

define Package/podman
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Podman
  URL:=https://podman.io
  #DEPENDS:=$(GO_ARCH_DEPENDS) 
  DEPENDS:=+conmon +cni +cni-plugins +btrfs-progs +glib2 +gnupg2 +iptables +libgpg-error +libseccomp +libgpgme +nsenter +zoneinfo-simple +crun +uxc +cgroupfs-mount +iptables-mod-extra +iptables-mod-conntrack-label +kmod-nf-nat +kmod-veth +kmod-tun +kmod-nf-conntrack-netlink +kmod-nf-ipvs +kmod-ikconfig +kmod-fs-btrfs +libselinux +libsysfs +libapparmor
endef

define Package/podman-docker
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Podman docker compatibility
  URL:=https://podman.io
  #DEPENDS:=$(GO_ARCH_DEPENDS)
  DEPENDS:=+podman +libdevmapper
endef

define Package/podman/description
  Podman: A tool for managing OCI containers and pods
endef

define Package/podman/conffiles
/etc/containers
endef

CONFIGURE_VARS+= BUILDTAGS="exclude_graphdriver_devicemapper seccomp selinux apparmor"
MAKE_VARS+= CGO_ENABLED=1 SELINUXOPT=1 BUILDTAGS="exclude_graphdriver_devicemapper seccomp selinux apparmor"

define Build/Prepare
	$(call Build/Prepare/Default)
	$(eval $(call Download,default-registries))
	$(eval $(call Download,default-policy))
endef

define Build/Compile
	+$(MAKE_VARS) \
	$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR) $(MAKE_FLAGS);
endef

define Package/podman/install
	#$(call GoPackage/Package/Install/Bin,$(1))
	$(INSTALL_DIR) $(1)/etc/containers $(1)/etc/cni/net.d $(1)/etc/init.d $(1)/usr/bin
	$(INSTALL_CONF) $(DL_DIR)/default-policy.json-362f70b056 $(1)/etc/containers/policy.json
	$(INSTALL_CONF) $(DL_DIR)/registries.fedora-da9a9c8778 $(1)/etc/containers/registries.conf
	$(INSTALL_CONF) $(PKG_BUILD_DIR)/vendor/github.com/containers/storage/storage.conf $(1)/etc/containers/storage.conf
	$(INSTALL_CONF) ./files/containers.conf $(1)/etc/containers/containers.conf
	$(INSTALL_CONF) $(PKG_BUILD_DIR)/cni/87-podman-bridge.conflist $(1)/etc/cni/net.d/
	$(INSTALL_BIN) ./files/podman.init $(1)/etc/init.d/podman
	chmod a+x $(1)/etc/init.d/podman
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/podman $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/podman-remote $(1)/usr/bin/
endef

define Package/podman-docker/install
	$(INSTALL_DIR) $(1)/tmp/run/ $(1)/usr/bin/
	ln -s /var/run/podman/podman.sock $(1)/tmp/run/docker.sock
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/docker $(1)/usr/bin/
endef

#$(eval $(call GoBinPackage,podman))
$(eval $(call BuildPackage,podman))
$(eval $(call BuildPackage,podman-docker))
