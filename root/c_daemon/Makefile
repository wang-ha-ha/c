
PROG = root_deamon

SRC  = $(wildcard *.c)

OBJ  = $(patsubst %.c,%.o,$(SRC))

CC	:= gcc 

CCFLAG = -g  

all:$(PROG)

$(PROG):$(OBJ)
	$(CC) $(CCFLAG) $^ -o $@ 

.PHONY:
clean:
	rm -rf *.o $(PROG)
