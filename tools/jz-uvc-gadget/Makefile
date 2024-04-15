CROSS_COMPILE	?= mips-linux-gnu-

SDK_LIB_DIR = ${ZRT_ENV_TOP_DIR}/70mai/app/zeratul-sdk/lib/uclibc
SDK_INC_DIR = ${ZRT_ENV_TOP_DIR}/70mai/app/zeratul-sdk/include
# SRC_INC_DIR = ${ZRT_ENV_TOP_DIR}/samples/libimp-samples

LIBS = $(SDK_LIB_DIR)/libimp.a $(SDK_LIB_DIR)/libalog.a

INCLUDES = -I$(SDK_INC_DIR) -I$(SRC_INC_DIR) 

CC		:= $(CROSS_COMPILE)gcc
CPP		:= $(CROSS_COMPILE)g++
CFLAGS		:= $(INCLUDES) -W -Wall -g -muclibc
LDFLAGS		:= -g -muclibc

all: uvc-gadget

uvc-gadget: $(SDK_LIB_DIR)/libimp.a $(SDK_LIB_DIR)/libalog.a  uvc-gadget.o sample-common.o
	$(CC)  $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS) -lpthread -lm -lrt

clean:
	rm -f *.o
	rm -f uvc-gadget

release:
	cp uvc-gadget ${ZRT_ENV_TOP_DIR}/os/rootfs/7.2.0/camera/system/bin/
	${CROSS_COMPILE}strip ${ZRT_ENV_TOP_DIR}/os/rootfs/7.2.0/camera/system/bin/uvc-gadget
