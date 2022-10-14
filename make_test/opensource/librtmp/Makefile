TARGET  := librtmp.so

CC      := arm-linux-g++
LIBS    :=
LDFLAGS :=
DEFINES :=
INCLUDE := -I./include/librtmp
CFLAGS  :=  -w -O3 -fPIC -fpermissive $(DEFINES) $(INCLUDE)
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
	install -d $(DESTDIR)/include/librtmp
	install -p -D -m 0755 ./include/librtmp/*  $(DESTDIR)/include/librtmp

clean :
	rm $(TARGET) $(OBJS)

