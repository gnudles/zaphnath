all: test

test.o: test.c zaphnath.h
lookups.o: lookups.c
zaphnath.o: zaphnath.c zaphnath.h
test: test.o lookups.o zaphnath.o
	gcc test.o lookups.o zaphnath.o -o test

