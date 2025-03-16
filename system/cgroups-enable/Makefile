include $(TOPDIR)/rules.mk

PKG_NAME:=cgroups-enable
PKG_VERSION:=1.0.0
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/cgroups-enable
  SECTION:=base
  CATEGORY:=Base system
  DEPENDS:=
  TITLE:=Init script to enable cgroup controllers
endef

define Package/cgroups-enable/description
 This package contains a init script that runs early and enables cgroup controllers.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
endef

define Build/Compile
endef

define Package/cgroups-enable/install
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/cgroups.sh $(1)/etc/init.d/cgroups
endef

$(eval $(call BuildPackage,cgroups-enable))
