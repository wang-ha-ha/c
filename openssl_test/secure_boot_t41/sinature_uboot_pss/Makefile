BIN=scboot_tool
SRC=scboot_tool.c scboot_rsa_tool.c
SRCC=check_pss.c scboot_rsa_tool.c pss.c rsa.c bignum.c
BINCHECK=check_pss
CC=gcc
OPENSSL_LIB=./exlib/openssl/lib
OPENSSL_INC=./exlib/openssl/include
all:
	${CC} ${SRC} -o ${BIN} -I$(OPENSSL_INC) -L$(OPENSSL_LIB) -lssl -lcrypto
	${CC} ${SRCC} -I./ -o ${BINCHECK} -I$(OPENSSL_INC) -L$(OPENSSL_LIB) -lssl -lcrypto
.PHONEY:clean
clean:
	rm -f *.o *~

distclean: clean
	rm -f ${BIN}
