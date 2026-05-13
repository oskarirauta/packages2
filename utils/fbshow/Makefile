include $(TOPDIR)/rules.mk

PKG_NAME:=fbshow
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/hansliss/fbshow.git
PKG_MIRROR_HASH:=3804774468a95459fb4f5f41e63325d4caa2a39d6a1fd3302f59608403f9dfde
PKG_SOURCE_DATE:=2024-02-03
PKG_SOURCE_VERSION:=c573dff3f1eb9f62705d2dd8f308d42f90e7524e

PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>
PKG_FORTIFY_SOURCE:=0

include $(INCLUDE_DIR)/package.mk

define Package/fbshow
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Load an image and text to a framebuffer device
  DEPENDS:=+imagemagick +libfreetype
endef

define Package/fbshow/description
  Open a framebuffer for a display device (e.g. small OLED or LCD screen on a Raspberry Pi) and write an image or some text to it.
endef

TARGET_CFLAGS += -I$(STAGING_DIR)/usr/include -I$(STAGING_DIR)/usr/include/ImageMagick-7

define Package/fbshow/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/fbshow $(1)/usr/bin/
# $(1)/usr/share/splash/fonts $(1)/usr/share/splash/images
#	$(INSTALL_BIN) $(PKG_BUILD_DIR)/splash $(1)/usr/bin/
#	$(INSTALL_DATA) $(PKG_BUILD_DIR)/res/fonts/** $(1)/usr/share/splash/fonts/
#	$(INSTALL_DATA) ./files/openwrt.png $(1)/usr/share/splash/images/splash.png
endef

$(eval $(call BuildPackage,fbshow))
