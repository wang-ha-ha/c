# all:
# 	mips-linux-gnu-g++ main.cpp CrashDumper.cpp -I${ZRT_ENV_TOP_DIR}/70mai/app/include -Wl,--start-group ${ZRT_ENV_TOP_DIR}/70mai/app/lib/uclibc/libunwind-ptrace.a ${ZRT_ENV_TOP_DIR}/70mai/app/lib/uclibc/libunwind-generic.a -Wl,--end-group -o crash_catcher -std=c++11 -muclibc
# 	mips-linux-gnu-g++ crash_catcher_signal_handler.c -shared -o libcrash_catcher_signal_handler.so -fPIC -muclibc
# 	mips-linux-gnu-g++ test_main.cpp -Lcrash_catcher_signal_handler -o test_main -g -muclibc

CC = mips-linux-gnu-gcc
CXX = mips-linux-gnu-g++

CFLAGS	:= -I. -I${ZRT_ENV_TOP_DIR}/70mai/app/include -L${ZRT_ENV_TOP_DIR}/70mai/app/lib/uclibc

all:libcrash_catcher_signal_handler.so crash_catcher  test_main

crash_catcher: main.o CrashDumper.o
	$(CXX) -muclibc -lunwind -lunwind-ptrace -lunwind-generic -ldl $(CFLAGS) -o $@ $^

libcrash_catcher_signal_handler.so:crash_catcher_signal_handler.o crash_catcher_collect_info.o
	$(CXX) -muclibc -lunwind -lunwind-ptrace -lunwind-generic -shared -fPIC $(CFLAGS) -o $@ $^ 

test_main: test_main.o
	$(CXX) -muclibc -lcrash_catcher_signal_handler -o $@ $^

%.o:%.c
	$(CC) $(CFLAGS) -muclibc -c -o $@ $<

%.o:%.cpp
	$(CXX) $(CFLAGS) -muclibc -c -o $@ $<

clear:
	rm *.o 