all:sm_read sm_write

sm_read:read.o shared_struct.h
	gcc $^ -o $@

sm_write:write.o shared_struct.h
	gcc $^ -o $@

clean:
	-rm *.o sm_read sm_write