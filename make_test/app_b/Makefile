include ../.config

CPP_ENABLE := y
APP_NAME := app_b
INGNORE_DIR := include
OBJS := $(patsubst %.c,%.o,$(shell find . -type f -name '*.c'))	
OBJS += $(patsubst %.cpp,%.o,$(shell find . -type f -name '*.cpp'))	
INCLUDE := -I./include -I../lib_a_so/include -I../lib_b_so/include
LIB := -l _a -l _b

include $(TOPDIR)/app.mk