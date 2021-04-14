include $(TOPDIR)/rules.mk

PKG_NAME:=python3-readline
PKG_VERSION:=8.0.0
PKG_RELEASE:=$(AUTORELEASE)

PYPI_NAME:=gnureadline
PKG_HASH:=61eef72ed02dad415ede49752e972a1d2bd8c35c1e4464565d7effd806c99476

PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>
PKG_LICENSE:=GPL-1.0-only

include ../../../packages/lang/python/pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../../../packages/lang/python/python3-package.mk

define Package/python3-readline
	SECTION:=lang
	CATEGORY:=Languages
	SUBMENU:=Python
	TITLE:=Python3 readline
	DEPENDS:=+python3 +libreadline +libncurses
endef

define Package/python3-readline/description
  Python readline support
endef

$(eval $(call Py3Package,python3-readline))
$(eval $(call BuildPackage,python3-readline))
$(eval $(call BuildPackage,python3-readline-src))
