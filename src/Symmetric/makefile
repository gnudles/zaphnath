#CFLAGS+= -O3 -g -fPIC -m64 -Wall -Wextra -fno-strict-aliasing -fwrapv
CFLAGS+= -O3 -g -fPIC  -Wall -Wextra 
#-fsanitize=undefined
CCMIPS=mips-linux-gnu-gcc
CC=clang
all: zpn512sum encrypt_test ZPN_RNG zaphnath.S
mips: zpn512sum_MIPS ZPN_RNG_MIPS encrypt_test_MIPS

encrypt_test.o: encrypt_test.c zaphnath.h
zaphnath.o: zaphnath.c zaphnath.h zpn_common.h zpn_hash.h
zpn512sum.o: zpn512sum.c zpn_common.h zpn_hash.h
zpn_hash_rng.o: zpn_hash_rng.c zpn_common.h zpn_hash.h
zpn_hash.o: zpn_hash.c zpn_common.h zpn_hash.h
zaphnath.S: zaphnath.c zaphnath.h
	$(CC) -S -g zaphnath.c  -o zaphnath.S 
encrypt_test: encrypt_test.o zaphnath.o zpn_hash.o
	$(CC) encrypt_test.o zaphnath.o zpn_hash.o -o encrypt_test 
	#$(CC) encrypt_test.o zaphnath.o -lasan -lubsan -o encrypt_test 
zpn512sum: zpn512sum.o zpn_hash.o
	$(CC) zpn512sum.o zpn_hash.o -o zpn512sum
ZPN_RNG: zpn_hash_rng.o zpn_hash.o
	$(CC) zpn_hash_rng.o zpn_hash.o -o ZPN_RNG
#these are used for big endian testings...
zpn512sum.mips.o: zpn512sum.c zpn_common.h zpn_hash.h
	$(CCMIPS) -c $(CFLAGS) zpn512sum.c -o zpn512sum.mips.o
zpn_hash_rng.mips.o: zpn_hash_rng.c zpn_common.h zpn_hash.h
	$(CCMIPS) -c $(CFLAGS) zpn_hash_rng.c -o zpn_hash_rng.mips.o
zpn_hash.mips.o: zpn_hash.c zpn_common.h zpn_hash.h
	$(CCMIPS) -c $(CFLAGS) zpn_hash.c -o zpn_hash.mips.o
zaphnath.mips.o: zaphnath.c zaphnath.h zpn_common.h zpn_hash.h
	$(CCMIPS) -c $(CFLAGS) zaphnath.c -o zaphnath.mips.o
encrypt_test.mips.o: encrypt_test.c zaphnath.h
	$(CCMIPS) -c $(CFLAGS) encrypt_test.c -o encrypt_test.mips.o
encrypt_test_MIPS: encrypt_test.mips.o zaphnath.mips.o zpn_hash.mips.o
	$(CCMIPS)  encrypt_test.mips.o zaphnath.mips.o zpn_hash.mips.o -o encrypt_test_MIPS -static
zpn512sum_MIPS: zpn512sum.mips.o zpn_hash.mips.o
	$(CCMIPS)  zpn512sum.mips.o zpn_hash.mips.o -o zpn512sum_MIPS -static
ZPN_RNG_MIPS: zpn_hash_rng.mips.o zpn_hash.mips.o
	$(CCMIPS) zpn_hash_rng.mips.o zpn_hash.mips.o -o ZPN_RNG_MIPS -static