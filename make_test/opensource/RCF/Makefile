TARGET  := libRCF.so
CC      := mips-linux-gnu-g++
LIBS    := -luuid -L../target/libuuid/usr/local/lib
LDFLAGS :=
DEFINES :=
INCLUDE := -I./include -I../target/libuuid/usr/local/include
CFLAGS  :=  -Wall -O3 -fPIC $(DEFINES) $(INCLUDE)
CXXFLAGS:= $(CFLAGS) -std=c++1y
SHARE   :=  -shared -o
  
SOURCE  := src/RCF/RCF.cpp
OBJS    := src/RCF/RCF.o
  
.PHONY : all clean install-strip
  
all : $(TARGET)

$(TARGET) : $(OBJS)
	$(CC) $(CXXFLAGS) $(SHARE) $@ $^ $(LDFLAGS) $(LIBS)

$(OBJS) : $(SOURCE)
	$(CC) $(CXXFLAGS) -c -o $@  $< $(LDFLAGS) $(LIBS)

install-strip : $(TARGET)
	 install -d $(DESTDIR)/lib
	 install -p -D -m  0755 $(TARGET) $(DESTDIR)/lib
	 install -d $(DESTDIR)/include
	 cp ./include/* $(DESTDIR)/include/ -rf

clean :
	-rm $(TARGET) $(OBJS)
