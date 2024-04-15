#
# Copyright (C) 2008-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=wtn6f
PKG_RELEASE:=3

include $(INCLUDE_DIR)/package.mk

define KernelPackage/wtn6f
  SUBMENU:=Other modules
  TITLE:=wtn6f
  VERSION:=$(LINUX_VERSION)-$(BOARD)-$(PKG_RELEASE)
  FILES:=$(PKG_BUILD_DIR)/wtn6f.ko
  DEPENDS:=
  AUTOLOAD:= $(call AutoLoad,30,wtn6f)
  KCONFIG:=
endef

define KernelPackage/wtn6f/description
 wtn6f driver
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

$(eval $(call KernelPackage,wtn6f))
