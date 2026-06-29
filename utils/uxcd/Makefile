include $(TOPDIR)/rules.mk

PKG_NAME:=uxcd
PKG_VERSION:=2.0.0
PKG_RELEASE:=1

# The release tarball is built recursively (submodules included) by the repo's
# release workflow, so it carries the vendored C++ libraries and docker2uxcd.
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/oskarirauta/$(PKG_NAME)/releases/download/$(PKG_VERSION)/
PKG_HASH:=d21918ed57e1496ea3a7be40fc2af370e55e06f0646d18a3a1d3aee3787a5e25

PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>

include $(INCLUDE_DIR)/package.mk

define Package/uxcd
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=Containers
  TITLE:=Container supervisor for OpenWrt (uxc alternative)
  URL:=https://github.com/oskarirauta/uxcd
  # libstdcpp: uxcd/uxc/uxe are C++ (g++), so they link libstdc++.so. The
  # daemon and CLIs are otherwise self-contained (libubox/libubus) and never
  # shell out. `uxc pull`/`uxc build` hand off to the optional docker2uxcd
  # package, which carries the heavier jq/ca-bundle deps for that job.
  DEPENDS:=+libubox +libubus +libblobmsg-json +libstdcpp
  CONFLICTS:=uxc
endef

define Package/uxcd/description
  uxcd is a small supervisor daemon for OpenWrt containers and a drop-in-style
  replacement for the stock uxc: it owns the container lifecycle (launches ujail
  itself) with intent-aware restart and crash backoff, re-adopts containers
  across a uxcd restart, captures logs, runs tcp/http/resource/exec healthchecks,
  emits ubus events, and supports host / isolated / shared-netns (infra)
  networking plus per-container volumes/devices/env/resources kept in the
  registry (so they survive image updates).

  Ships three programs - uxcd (daemon), uxc (CLI) and uxe (exec/shell) - and the
  netifd "netns" protocol. `uxc pull` (fetch a registry image) and `uxc build`
  (build from a Dockerfile, no Docker daemon) additionally need the separate
  docker2uxcd package; everything else works without it.
endef

# /etc/config/uxcd is preserved across sysupgrade.
define Package/uxcd/conffiles
/etc/config/uxcd
endef

define Build/Configure
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		CXX="$(TARGET_CXX)" \
		CXXFLAGS="$(TARGET_CXXFLAGS) $(TARGET_CPPFLAGS) --std=c++17 -fPIC" \
		LDFLAGS="$(TARGET_LDFLAGS)"
endef

define Package/uxcd/install
	$(INSTALL_DIR) $(1)/usr/sbin $(1)/sbin $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/uxcd $(1)/usr/sbin/uxcd
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/uxc  $(1)/sbin/uxc
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/uxe  $(1)/usr/bin/uxe

	$(INSTALL_DIR) $(1)/lib/netifd/proto
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/netifd/netns.sh $(1)/lib/netifd/proto/netns.sh

	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/uxcd.init $(1)/etc/init.d/uxcd

	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) $(PKG_BUILD_DIR)/uxcd.config $(1)/etc/config/uxcd

	# Prometheus metrics endpoint (served by uhttpd at /cgi-bin/uxcd-metrics)
	$(INSTALL_DIR) $(1)/www/cgi-bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/uxcd-metrics $(1)/www/cgi-bin/uxcd-metrics
endef

define Package/luci-proto-netns
  SECTION:=luci
  CATEGORY:=LuCI
  SUBMENU:=3. Applications
  TITLE:=Support for the uxcd netns protocol
  DEPENDS:=+luci-base +uxcd
  PKGARCH:=all
endef

define Package/luci-proto-netns/description
  LuCI protocol handler that adds the uxcd "netns" shared-namespace network
  protocol to the LuCI interface configuration, rendering its options as form
  fields (address/netmask/gateway/dns on General; name/device/peer on Advanced).
endef

define Package/luci-proto-netns/install
	$(INSTALL_DIR) $(1)/www/luci-static/resources/protocol
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/luci/proto-netns/netns.js $(1)/www/luci-static/resources/protocol/netns.js
endef

define Package/luci-app-uxcd
  SECTION:=luci
  CATEGORY:=LuCI
  SUBMENU:=3. Applications
  TITLE:=Container management UI for uxcd
  DEPENDS:=+luci-base +uxcd
  # uxcd replaces uxc (uxcd CONFLICTS:=uxc), so their LuCI front-ends must not
  # coexist either - both drive container management and would clash.
  CONFLICTS:=luci-app-uxc
  PKGARCH:=all
endef

define Package/luci-app-uxcd/description
  LuCI web interface for uxcd: a "Containers" tab listing every supervised
  container with live state/health/memory/cpu, start/stop/restart buttons and a
  per-container detail + log view, plus a compact "remote control" widget on the
  Status overview page. It talks to the uxcd ubus object over rpcd (polling) and
  ships an rpcd ACL granting the LuCI user the needed uxcd ubus methods.
endef

define Package/luci-app-uxcd/install
	$(INSTALL_DIR) $(1)/www/luci-static/resources/view/uxcd
	$(INSTALL_DIR) $(1)/www/luci-static/resources/view/status/include
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/luci/app-uxcd/htdocs/luci-static/resources/uxcd.js \
		$(1)/www/luci-static/resources/uxcd.js
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/luci/app-uxcd/htdocs/luci-static/resources/view/uxcd/overview.js \
		$(1)/www/luci-static/resources/view/uxcd/overview.js
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/luci/app-uxcd/htdocs/luci-static/resources/view/uxcd/storage.js \
		$(1)/www/luci-static/resources/view/uxcd/storage.js
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/luci/app-uxcd/htdocs/luci-static/resources/view/status/include/30_containers.js \
		$(1)/www/luci-static/resources/view/status/include/30_containers.js

	$(INSTALL_DIR) $(1)/usr/share/luci/menu.d
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/luci/app-uxcd/root/usr/share/luci/menu.d/luci-app-uxcd.json \
		$(1)/usr/share/luci/menu.d/luci-app-uxcd.json

	$(INSTALL_DIR) $(1)/usr/share/rpcd/acl.d
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/luci/app-uxcd/root/usr/share/rpcd/acl.d/luci-app-uxcd.json \
		$(1)/usr/share/rpcd/acl.d/luci-app-uxcd.json
endef

define Package/docker2uxcd
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=Containers
  TITLE:=Image converter / Dockerfile builder for uxcd
  URL:=https://github.com/oskarirauta/docker2uxc
  # jq (JSON) + ca-bundle (cert store for registry HTTPS). The fetcher is the
  # system wget - GNU wget built with SSL, or uclient-fetch which provides a
  # wget alias; either works, so we do NOT force a specific TLS backend (which
  # would clash with e.g. an openssl-built wget). The script errors clearly if
  # no wget is found. tar/gzip/chroot/mount come from busybox.
  DEPENDS:=+uxcd +jq +ca-bundle
  PKGARCH:=all
endef

define Package/docker2uxcd/description
  The uxcd-aware branch of docker2uxc: it fetches and converts an OCI/registry
  image (or builds one from a Dockerfile, single-stage, host architecture, no
  Docker daemon) into a uxc OCI bundle and registers it with uxcd. This is what
  `uxc pull` and `uxc build` hand off to; install it only if you need them.
endef

define Package/docker2uxcd/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/docker2uxcd/docker2uxc $(1)/usr/bin/docker2uxcd
	$(INSTALL_DIR) $(1)/usr/share/docker2uxc/profiles
	$(CP) $(PKG_BUILD_DIR)/docker2uxcd/profiles/. $(1)/usr/share/docker2uxc/profiles/ 2>/dev/null || true
endef

define Package/uxcd-examples
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=Containers
  TITLE:=Example container + network configs for uxcd
  URL:=https://github.com/oskarirauta/uxcd
  DEPENDS:=+uxcd
  PKGARCH:=all
endef

define Package/uxcd-examples/description
  Sample, ready-to-adapt configurations for uxcd installed under
  /usr/share/uxcd/examples (NOT into /etc, so nothing is auto-registered):
  per-container /etc/uxc/<name>.json examples covering volumes, devices, env,
  resources, depends_on and healthchecks, plus an /etc/config/network snippet
  for a shared "netns" container network namespace. Copy what you need.
endef

define Package/uxcd-examples/install
	$(INSTALL_DIR) $(1)/usr/share/uxcd/examples
	$(CP) $(PKG_BUILD_DIR)/examples/. $(1)/usr/share/uxcd/examples/
endef

$(eval $(call BuildPackage,uxcd))
$(eval $(call BuildPackage,docker2uxcd))
$(eval $(call BuildPackage,uxcd-examples))
$(eval $(call BuildPackage,luci-proto-netns))
$(eval $(call BuildPackage,luci-app-uxcd))
