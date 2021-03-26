include $(TOPDIR)/rules.mk

PKG_NAME:=crun
PKG_VERSION:=0.18
PKG_RELEASE:=1
PKG_LICENSE:=GPL-2.0

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/containers/crun.git
PKG_SOURCE_DATE:=2021-03-18
PKG_SOURCE_VERSION:=dc3b474d4329090496f0f99a00fc60596d623ab5
PKG_MIRROR_HASH:=ff778571653db8ff4e161b2fb9735c05a3d004d333bc77a45d6653dd5304da39

PKG_BUILD_PARALLEL:=1
PKG_FIXUP:=autoreconf
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/crun
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=crun container runtime
  URL:=https://github.com/containers/crun
  DEPENDS:=+libseccomp +libcap
endef

define Package/crun/description
  A fast and low-memory footprint OCI Container Runtime fully written in C.
endef

CONFIGURE_ARGS+= \
	--disable-systemd \
	--enable-embedded-yajl \
	--enable-caps \
	--enable-dl \
	--enable-seccomp \
	--enable-bpf

define Build/Prepare
	$(call Build/Prepare/Default)
	echo "#define GIT_VERSION \"$(PKG_SOURCE_VERSION)\"" > $(PKG_BUILD_DIR)/libocispec/src/git-version.h
endef

define Package/crun/install
	$(INSTALL_DIR) $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/crun $(1)/usr/bin/
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/bin $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/crun $(1)/usr/bin/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libcrun.* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,crun))
