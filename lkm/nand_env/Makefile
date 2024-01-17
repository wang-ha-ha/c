ARCH ?= mips
CROSS_COMPILE ?= mips-linux-gnu-

KDIR ?= $(ZRT_ENV_KERNEL_DIR)
MODULE_NAME := env-flash

all: modules

.PHONY: modules clean

$(MODULE_NAME)-objs := env.o crc32.o
obj-m := $(MODULE_NAME).o

modules:
	@$(MAKE) -C $(KDIR) M=$(shell pwd) $@

clean:
	@rm -rf *.o *~ .depend .*.cmd  *.mod.c .tmp_versions *.ko *.symvers modules.order

release:
	mkdir -p ../modules/${ZRT_ENV_TOOL_CHAIN}/$(subst _,.,${ZRT_ENV_LINUX_VERSION})
	cp ${MODULE_NAME}.ko ../modules/${ZRT_ENV_TOOL_CHAIN}/$(subst _,.,${ZRT_ENV_LINUX_VERSION})