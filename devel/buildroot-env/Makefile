include $(TOPDIR)/rules.mk

PKG_NAME:=buildroot-env
PKG_VERSION:=20230519
PKG_RELEASE:=1

PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>

include $(INCLUDE_DIR)/package.mk

define Package/buildroot-env
  SECTION:=devel
  CATEGORY:=Development
  TITLE:=Openwrt buildroot environment
  DEPENDS:= \
		+libuargp +musl-fts +musl-obstack +libzstd +libc-stubs +libstdcpp \
		+pkg-config +make +gcc +diffutils +autoconf +automake +check +git +git-http +patch +libtool-bin \
		+grep +rsync +tar +python3 +getopt +procps-ng-ps +gawk +sed +xz +unzip +gzip +bzip2 +flock +wget-ssl \
		+perl +perlbase-findbin +perlbase-pod +perlbase-storable +perlbase-feature +perlbase-b +python3 \
		+coreutils-nohup +coreutils-install +coreutils-sort +coreutils-ls +coreutils-realpath +coreutils-stat \
		+coreutils-nproc +coreutils-od +coreutils-mkdir +coreutils-date +coreutils-comm +coreutils-printf \
		+coreutils-ln +coreutils-cp +coreutils-split +coreutils-csplit +coreutils-cksum +coreutils-expr \
		+coreutils-tr +coreutils-test +coreutils-uniq +coreutils-fmt +rev +curl \
		+libncurses-dev +zlib-dev +xz-utils +tar +ldconfig-script +pax-utils \
		+bash +quilt +htop +whereis +less +file +findutils +findutils-locate +chattr +lsattr +xxd \
		+perlbase-ipc +perlbase-module +perlbase-extutils \
		+perlbase-version +perlbase-dirhandle +perlbase-autosplit +perlbase-ostype \
		+perlbase-open +perlbase-if +perlbase-digest \
		+perlbase-autouse +perlbase-sys +perlbase-sort \
		+perlbase-diagnostics +python3-lib2to3 +coreutils-join +re2
endef

define Package/buildroot-env/description
  Provides environment suitable to build openwrt on openwrt with
  depencies and necessary missing header files.

  Requires unstripped build. Further documentation is available at
  https://openwrt.org/docs/guide-developer/toolchain/building_openwrt_on_openwrt
endef

define Build/Configure
	# Nothing to configure
endef

define Build/Compile
	# Nothing to build
endef

define Build/Install
	# Nothing to install
endef

define Package/buildroot-env/install
	$(INSTALL_DIR) $(1)/usr/include $(1)/usr/lib
	$(INSTALL_DATA) $(STAGING_DIR)/usr/include/argp.h $(1)/usr/include/
	$(INSTALL_DATA) $(STAGING_DIR)/usr/include/fts.h $(1)/usr/include/
	$(INSTALL_DATA) $(STAGING_DIR)/usr/include/obstack.h $(1)/usr/include/
	$(INSTALL_DATA) $(STAGING_DIR)/usr/include/{zdict.h,zstd.h,zstd_errors.h} $(1)/usr/include/
	ln -s libncursesw.so.6.4 $(1)/usr/lib/libncurses.so
	ln -s libncursesw.a $(1)/usr/lib/libncurses.a
endef

$(eval $(call BuildPackage,buildroot-env))
