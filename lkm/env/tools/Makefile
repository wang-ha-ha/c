HOST_CC = gcc
HOST_CFLAGS = -O2 -Wall

CROSS_CC = mips-linux-gnu-gcc
CROSS_STRIP = mips-linux-gnu-strip
CROSS_CFLAGS = -O2 -Wall -muclibc
CROSS_LDFLAG = -muclibc -lpthread

PROGS = fw_genenv fw_setenv 

all: $(PROGS)

fw_genenv: fw_genenv.host.o crc16.host.o
	$(HOST_CC) $(HOST_LDFLAG) -o $@ $^

fw_setenv: fw_setenv.cross.o crc16.cross.o fw_env.cross.o
	$(CROSS_CC) $(CROSS_LDFLAG) -o $@ $^
	$(CROSS_STRIP) $@

%.host.o:%.c
	$(HOST_CC) -c $(HOST_CFLAGS) $< -o $@

%.cross.o:%.c
	$(CROSS_CC) -c $(CROSS_CFLAGS) $< -o $@

clean:
	rm -f *.o *~

distclean: clean
	rm -f $(PROGS)
