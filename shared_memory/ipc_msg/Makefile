all:test_read test_write

test_read:test_read.o ipc_msg_queue.o ipc_msg_queue.h
	gcc $^ -o $@

test_write:test_write.o ipc_msg_queue.o ipc_msg_queue.h
	gcc $^ -o $@

%.o:%.c
	gcc -DSTREAM -c -o $@ $<

clean:
	-rm *.o test_read test_write