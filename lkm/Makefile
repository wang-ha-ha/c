PRJ_CONFIG ?= ../../.env

ifneq ($(wildcard $(PRJ_CONFIG)), )
include $(PRJ_CONFIG)
else
$(error "Please run source at top of zeratul project.")
endif

MAKEFLAGS	+= --no-print-directory
KDIR 		:= $(ZRT_ENV_OS_DIR)/kernel

export KDIR

MODULES 	:= hello

.PHONY : all clean clean_build $(MODULES)

all clean: $(MODULES)

$(MODULES):
	@if [ -z "$(SUB)" ] || [ "$(SUB)" = "$@" ]; then \
		make -C $@ $(MAKECMDGOALS); \
	fi
