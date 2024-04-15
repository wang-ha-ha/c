#
# Copyright (C) 2008-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=es8311
PKG_RELEASE:=3
PKG_LICENSE:=GPL-2.0

include $(INCLUDE_DIR)/package.mk

define KernelPackage/es8311
  SUBMENU:=Sound Support
  TITLE:=es8311
  VERSION:=$(LINUX_VERSION)-$(BOARD)-$(PKG_RELEASE)
  FILES:=$(PKG_BUILD_DIR)/snd-soc-es8311.ko
  DEPENDS:= +kmod-i2c-core +kmod-sound-soc-core +kmod-regmap-i2c +kmod-sound-mt7620
  AUTOLOAD:= $(call AutoLoad,90,snd-soc-es8311)
  KCONFIG:=
endef

define KernelPackage/es8311/description
 es8311 driver
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)
endef

MAKE_OPTS:= \
	$(KERNEL_MAKE_FLAGS) \
	M="$(PKG_BUILD_DIR)"

define Build/Compile
	$(MAKE) -C "$(LINUX_DIR)" \
		$(MAKE_OPTS) \
		modules
endef

define KernelPackage/demo/install
    $(INSTALL_DIR) $(1)/
endef

$(eval $(call KernelPackage,es8311))
