include $(TOPDIR)/rules.mk

PKG_NAME:=bat
PKG_VERSION:=0.24.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/sharkdp/bat/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=907554a9eff239f256ee8fe05a922aad84febe4fe10a499def72a4557e9eedfb

PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>
PKG_LICENSE:=MIT Apache-2.0
PKG_LICENSE_FILES:=LICENSE-MIT LICENSE-APACHE

PKG_BUILD_DEPENDS:=rust/host
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk
include ../../../packages/lang/rust/rust-package.mk

define Package/bat
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=A cat clone with syntax highlighting and Git integration
  DEPENDS:=$(RUST_ARCH_DEPENDS) +zlib +oniguruma
  URL:=https://github.com/sharkdp/bat
endef

define Package/bat/description
  bat is a cat clone with syntax highlighting and Git integration.
  It automatically pages output through less, shows line numbers,
  highlights the active search term, and supports many languages.
endef

define Build/Compile
	+$(CARGO_PKG_VARS) \
	RUSTONIG_SYSTEM_LIBONIG=1 \
	cargo install -v \
		--profile $(CARGO_PKG_PROFILE) \
		--root $(PKG_INSTALL_DIR) \
		--path "$(PKG_BUILD_DIR)" \
		$(if $(filter --jobserver%,$(PKG_JOBS)),,-j1) \
		$(CARGO_PKG_ARGS)
endef

define Package/bat/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/bin/bat $(1)/usr/bin/
endef

$(eval $(call RustBinPackage,bat))
$(eval $(call BuildPackage,bat))
