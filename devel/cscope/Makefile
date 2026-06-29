include $(TOPDIR)/rules.mk

PKG_NAME:=cscope
PKG_VERSION:=15.9
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://sourceforge.net/projects/cscope/files/cscope/v$(PKG_VERSION)/
PKG_HASH:=c5505ae075a871a9cd8d9801859b0ff1c09782075df281c72c23e72115d9f159

PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>
PKG_LICENSE:=BSD-3-Clause
PKG_LICENSE_FILES:=COPYING

PKG_BUILD_PARALLEL:=1
PKG_FIXUP:=autoreconf
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

CONFIGURE_VARS += \
	hw_cv_func_snprintf_c99=yes \
	hw_cv_func_vsnprintf_c99=yes

define Package/cscope
  SECTION:=devel
  CATEGORY:=Development
  TITLE:=Cscope source code browser
  DEPENDS:=+libncurses +libreadline
  URL:=http://cscope.sourceforge.net/
endef

define Package/cscope/description
  Cscope is a developer tool for browsing source code. It has an
  interactive text screen interface that lets you browse for specified
  elements of code: functions, macros, variables, #include statements,
  and more. Highly useful for navigating large C/C++ codebases.
endef

define Package/cscope/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/cscope $(1)/usr/bin/
endef

$(eval $(call BuildPackage,cscope))
