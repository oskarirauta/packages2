include $(TOPDIR)/rules.mk

PKG_NAME:=ugrep
PKG_VERSION:=7.8.2
PKG_RELEASE:=1

PKG_SOURCE:=v$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/Genivia/ugrep/archive/refs/tags
PKG_HASH:=f991cc6c61dbc5af5a3b3939083e917df4113509549670fb400d121f639f69f6

PKG_BUILD_DIR:=$(BUILD_DIR)/ugrep-$(PKG_VERSION)

PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>
PKG_LICENSE:=BSD-3-Clause
PKG_LICENSE_FILES:=LICENSE.txt

PKG_INSTALL:=1
PKG_FORTIFY_SOURCE:=0

include $(INCLUDE_DIR)/package.mk

define Package/ugrep
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Ultra fast grep with interactive query UI
  URL:=https://ugrep.com/
  DEPENDS:=+libpcre2 +zlib +liblzma +liblz4 +libzstd +libstdcpp
endef

define Package/ugrep/description
  Ultra fast grep with interactive query UI, fuzzy search, and support for
  searching compressed files (.gz, .xz, .lz4, .zst). Includes ug, ug+,
  ugrep+, and ugrep-indexer.
endef

define Package/ugrep-zsh-completion
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=ZSH extensions
  TITLE:=Zsh completions for ugrep
  URL:=https://ugrep.com/
  DEPENDS:=+zsh +ugrep
endef

define Package/ugrep-zsh-completion/description
  Zsh completion functions for ugrep and its aliases
  (ug, ug+, ugrep, ugrep+, ugrep-indexer).
endef

CONFIGURE_ARGS += \
	--without-brotli \
	--without-bzlib \
	--without-bzip3 \
	--without-boost-regex \
	--with-bash-completion-dir=no \
	--with-fish-completion-dir=no \
	--with-zsh-completion-dir=/usr/share/zsh/site-functions \
	--with-pcre2=$(STAGING_DIR)/usr \
	--with-zlib=$(STAGING_DIR)/usr \
	--with-lzma=$(STAGING_DIR)/usr \
	--with-lz4=$(STAGING_DIR)/usr \
	--with-zstd=$(STAGING_DIR)/usr

define Package/ugrep/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/ugrep $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/ug $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/ugrep+ $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/ug+ $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/ugrep-indexer $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/usr/share/ugrep
	$(CP) $(PKG_INSTALL_DIR)/usr/share/ugrep/patterns $(1)/usr/share/ugrep/
endef

define Package/ugrep-zsh-completion/install
	$(INSTALL_DIR) $(1)/usr/share/zsh/site-functions
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/share/zsh/site-functions/_ug $(1)/usr/share/zsh/site-functions/
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/share/zsh/site-functions/_ug+ $(1)/usr/share/zsh/site-functions/
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/share/zsh/site-functions/_ugrep $(1)/usr/share/zsh/site-functions/
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/share/zsh/site-functions/_ugrep+ $(1)/usr/share/zsh/site-functions/
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/share/zsh/site-functions/_ugrep-indexer $(1)/usr/share/zsh/site-functions/
endef

$(eval $(call BuildPackage,ugrep))
$(eval $(call BuildPackage,ugrep-zsh-completion))
