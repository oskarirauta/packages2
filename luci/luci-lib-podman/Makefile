include $(TOPDIR)/rules.mk

PKG_NAME:=luci-lib-podman
PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>

LUCI_TYPE:=col

LUCI_TITLE:=LuCI library for podman
LUCI_DESCRIPTION:=Podman API for LuCI

LUCI_DEPENDS:=@(aarch64||arm||x86_64) +luci-lib-jsonc
LUCI_PKGARCH:=all

include ../../../luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature
