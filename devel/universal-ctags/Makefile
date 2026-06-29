include $(TOPDIR)/rules.mk

PKG_NAME:=universal-ctags
PKG_VERSION:=6.1.0
PKG_RELEASE:=1

PKG_SOURCE:=ctags-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/universal-ctags/ctags/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=1eb6d46d4c4cace62d230e7700033b8db9ad3d654f2d4564e87f517d4b652a53
PKG_BUILD_DIR:=$(BUILD_DIR)/ctags-$(PKG_VERSION)

PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>
PKG_LICENSE:=GPL-2.0-or-later
PKG_LICENSE_FILES:=COPYING

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1
PKG_FIXUP:=autoreconf

include $(INCLUDE_DIR)/package.mk

CONFIGURE_ARGS += \
	--disable-pcre2

define Package/universal-ctags
  SECTION:=devel
  CATEGORY:=Development
  TITLE:=Universal ctags - maintained ctags implementation
  DEPENDS:=+jansson +libseccomp +libxml2 +libyaml
  URL:=https://ctags.io
endef

define Package/universal-ctags/description
  Universal Ctags generates an index (or tag) file of language objects
  found in source files that allows these items to be quickly and easily
  located by a text editor or other utility. It is a maintained
  implementation of ctags supporting many languages.
endef

define Package/universal-ctags/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/ctags $(1)/usr/bin/
	[ -f $(PKG_INSTALL_DIR)/usr/bin/readtags ] && \
		$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/readtags $(1)/usr/bin/ || true
endef

$(eval $(call BuildPackage,universal-ctags))
