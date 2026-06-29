include $(TOPDIR)/rules.mk

PKG_NAME:=delta
PKG_VERSION:=0.18.2
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/dandavison/delta/tar.gz/$(PKG_VERSION)?
PKG_HASH:=64717c3b3335b44a252b8e99713e080cbf7944308b96252bc175317b10004f02

PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

PKG_BUILD_DEPENDS:=rust/host
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk
include ../../../packages/lang/rust/rust-package.mk

define Package/delta
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=A syntax-highlighting pager for git and diff output
  DEPENDS:=$(RUST_ARCH_DEPENDS) +zlib +oniguruma
  URL:=https://github.com/dandavison/delta
endef

define Package/delta/description
  delta is a viewer for git and diff output with syntax highlighting,
  line numbers, side-by-side view, and configurable themes. It
  integrates with git as a pager and can be used standalone.
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

define Package/delta/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/bin/delta $(1)/usr/bin/
endef

$(eval $(call RustBinPackage,delta))
$(eval $(call BuildPackage,delta))
