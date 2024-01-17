CROSS_COMPILE = mips-linux-gnu
GCC = $(CROSS_COMPILE)-gcc

INCLUDES = -I.
CFLAGS = $(INCLUDES) -g -O2 -Wall

CFLAGS += -muclibc
LDFLAG += -muclibc

LIBS = -lpthread

PROGRAMS = efuse_tool test

All: $(PROGRAMS)

test: efuse.o test.o
	$(GCC) $(LDFLAG) ${LIBS} -o $@ $^

efuse_tool: efuse_tool.o efuse.o
	$(GCC) $(LDFLAG) ${LIBS} -o $@ $^

%.o:%.c
	$(GCC) -c $(CFLAGS) $< -o $@

.PHONY: clean

clean:
	rm -f *.o *~

distclean: clean
	rm -f $(PROGRAMS)

