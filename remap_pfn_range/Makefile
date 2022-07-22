obj-m:=remap_test.o  

ifneq ($(CPU),OA8000)
KERNELBUILD:=/lib/modules/$(shell uname -r)/build
MYARCH:=x86_64
MYCROSS_COMPILE:=
else #make CPU=OA8000
KERNELBUILD:=/home/wangchuanqi/workspace/doorbell/work-out/OV8000/out/oa8000_doorbell/linux
MYARCH:=arm 
MYCROSS_COMPILE:=/opt/arm-ov-linux-20210322/bin/arm-linux-
endif

default:
	make -C $(KERNELBUILD) ARCH=$(MYARCH) CROSS_COMPILE=$(MYCROSS_COMPILE) M=$(shell pwd) modules
clean:
	rm -rf *.o *.ko *.mod.c .*.cmd *.markers *.order *.symvers .tmp_versions