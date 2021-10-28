SRC = $(wildcard *.c)
OBJ = $(patsubst %.c,%.o,$(SRC))
target = netlink_udev

DEFS ?= 
CFLAGS ?= 
CC ?= gcc
INCLUDE ?= 
LIB ?= 

$(target):$(OBJ)
	$(CC) $^ -o $@ $(CFLAGS) $(DEFS) $(INCLUDE) $(LIB)
.PHONY:
clean:
	rm *.o $(target)
