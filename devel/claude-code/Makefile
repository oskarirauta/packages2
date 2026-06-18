include $(TOPDIR)/rules.mk

PKG_NAME:=claude-code
PKG_VERSION:=2.1.181
PKG_RELEASE:=1

# Platform-specific native binary from the npm registry.
# Only linux-x64-musl is handled here; add linux-arm64-musl with its own
# PKG_HASH if aarch64 targets are needed.
PKG_SOURCE:=claude-code-linux-x64-musl-$(PKG_VERSION).tgz
PKG_SOURCE_URL:=https://registry.npmjs.org/@anthropic-ai/claude-code-linux-x64-musl/-
PKG_HASH:=7bf1807d002ae3dbf32bd7aa85c16c0fed62ec4da07c371c93dcd47e6fa91fe6

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>
PKG_LICENSE:=SEE LICENSE IN LICENSE.md
PKG_LICENSE_FILES:=LICENSE.md

include $(INCLUDE_DIR)/package.mk

define Package/claude-code
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Claude Code - AI coding assistant CLI
  URL:=https://claude.ai/code
  DEPENDS:=@x86_64
endef

define Package/claude-code/description
  Claude Code is Anthropic's official CLI for AI-assisted software
  development. Self-contained musl binary, no Node.js required.
endef

# npm tarballs extract to a "package/" subdirectory; strip it so the binary
# lands directly in PKG_BUILD_DIR.
define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(TAR) -xzf $(DL_DIR)/$(PKG_SOURCE) -C $(PKG_BUILD_DIR) --strip-components=1
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/claude-code/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/claude $(1)/usr/bin/
	$(STAGING_DIR_HOST)/bin/patchelf --replace-needed libc.musl-x86_64.so.1 ld-musl-x86_64.so.1 $(1)/usr/bin/claude
endef

$(eval $(call BuildPackage,claude-code))
