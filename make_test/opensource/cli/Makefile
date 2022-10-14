TARGET  := libCLILib.so
CC      := arm-linux-g++
LIBS    := 
LDFLAGS :=
DEFINES :=
INCLUDE := -I./include  -I../../toolchains/include 
CFLAGS  :=  -Wall -O3 -fPIC $(DEFINES) $(INCLUDE)
CXXFLAGS:= $(CFLAGS) -std=c++1y
SHARE   :=  -shared -o
  
SOURCE  :=  $(wildcard ./src/*.cpp)
OBJS    :=  $(patsubst %.cpp,%.o,$(SOURCE))
  
.PHONY : all clean install-strip
  
all : $(TARGET)

$(TARGET) : $(OBJS)
	$(CC) $(CXXFLAGS) $(SHARE) $@ $^ $(LDFLAGS) $(LIBS)

%.o : %.cpp
	$(CC) $(CXXFLAGS)  -c -o $@  $< $(LDFLAGS) $(LIBS)


install-strip : $(TARGET)
	 install -d $(DESTDIR)/lib
	 install -p -D -m  0755 $(TARGET) $(DESTDIR)/lib
	 install -d $(DESTDIR)/include
	 cp ./include/* $(DESTDIR)/include/ -rf

clean :
	rm $(TARGET) $(OBJS)
