include ../.config

LIB_NAME := lib_a.so
INGNORE_DIR := include
OBJS := $(patsubst %.c,%.o,$(shell find -L src -type f -name '*.c'))	
INCLUDE := -I./include

include $(TOPDIR)/app.mk