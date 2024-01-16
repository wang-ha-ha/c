all:test_read test_write

test_read:test_read.o ringbuf.o ringbuf.h
	gcc $^ -o $@

test_write:test_write.o ringbuf.o ringbuf.h
	gcc $^ -o $@

clean:
	-rm *.o test_read test_write