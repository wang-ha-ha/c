TARGET  := libcJSON.so

CC	:= mips-linux-gnu-g++
LIBS    :=
LDFLAGS :=
DEFINES :=
INCLUDE := -I./include
CFLAGS  :=  -Wall -O3 -fPIC $(DEFINES) $(INCLUDE)
CXXFLAGS:= $(CFLAGS)
SHARE   := -shared -o
  
SOURCE  :=  $(wildcard ./src/*.c)
OBJS    :=  $(patsubst %.c,%.o,$(SOURCE))
  
.PHONY : all clean install
  
all : $(TARGET)

$(TARGET) : $(OBJS)
	$(CC) $(CXXFLAGS) $(SHARE) $@ $^ $(LDFLAGS) $(LIBS)

install-strip : $(TARGET)
	 install -d $(DESTDIR)/lib
	 install -p -D -m 0755 $(TARGET) $(DESTDIR)/lib
	 install -d $(DESTDIR)/include
	 install -p -D -m 0755 ./include/* $(DESTDIR)/include

clean :
	rm $(TARGET) $(OBJS)
	    

