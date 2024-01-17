all:test_read test_write

test_read:test_read.o ipc_msg_queue.o ipc_msg_queue.h
	gcc $^ -o $@ -lpthread

test_write:test_write.o ipc_msg_queue.o ipc_msg_queue.h
	gcc $^ -o $@ -lpthread

%.o:%.c
	gcc -DSTREAM -lpthread -c -o $@ $<

clean:
	-rm *.o test_read test_write