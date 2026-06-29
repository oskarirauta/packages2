include $(TOPDIR)/rules.mk

PKG_NAME:=fzf
PKG_VERSION:=0.54.3
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/junegunn/fzf/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=6413f3916f8058b396820f9078b1336d94c72cbae39c593b1d16b83fcc4fdf74

PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

PKG_BUILD_DEPENDS:=golang/host
PKG_BUILD_PARALLEL:=1
PKG_BUILD_FLAGS:=no-mips16

GO_PKG:=github.com/junegunn/fzf
GO_PKG_BUILD_PKG:=$(GO_PKG)

include $(INCLUDE_DIR)/package.mk
include ../../../packages/lang/golang/golang-package.mk

define Package/fzf
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=A command-line fuzzy finder
  DEPENDS:=$(GO_ARCH_DEPENDS)
  URL:=https://github.com/junegunn/fzf
endef

define Package/fzf/description
  fzf is a general-purpose command-line fuzzy finder. It is an
  interactive filter for any list: files, command history, processes,
  git commits, and more.
endef

define Package/fzf/install
	$(call GoPackage/Package/Install/Bin,$(PKG_INSTALL_DIR))
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/fzf $(1)/usr/bin/
endef

$(eval $(call GoBinPackage,fzf))
$(eval $(call BuildPackage,fzf))
