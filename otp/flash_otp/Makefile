#
# Copyright (C) 2008-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=flash_otp
PKG_RELEASE:=3

include $(INCLUDE_DIR)/package.mk

define KernelPackage/flash_otp
  SUBMENU:=Other modules
  TITLE:=flash_otp
  VERSION:=$(LINUX_VERSION)-$(BOARD)-$(PKG_RELEASE)
  FILES:=$(PKG_BUILD_DIR)/flash_otp.ko
  DEPENDS:=
  AUTOLOAD:= $(call AutoLoad,30,flash_otp)
  KCONFIG:=
endef

define KernelPackage/flash_otp/description
 flash_otp driver
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)
endef

MAKE_OPTS:= \
	ARCH="$(LINUX_KARCH)" \
	CROSS_COMPILE="$(TARGET_CROSS)" \
	SUBDIRS="$(PKG_BUILD_DIR)"

define Build/Compile
	$(MAKE) -C "$(LINUX_DIR)" \
		$(MAKE_OPTS) \
		modules
endef

define KernelPackage/demo/install
    $(INSTALL_DIR) $(1)/
endef

$(eval $(call KernelPackage,flash_otp))
