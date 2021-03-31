
include $(TOPDIR)/rules.mk

PKG_NAME:=nasm
PKG_VERSION:=2.5.05
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-x86_64-precompiled.tar.gz
PKG_SOURCE_URL:=https://github.com/oskarirauta/owrt-packages2/raw/main/devel/nasm/files
PKG_HASH:=88df3daa263cd09ec18d4dd7693d5092bc6edd9cbbf8f021a52adfa982f6a8b0

PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>
PKG_LICENSE:=BSD-2-Clause
PKG_LICENSE_FILES:=LICENSE
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/nasm
  SECTION:=devel
  CATEGORY:=Development
  TITLE:=nasm
  DEPENDS:=@TARGET_x86_64
endef

define Package/nasm/description
  NASM, the Netwide Assembler: an assembler targetting the
  Intel x86 series of processors, with portable source.
endef

define Build/Configure
	# Nothing to configure
endef

define Build/Compile
	# Nothing to build
endef

define Build/Install
	# Nothing to install
endef

define Package/nasm/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/ndisasm $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/nasm $(1)/usr/bin/
endef

$(eval $(call BuildPackage,nasm))
