include $(TOPDIR)/rules.mk

PKG_NAME:=sftp-server
PKG_VERSION:=3
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/ewxrjk/sftpserver.git
PKG_SOURCE_DATE:=2024-04-17
PKG_SOURCE_VERSION:=454309adcd99e3253f106f1f634e800e4d098b18
PKG_MIRROR_HASH:=cea002e2822e6423e5eb05dc4bc2462ee42bfd553f23ea3952622bbc9456f931

PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>
PKG_LICENSE:=Apache-2.0
PKG_LICENSE_FILES:=LICENSE
PKG_FIXUP:=autoreconf
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/sftp-server
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=SSH
  TITLE:=Green End SFTP Server
  DEPENDS:=
  CONFLICTS:=openssh-sftp-server
  URL:=http://www.greenend.org.uk/rjk/sftpserver/
endef

define Package/sftp-server/description
 This is an SFTP server supporting up to protocol version 6. It is possible to use it as a drop-in replacement for the OpenSSH server.
endef

CONFIGURE_ARGS += --libexecdir=/usr/libexec

define Package/sftp-server/install
	$(INSTALL_DIR) $(1)/usr/libexec
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/libexec/gesftpserver $(1)/usr/libexec/sftp-server
endef

$(eval $(call BuildPackage,sftp-server))
