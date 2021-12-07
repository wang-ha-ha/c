all:sm_read sm_write

sm_read:read.o
	gcc $^ -o $@

sm_write:write.o
	gcc $^ -o $@

clean:
	-rm *.o sm_read sm_write