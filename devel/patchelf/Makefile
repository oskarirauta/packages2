include $(TOPDIR)/rules.mk

PKG_NAME:=patchelf
PKG_VERSION:=0.18.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=https://github.com/NixOS/patchelf/releases/download/$(PKG_VERSION)/
PKG_HASH:=1952b2a782ba576279c211ee942e341748fdb44997f704dd53def46cd055470b

PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>
PKG_LICENSE:=GPL-3.0-or-later
PKG_LICENSE_FILES:=COPYING

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/patchelf
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Small utility to modify ELF binaries
  URL:=https://github.com/NixOS/patchelf
  DEPENDS:=+libstdcpp
endef

define Package/patchelf/description
  PatchELF is a small utility to modify the dynamic linker and RPATH of
  ELF executables. Supports adding/removing NEEDED entries, changing the
  interpreter, and setting RPATH/RUNPATH.
endef

define Package/patchelf-zsh-completion
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Zsh completion for patchelf
  URL:=https://github.com/NixOS/patchelf
  DEPENDS:=+patchelf +zsh
endef

define Package/patchelf-zsh-completion/description
  Zsh tab completion for patchelf.
endef

define Package/patchelf/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/patchelf $(1)/usr/bin/
endef

define Package/patchelf-zsh-completion/install
	$(INSTALL_DIR) $(1)/usr/share/zsh/site-functions
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/completions/zsh/_patchelf \
		$(1)/usr/share/zsh/site-functions/_patchelf
endef

$(eval $(call BuildPackage,patchelf))
$(eval $(call BuildPackage,patchelf-zsh-completion))
