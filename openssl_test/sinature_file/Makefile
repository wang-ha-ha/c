
OBJ=sinature_tool.o sinature_rsa_tool.o
CHECK_OBJ=check_pss.o sinature_rsa_tool.o
CC=gcc

OPENSSL_LIB=./exlib/openssl/lib
OPENSSL_INC=./exlib/openssl/include
all:sinature_tool check_pss test

sinature_tool:${OBJ}
	${CC} -o $@ $^  -I. -lssl -lcrypto -g

check_pss:${CHECK_OBJ}
	${CC} -o $@ $^ -I. -lssl -lcrypto -g

test:test.o sinature_rsa_tool.o
	${CC} -o $@ $^ -I. -lssl -lcrypto -g

%.o:%.c
	$(CC) -c -o $@ $< -I.

.PHONEY:clean
clean:
	rm -f *.o *~ sinature_tool check_pss test
