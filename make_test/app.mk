ifneq (1, $(words $(APP_NAME) $(LIB_NAME)))
$(error You must specify exactly one of APP_NAME or LIB_NAME)
endif

ifeq ($(OBJS),)
$(error You must specify OBJS)
endif

CC			:= $(CROSS_COMPILE)gcc
CPP			:= $(CROSS_COMPILE)g++
ifeq ($(CPP_ENABLE),y)
LD			:= $(CROSS_COMPILE)g++
else
LD			:= $(CROSS_COMPILE)gcc
endif
STRIP		:= $(CROSS_COMPILE)strip

_empty:=
_space:= $(_empty) $(_empty)
INGNORE_DIR_GREP := $(foreach dir,$(INGNORE_DIR), | grep -v $(dir)) 
SRC_DIR := $(shell find . -type d $(INGNORE_DIR_GREP))

vpath %.c $(subst $(_space),:,$(SRC_DIR))
ifeq ($(CPP_ENABLE),y)
vpath %.cpp $(subst $(_space),:,$(SRC_DIR))
endif

ifneq ($(APP_NAME),)
TARGET := $(APP_NAME)
BIN_DIR := $(TOPDIR)/out/bin
else
TARGET := $(LIB_NAME)
CFLAGS += -fPIC
CPPFLAGS += -fPIC
LDFLAGS += -shared
BIN_DIR := $(TOPDIR)/out/lib
endif

LDFLAGS += -L$(TOPDIR)/out/lib

OBJ_DIR := $(TOPDIR)/out/obj/$(subst .so,,$(TARGET))
OBJ_WITHOUT_DIR = $(notdir $(OBJS))
OBJ_WITH_DIR = $(addprefix $(OBJ_DIR)/,$(OBJ_WITHOUT_DIR))

.PHONY: all clean

all:DIR_CREATE $(TARGET)

$(TARGET):$(OBJ_WITH_DIR)
	$(LD) $^ -o $(BIN_DIR)/$@ $(CFLAGS) $(DEFS) $(INCLUDE) $(LDFLAGS) $(LIB)

define CRT_DIR
	if [ ! -d $(1) ];\
		then\
		mkdir -p $(1);\
	fi  
endef
 
DIR_CREATE:  
	@$(call CRT_DIR,$(OBJ_DIR))
	@$(call CRT_DIR,$(BIN_DIR))

$(OBJ_DIR)/%.o:%.c
	$(CC) -c $< -o $@ $(CFLAGS) $(DEFS) $(INCLUDE)

$(OBJ_DIR)/%.o:%.cpp
	$(CPP) -c $< -o $@ $(CPPFLAGS) $(DEFS) $(INCLUDE)

clean:
	-rm $(OBJ_DIR)/*.o

