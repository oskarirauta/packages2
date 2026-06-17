include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=kernel-headers
PKG_VERSION:=$(LINUX_VERSION)
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(LINUX_VERSION)

PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>
PKG_LICENSE:=GPL-2.0-only

include $(INCLUDE_DIR)/package.mk

KERNEL_HEADERS_DIR:=/usr/src/linux-$(LINUX_VERSION)

define Package/kernel-headers
  SECTION:=devel
  CATEGORY:=Development
  TITLE:=Linux kernel headers for external module compilation
  URL:=https://www.kernel.org/
  DEPENDS:=+kmod
endef

define Package/kernel-headers/description
  Kernel headers, configuration and build scripts required to compile
  external kernel modules against the running kernel. Installs to
  /usr/src/linux-$(LINUX_VERSION) and creates the
  /lib/modules/$(LINUX_VERSION)/build symlink.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
endef

define Build/Configure
endef

# Compile fixdep and modpost for the target (musl) using the cross-compiler.
# The kernel build tree has glibc-linked host binaries that will not run on
# the musl-based target; these replacements are identical in function.
define Build/Compile
	$(TARGET_CC) $(TARGET_CFLAGS) \
		-I$(LINUX_DIR)/scripts/include \
		-I$(LINUX_DIR)/scripts/mod \
		-I$(LINUX_DIR)/include \
		-o $(PKG_BUILD_DIR)/fixdep \
		$(LINUX_DIR)/scripts/basic/fixdep.c

	$(TARGET_CC) $(TARGET_CFLAGS) \
		-include $(CURDIR)/files/musl-arm-elf-compat.h \
		-I$(LINUX_DIR)/scripts/include \
		-I$(LINUX_DIR)/scripts/mod \
		-I$(LINUX_DIR)/include \
		-o $(PKG_BUILD_DIR)/modpost \
		$(LINUX_DIR)/scripts/mod/modpost.c \
		$(LINUX_DIR)/scripts/mod/file2alias.c \
		$(LINUX_DIR)/scripts/mod/sumversion.c \
		$(LINUX_DIR)/scripts/mod/symsearch.c
endef

define Package/kernel-headers/install
	# Top-level build files
	$(INSTALL_DIR) $(1)$(KERNEL_HEADERS_DIR)
	$(INSTALL_DATA) $(LINUX_DIR)/Makefile        $(1)$(KERNEL_HEADERS_DIR)/
	$(INSTALL_DATA) $(LINUX_DIR)/Kbuild          $(1)$(KERNEL_HEADERS_DIR)/
	$(INSTALL_DATA) $(LINUX_DIR)/.config         $(1)$(KERNEL_HEADERS_DIR)/
	$(INSTALL_DATA) $(LINUX_DIR)/Module.symvers  $(1)$(KERNEL_HEADERS_DIR)/
	$(INSTALL_DATA) $(LINUX_DIR)/modules.builtin $(1)$(KERNEL_HEADERS_DIR)/
	$(INSTALL_DATA) $(LINUX_DIR)/modules.order   $(1)$(KERNEL_HEADERS_DIR)/

	# All kernel headers
	$(CP) $(LINUX_DIR)/include $(1)$(KERNEL_HEADERS_DIR)/

	# Architecture-specific headers and Makefile
	$(INSTALL_DIR) $(1)$(KERNEL_HEADERS_DIR)/arch/$(LINUX_KARCH)
	$(CP) $(LINUX_DIR)/arch/$(LINUX_KARCH)/include \
		$(1)$(KERNEL_HEADERS_DIR)/arch/$(LINUX_KARCH)/
	$(INSTALL_DATA) $(LINUX_DIR)/arch/$(LINUX_KARCH)/Makefile \
		$(1)$(KERNEL_HEADERS_DIR)/arch/$(LINUX_KARCH)/

	# Build system scripts (Makefiles and support files; no host-compiled binaries)
	$(INSTALL_DIR) $(1)$(KERNEL_HEADERS_DIR)/scripts
	$(INSTALL_DATA) $(LINUX_DIR)/scripts/Kbuild.include       $(1)$(KERNEL_HEADERS_DIR)/scripts/
	$(INSTALL_DATA) $(LINUX_DIR)/scripts/subarch.include      $(1)$(KERNEL_HEADERS_DIR)/scripts/
	$(INSTALL_DATA) $(LINUX_DIR)/scripts/module.lds           $(1)$(KERNEL_HEADERS_DIR)/scripts/
	$(INSTALL_DATA) $(LINUX_DIR)/scripts/Makefile.build       $(1)$(KERNEL_HEADERS_DIR)/scripts/
	$(INSTALL_DATA) $(LINUX_DIR)/scripts/Makefile.clean       $(1)$(KERNEL_HEADERS_DIR)/scripts/
	$(INSTALL_DATA) $(LINUX_DIR)/scripts/Makefile.compiler    $(1)$(KERNEL_HEADERS_DIR)/scripts/
	$(INSTALL_DATA) $(LINUX_DIR)/scripts/Makefile.debug       $(1)$(KERNEL_HEADERS_DIR)/scripts/
	$(INSTALL_DATA) $(LINUX_DIR)/scripts/Makefile.extrawarn   $(1)$(KERNEL_HEADERS_DIR)/scripts/
	$(INSTALL_DATA) $(LINUX_DIR)/scripts/Makefile.host        $(1)$(KERNEL_HEADERS_DIR)/scripts/
	$(INSTALL_DATA) $(LINUX_DIR)/scripts/Makefile.lib         $(1)$(KERNEL_HEADERS_DIR)/scripts/
	$(INSTALL_DATA) $(LINUX_DIR)/scripts/Makefile.modfinal    $(1)$(KERNEL_HEADERS_DIR)/scripts/
	$(INSTALL_DATA) $(LINUX_DIR)/scripts/Makefile.modpost     $(1)$(KERNEL_HEADERS_DIR)/scripts/
	$(INSTALL_DATA) $(LINUX_DIR)/scripts/Makefile.userprogs   $(1)$(KERNEL_HEADERS_DIR)/scripts/
	$(INSTALL_DATA) $(LINUX_DIR)/scripts/Makefile.dtbs        $(1)$(KERNEL_HEADERS_DIR)/scripts/

	# scripts/basic: only fixdep (target-compiled) + its Makefile
	$(INSTALL_DIR) $(1)$(KERNEL_HEADERS_DIR)/scripts/basic
	$(INSTALL_DATA) $(LINUX_DIR)/scripts/basic/Makefile $(1)$(KERNEL_HEADERS_DIR)/scripts/basic/
	$(INSTALL_BIN)  $(PKG_BUILD_DIR)/fixdep             $(1)$(KERNEL_HEADERS_DIR)/scripts/basic/

	# scripts/mod: modpost (target-compiled) + generated and required headers
	$(INSTALL_DIR) $(1)$(KERNEL_HEADERS_DIR)/scripts/mod
	$(INSTALL_DATA) $(LINUX_DIR)/scripts/mod/Makefile             $(1)$(KERNEL_HEADERS_DIR)/scripts/mod/
	$(INSTALL_DATA) $(LINUX_DIR)/scripts/mod/elfconfig.h          $(1)$(KERNEL_HEADERS_DIR)/scripts/mod/
	$(INSTALL_DATA) $(LINUX_DIR)/scripts/mod/devicetable-offsets.h $(1)$(KERNEL_HEADERS_DIR)/scripts/mod/
	$(INSTALL_DATA) $(LINUX_DIR)/scripts/mod/elf.h                $(1)$(KERNEL_HEADERS_DIR)/scripts/mod/
	$(INSTALL_DATA) $(LINUX_DIR)/scripts/mod/modpost.h            $(1)$(KERNEL_HEADERS_DIR)/scripts/mod/
	$(INSTALL_BIN)  $(PKG_BUILD_DIR)/modpost                      $(1)$(KERNEL_HEADERS_DIR)/scripts/mod/

	# scripts/include: helper headers (hash, list etc. — needed to rebuild modpost from source)
	$(CP) $(LINUX_DIR)/scripts/include $(1)$(KERNEL_HEADERS_DIR)/scripts/

	# /lib/modules/<version>/build symlink
	$(INSTALL_DIR) $(1)/lib/modules/$(LINUX_VERSION)
	ln -sf $(KERNEL_HEADERS_DIR) $(1)/lib/modules/$(LINUX_VERSION)/build
endef

$(eval $(call BuildPackage,kernel-headers))
