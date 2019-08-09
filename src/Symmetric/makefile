CFLAGS+= -O3 -g -fPIC -m64 -Wall -Wextra -fno-strict-aliasing -fwrapv
#-fsanitize=undefined
CC=clang
all: zpn512sum test zaphnath.S

test.o: test.c zaphnath.h
zaphnath.o: zaphnath.c zaphnath.h
zpn512sum.o: zpn512sum.c zpn_common.h zpn_hash.h
zpn_hash.o: zpn_hash.c zpn_common.h zpn_hash.h
zaphnath.S: zaphnath.c zaphnath.h
	$(CC) -S -g zaphnath.c  -o zaphnath.S 
test: test.o zaphnath.o
	$(CC) test.o zaphnath.o -lasan -lubsan -o test 
zpn512sum: zpn512sum.o zpn_hash.o
	$(CC) zpn512sum.o zpn_hash.o -o zpn512sum 
