CFLAGS+= -O3 -g -fPIC -m64 -Wall -Wextra -fno-strict-aliasing -fwrapv 
#-fsanitize=undefined
CC=clang
all: test zaphnath.S

test.o: test.c zaphnath.h
zaphnath.o: zaphnath.c zaphnath.h
zaphnath.S: zaphnath.c zaphnath.h
	$(CC) -S -g zaphnath.c  -o zaphnath.S 
test: test.o zaphnath.o
	$(CC) test.o zaphnath.o -lasan -lubsan -o test 

