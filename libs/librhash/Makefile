#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=librhash
PKG_VERSION:=1.4.1
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/rhash/RHash.git
PKG_SOURCE_DATE:=2021-03-14
PKG_SOURCE_VERSION:=821b030830771dcce4b660cd1cfc463d8374f2fa
PKG_MIRROR_HASH:=e5151dba632f92cf8cdd013feb5ae2910510afd9be975ece051e9e8e34b5aba3

include $(INCLUDE_DIR)/package.mk

PKG_INSTALL:=1

define Package/librhash
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=RHash
  URL:=https://github.com/rhash/RHash
endef

define Package/librhash/description
  RHash (Recursive Hasher) is a console utility for calculation and verification of magnet links and various message 
  digests, including CRC32, CRC32C, MD4, MD5, SHA1, SHA256, SHA512, SHA3, AICH, ED2K, DC++ TTH, BitTorrent 
  BTIH, Tiger, GOST R 34.11-94, GOST R 34.11-2012, RIPEMD-160, HAS-160, EDON-R, and Whirlpool.
endef

define Package/librhash/conffiles
/etc/rhashrc
endef

TARGET_CFLAGS += $(FPIC)
CONFIGURE_ARGS += \
	--extra-cflags="$(TARGET_CFLAGS)" \
	--extra-ldflags="$(TARGET_LDFLAGS)"

define Build/InstallDev
	$(INSTALL_DIR) $(1)/etc $(1)/usr/bin $(1)/usr/lib
	$(INSTALL_CONF) $(PKG_INSTALL_DIR)/etc/rhashrc $(1)/etc/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/librhash.so.0 $(1)/usr/lib/
	$(LN) librhash.so $(1)/usr/lib/librhash.so
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin/
endef

define Package/librhash/install
	$(INSTALL_DIR) $(1)/etc $(1)/usr/bin $(1)/usr/lib
	$(INSTALL_CONF) $(PKG_INSTALL_DIR)/etc/rhashrc $(1)/etc/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/librhash.so.0 $(1)/usr/lib/
	$(LN) librhash.so $(1)/usr/lib/librhash.so
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin/
endef

$(eval $(call BuildPackage,librhash))
