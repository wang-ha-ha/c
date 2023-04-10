HOST_CC = gcc
HOST_CFLAGS = -O2 -Wall

CROSS_CC = mips-linux-gnu-gcc
CROSS_STRIP = mips-linux-gnu-strip
CROSS_CFLAGS = -O2 -Wall -muclibc
CROSS_LDFLAG = -muclibc -lpthread

PROGS = fw_genenv fw_setenv 

all: $(PROGS)

fw_genenv: fw_genenv.host.o crc32.host.o
	$(HOST_CC) $(HOST_LDFLAG) -o $@ $^

fw_setenv: fw_setenv.cross.o crc32.cross.o fw_env.cross.o
	$(CROSS_CC) $(CROSS_LDFLAG) -o $@ $^
	$(CROSS_STRIP) $@

%.host.o:%.c
	$(HOST_CC) -c $(HOST_CFLAGS) $< -o $@

%.cross.o:%.c
	$(CROSS_CC) -c $(CROSS_CFLAGS) $< -o $@

install:
	cp fw_genenv ../../../env_config
	#ln -s  fw_setenv fw_printenv
	cp fw_setenv ../../../../os/rootfs/7.2.0/camera/system/bin

clean:
	-rm -f *.o *~ 

distclean: clean
	-rm -f $(PROGS) fw_printenv
