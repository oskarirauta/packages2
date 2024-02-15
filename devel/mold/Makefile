
include $(TOPDIR)/rules.mk

PKG_NAME:=mold
PKG_VERSION:=2.4.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/rui314/mold/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=be65f3d785d32ece7b3204ecaa57810847fdd25c232cf704cbfff2dafb1ac107

PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/mold
  SECTION:=devel
  CATEGORY:=Development
  TITLE:=a modern linker
  DEPENDS:=+libatomic
endef

define Package/mold/description
  mold is a faster drop-in replacement for existing Unix linkers. It is several times quicker than the LLVM lld linker.
endef

CMAKE_OPTIONS += \
	-DMOLD_LTO=ON \
	-DMOLD_MOSTLY_STATIC=ON \
	-DMOLD_USE_SYSTEM_MIMALLOC=OFF \
	-DMOLD_USE_SYSTEM_TBB=OFF

define Package/mold/install
	$(INSTALL_DIR) $(1)/usr/bin $(1)/usr/lib/mold $(1)/usr/libexec/mold
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/mold $(1)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/mold/mold-wrapper.so $(1)/usr/lib/mold
	$(LN) mold $(1)/usr/bin/ld.mold
	$(LN) ../../bin/mold $(1)/usr/libexec/mold/ld
endef

$(eval $(call BuildPackage,mold))
