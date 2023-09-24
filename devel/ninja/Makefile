include $(TOPDIR)/rules.mk

PKG_NAME:=ninja
PKG_VERSION:=1.11.1
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/ninja-build/ninja/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=31747ae633213f1eda3842686f83c2aa1412e0f5691d1c14dbbcc67fe7400cea

PKG_LICENSE_FILES:=COPYING

include $(INCLUDE_DIR)/package.mk

define Package/ninja
  SECTION:=devel
  CATEGORY:=Development
  TITLE:=ninja
  URL:=https://github.com/ninja-build/ninja
  DEPENDS:=+libstdcpp
endef

define Package/ninja/description
  Ninja is a small build system with a focus on speed.
endef

CONFIGURE_ARGS:=--verbose --platform=linux --host=linux --with-python=/usr/bin/python

define Build/Configure
endef

define Build/Compile
	cd $(PKG_BUILD_DIR) && \
		$(CONFIGURE_VARS) \
		CXX="$(TARGET_CXX)" \
		CXXFLAGS="$(TARGET_CXXFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		$(STAGING_DIR_HOST)/bin/$(PYTHON) configure.py --bootstrap $(CONFIGURE_ARGS)
endef

define Package/ninja/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ninja $(1)/bin/
endef

$(eval $(call BuildPackage,ninja))
