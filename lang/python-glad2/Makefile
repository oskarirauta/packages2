# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python-glad2
PKG_VERSION:=2.0.8
PKG_RELEASE:=1

PYPI_NAME:=glad2
PKG_HASH:=b84079b9fa404f37171b961bdd1d8da21370e6c818defb8481c5b3fe3d6436da

PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

PKG_BUILD_DEPENDS:=python-setuptools-scm/host
HOST_BUILD_DEPENDS:= \
	python3/host \
	python-build/host \
	python-installer/host \
	python-wheel/host \
	python-setuptools-scm/host

include ../../../packages/lang/python/pypi.mk
include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/host-build.mk
include ../../../packages/lang/python/python3-package.mk
include ../../../packages/lang/python/python3-host-build.mk

define Package/python3-glad2
  SECTION:=lang
  CATEGORY:=Languages
  SUBMENU:=Python
  TITLE:=Multi-Language GL/GLES/EGL/GLX/VK/WGL Loader-Generator
  URL:=https://pypi.org/project/glad2
  DEPENDS:=+python3-light
endef

define Package/python3-glad2/description
Multi-Language GL/GLES/EGL/GLX/VK/WGL Loader-Generator based on the official specifications.
Glad uses the official Khronos-XML specs to generate a GL/GLES/EGL/GLX/VK/WGL Loader made for your needs.
endef

$(eval $(call Py3Package,python3-glad2))
$(eval $(call BuildPackage,python3-glad2))
$(eval $(call BuildPackage,python3-glad2-src))
$(eval $(call HostBuild))
