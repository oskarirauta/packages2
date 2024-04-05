include $(TOPDIR)/rules.mk

PKG_NAME:=infra-protocol
PKG_VERSION:=20240405
PKG_RELEASE:=1

PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>

include $(INCLUDE_DIR)/package.mk

define Package/infra-protocol
  SECTION:=net
  CATEGORY:=Network
  TITLE:=infra protocol
  PKGARCH:=all
endef

define Package/infra-protocol/description
  infra protocol allows automatic veth endpoint
  setup for new namespace. Protocol also supports
  running a script after setup, which can be used
  with, for example; ucx - to create a group of
  containers running in same namespaces, similar
  to pods known from podman.
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/infra-protocol/install
	$(INSTALL_DIR) $(1)/lib/netifd/proto
	$(INSTALL_BIN) ./files/infra.sh $(1)/lib/netifd/proto/infra.sh
endef

$(eval $(call BuildPackage,infra-protocol))
