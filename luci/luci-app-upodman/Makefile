include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-upodman
PKG_VERSION:=0.9
PKG_RELEASE:=2
PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>

LUCI_TITLE:=upodman luci interface
LUCI_DEPENDS:=+luci-mod-status +upodman
LUCI_PKGARCH:=all

include ../../../luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature
