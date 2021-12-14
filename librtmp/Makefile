LIBRARY_DIR = .

#--------- ENVIRONMENT SETTING --------------------
INCLUDES        = -I$(LIBRARY_DIR)/include -I$(LIBRARY_DIR)/include/librtmp/ 
WARNING         = -Wall -Wundef -Wsign-compare -Wno-missing-braces -Wstrict-prototypes
COMPILE_OPTS    = $(INCLUDES) -I. -O2 -fPIC -ffunction-sections -fdata-sections -DGLIB
CPPFLAGS        =
CFLAGS          = $(PLATFORM_CFLAGS) $(PRJCFG_CFLAGS)
C_FLAGS         = $(COMPILE_OPTS) $(CPPFLAGS) $(CFLAGS) $(WARNING) -fcommon
LD_FLAGS        = -fPIC -shared -lpthread -lssl -L$(LIBRARY_DIR)/output
#--------- END OF ENVIRONMENT SETTING -------------
DEP_LIBRARIES :=

#--------- Compiling --------------------
LIB_NAME=librtmp
INC = 
INC_PROTECT =
SRC = 	amf.c \
	hashswf.c \
	log.c \
	parseurl.c \
	rtmp.c 

HEADER = $(shell find . -name "*.h")

copy = if [ ! -z "$(1)" -a "$(1)" != " " ]; then cp -avf $(1) $(2); fi

OBJ = $(SRC:.c=.o)

SHELL = /bin/bash
MULTI_CORES = $(shell grep -c ^processor /proc/cpuinfo)

ifeq ("$(wildcard *.c */*.c)","")
all:
	@echo ">>> Skip"
clean:
	@echo ">>> Skip"
install_header:
	@echo ">>> Skip"
else

.PHONY: librtmp
all: librtmp

librtmp: $(OBJ)
	@$(AR) rcs $(LIB_NAME).a $(OBJ)
	@$(CC) -o $(LIB_NAME).so $(OBJ) $(LD_FLAGS)


%.o:%.c $(HEADER)
	$(CC) $(C_FLAGS) $< -c

clean:
	rm -vf $(OBJ) *.o *.a *.so*

install_header:
	@$(call copy, $(INC), $(LIBRARY_DIR)/include)
	@$(call copy, $(INC_PROTECT), $(LIBRARY_DIR)/include/protected)
endif

install: install_header
	@mkdir -p $(LIBRARY_DIR)/output
	@cp -avf *.so* *.a $(LIBRARY_DIR)/output
