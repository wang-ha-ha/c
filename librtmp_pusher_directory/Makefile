PROG := rtmp-test

SRC-C   := $(wildcard *.c)
SRC-CPP  := $(wildcard *.cpp)
$(info SRC-C:$(SRC-C))
$(info SRC-CPP :$(SRC-CPP))

OBJ := $(patsubst %.c,%.o,$(SRC-C))  $(patsubst %.cpp,%.o,$(SRC-CPP))
$(info OBJ:$(OBJ))

CC	= gcc
CPP = g++
LD  = g++

CCFLAG =
CPPFLAG = -std=c++11 
INCLUDES = -I. -I/home/yzy/CODE/librtmp/include
LIBS =  -L/home/yzy/CODE/librtmp -lrtmp

all:$(PROG)

$(PROG):$(OBJ)
	$(LD) $^ -o $@  $(LIBS) 

%.o:%.c
	$(CC) $< -c -o $@  $(LIBS) $(INCLUDES) $(CCFLAG) 

%.o:%.cpp
	$(CPP) $< -c -o $@ $(LIBS) $(INCLUDES) $(CPPFLAG) 
.PHONY:
clean:
	rm -rf *.o $(PROG)