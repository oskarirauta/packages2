include $(TOPDIR)/rules.mk

LUCI_TITLE:=LuCI Support for podman
LUCI_DEPENDS:=@(aarch64||arm||x86_64) \
	+luci-compat \
	+luci-lib-podman \
	+podman \
	+ttyd
LUCI_PKGARCH:=all

PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>

PKG_VERSION:=v0.41.0

include ../../../luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature
