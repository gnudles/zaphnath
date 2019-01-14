CFLAGS+= -O2
all: test

test.o: test.c zaphnath.h
zaphnath.o: zaphnath.c zaphnath.h
test: test.o zaphnath.o
	gcc test.o zaphnath.o -o test

