#CFLAGS+= -O3 -g -fPIC -m64 -Wall -Wextra -fno-strict-aliasing -fwrapv
CFLAGS+= -O3 -g -fPIC  -Wall -Wextra 
#-fsanitize=undefined
CCMIPS=mips-linux-gnu-gcc
CC=clang
all: zpn512sum zpn512sum_MIPS ZPN_RNG ZPN_RNG_MIPS test zaphnath.S

test.o: test.c zaphnath.h
zaphnath.o: zaphnath.c zaphnath.h zpn_common.h zpn_hash.h
zpn512sum.o: zpn512sum.c zpn_common.h zpn_hash.h
zpn_hash_rng.o: zpn_hash_rng.c zpn_common.h zpn_hash.h
zpn_hash.o: zpn_hash.c zpn_common.h zpn_hash.h
zaphnath.S: zaphnath.c zaphnath.h
	$(CC) -S -g zaphnath.c  -o zaphnath.S 
test: test.o zaphnath.o zpn_hash.o
	$(CC) test.o zaphnath.o zpn_hash.o -o test 
	#$(CC) test.o zaphnath.o -lasan -lubsan -o test 
zpn512sum: zpn512sum.o zpn_hash.o
	$(CC) zpn512sum.o zpn_hash.o -o zpn512sum
ZPN_RNG: zpn_hash_rng.o zpn_hash.o
	$(CC) zpn_hash_rng.o zpn_hash.o -o ZPN_RNG
zpn512sum.mips.o: zpn512sum.c zpn_common.h zpn_hash.h
	$(CCMIPS) -c $(CFLAGS) zpn512sum.c -o zpn512sum.mips.o
zpn_hash_rng.mips.o: zpn_hash_rng.c zpn_common.h zpn_hash.h
	$(CCMIPS) -c $(CFLAGS) zpn_hash_rng.c -o zpn_hash_rng.mips.o
zpn_hash.mips.o: zpn_hash.c zpn_common.h zpn_hash.h
	$(CCMIPS) -c $(CFLAGS) zpn_hash.c -o zpn_hash.mips.o
zpn512sum_MIPS: zpn512sum.mips.o zpn_hash.mips.o
	$(CCMIPS)  zpn512sum.mips.o zpn_hash.mips.o -o zpn512sum_MIPS -static
ZPN_RNG_MIPS: zpn_hash_rng.mips.o zpn_hash.mips.o
	$(CCMIPS) -static zpn_hash_rng.mips.o zpn_hash.mips.o -o ZPN_RNG_MIPS