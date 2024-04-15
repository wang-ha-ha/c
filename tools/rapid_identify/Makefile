CPP = mips-linux-gnu-g++

INCLUDES := -I../../../alg/libimgalgo/include 

INCLUDES += -I../../platform-osp/libosp/include 
INCLUDES += -I../../platform-osp 
INCLUDES += -I../../libcore/include 
INCLUDES += -I../../libcrypt/include 

CXXFLAGS = $(INCLUDES) -O2 -Wall -march=mips32r2 -z,relro -z,now
CXXFLAGS += -flto -ffunction-sections -finline-functions -fmerge-all-constants -DNDEBUG -std=c++11

LDFLAGS := -L../../lib/uclibc -losp -lunwind -lcore -lcrypt -lpthread -lm -lrt -ldl -lcserialport

rapid_identify:rapid_identify.o
	$(CPP)  -o $@ $^  $(CXXFLAGS) $(LDFLAGS) -muclibc
%.o:%.cpp
	$(CPP)  -c -o $@ $< $(CXXFLAGS) $(LDFLAGS) -muclibc

.PHONY: clean
clean:
	rm *.o rapid_identify
clear:
	rm *.o 
