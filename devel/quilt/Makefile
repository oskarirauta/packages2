include $(TOPDIR)/rules.mk

PKG_NAME:=quilt
PKG_VERSION:=0.67
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@SAVANNAH/quilt
PKG_HASH:=3be3be0987e72a6c364678bb827e3e1fcc10322b56bc5f02b576698f55013cc2

PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>
PKG_LICENSE:=GPL-2.0-only
PKG_LICENSE_FILES:=COPYING

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/quilt
  SECTION:=Devel
  CATEGORY:=Development
  TITLE:=Manage a series of patches by keeping track of the changes each patch makes
  URL:=https://savannah.nongnu.org/projects/quilt/
  DEPENDS:=+diffutils +findutils +gawk +bash +patch +perl
endef

define Package/quilt/description
  Quilt allows you to easily manage large numbers of patches by keeping
  track of the changes each patch makes. Patches can be applied,
  un-applied, refreshed, and more.
endef

CONFIGURE_VARS+= BASH=/bin/bash

CONFIGURE_ARGS+= \
	--disable-nls \
	--with-bash=/bin/bash \
	--with-cp=/bin/cp \
	--with-date=/bin/date \
	--with-perl=/usr/bin/perl \
	--with-grep=/bin/grep \
	--with-tail=/usr/bin/tail \
	--with-tr=/usr/bin/tr \
	--with-stat=/bin/stat \
	--with-sed=/bin/sed \
	--with-awk=/usr/bin/awk \
	--with-diff=/usr/bin/diff \
	--with-patch=/usr/bin/patch \
	--with-find=/usr/bin/find \
	--with-xargs=/usr/bin/xargs \
	--without-diffstat \
	--without-patch-wrapper \
	--without-pod2man \
	--without-column \
	--without-getopt \
	--without-sendmail \
	--without-7z \
	--without-rpmbuild

MAKE_INSTALL_FLAGS+= BUILD_ROOT="$(PKG_INSTALL_DIR)"

define Package/quilt/install
	$(INSTALL_DIR) $(1)/etc $(1)/usr/bin $(1)/usr/share/quilt
	$(INSTALL_CONF) $(PKG_INSTALL_DIR)/etc/quilt.quiltrc $(1)/etc/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/guards $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/quilt $(1)/usr/bin/
	$(CP) -aR $(PKG_INSTALL_DIR)/usr/share/quilt/** $(1)/usr/share/quilt/
endef

$(eval $(call BuildPackage,quilt))
