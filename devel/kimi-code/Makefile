include $(TOPDIR)/rules.mk

PKG_NAME:=kimi-code
PKG_VERSION:=0.18.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-linux-x64.zip
PKG_SOURCE_URL_FILE:=kimi-code-linux-x64.zip
PKG_HASH:=93f694264918dadb08755546f3c32128eaf3b0ba2727b277ea3c7445029f4833
PKG_SOURCE_URL:=https://github.com/MoonshotAI/kimi-code/releases/download/%40moonshot-ai%2Fkimi-code%40$(PKG_VERSION)/

PKG_BUILD_DEPENDS:=python3/host node/host

PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

NODE_FALLBACK:=24.15.0
NODE_ARCH:=$(if $(filter aarch64,$(ARCH)),arm64,x64)
MUSL_ARCH:=$(if $(filter aarch64,$(ARCH)),aarch64,x86_64)

PYTHON:=$(STAGING_DIR_HOSTPKG)/bin/python3
NPX:=$(STAGING_DIR_HOSTPKG)/lib/node_modules/npm/bin/npx
FUSE:=NODE_SEA_FUSE_fce680ab2cc467b6e072b8b5df1996b2

include $(INCLUDE_DIR)/package.mk

define Package/kimi-code
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Kimi Code - AI coding assistant CLI
  URL:=https://www.kimi.com
  DEPENDS:=@(x86_64||aarch64) +libstdcpp
endef

define Package/kimi-code/description
  A coding-focused perk of the Kimi membership, engineered to
  drop into any dev workflow, answer faster and more reliably,
  and get programming tasks done in record time.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	unzip -q -d $(PKG_BUILD_DIR) $(DL_DIR)/$(PKG_SOURCE)
	mv $(PKG_BUILD_DIR)/kimi $(PKG_BUILD_DIR)/kimi-glibc
	$(CP) ./files/extract_blob.py ./files/fetch-node.sh $(PKG_BUILD_DIR)/
	chmod +x $(PKG_BUILD_DIR)/fetch-node.sh
endef

define Build/Compile
	# 1. resolve + fetch the matching musl node (cached in DL_DIR) -> node-bin
	$(PKG_BUILD_DIR)/fetch-node.sh \
		$(PKG_BUILD_DIR)/kimi-glibc $(NODE_ARCH) $(DL_DIR) \
		$(PKG_BUILD_DIR)/node-bin $(NODE_FALLBACK)
	# 2. carve the (arch-independent) SEA blob out of the glibc binary
	$(PYTHON) $(PKG_BUILD_DIR)/extract_blob.py \
		$(PKG_BUILD_DIR)/kimi-glibc $(PKG_BUILD_DIR)/sea-prep.blob
	# 3. inject the blob into the musl node (postject runs under host node)
	$(CP) $(PKG_BUILD_DIR)/node-bin $(PKG_BUILD_DIR)/kimi
	cd $(PKG_BUILD_DIR) && $(NPX) -y postject kimi NODE_SEA_BLOB \
		sea-prep.blob --sentinel-fuse $(FUSE)
	chmod 0755 $(PKG_BUILD_DIR)/kimi
endef

define Package/kimi-code/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/kimi $(1)/usr/bin/kimi
	$(STAGING_DIR_HOST)/bin/patchelf --replace-needed \
		libc.musl-$(MUSL_ARCH).so.1 ld-musl-$(MUSL_ARCH).so.1 $(1)/usr/bin/kimi
endef

$(eval $(call BuildPackage,kimi-code))
