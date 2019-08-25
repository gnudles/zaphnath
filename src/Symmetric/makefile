#CFLAGS+= -O3 -g -fPIC -m64 -Wall -Wextra -fno-strict-aliasing -fwrapv
CFLAGS+= -O3 -g -fPIC  -Wall -Wextra 
#-fsanitize=undefined
CCMIPS=mips-linux-gnu-gcc
CC=clang

meson_gcc:
	mkdir -p build
	meson --buildtype debug build/gcc-debug
	meson --buildtype release build/gcc-release
meson_clang:
	mkdir -p build
	CC=clang CXX=clang++ meson --buildtype debug build/clang-debug
	CC=clang CXX=clang++ meson --buildtype release build/clang-release
ninja:
	ninja -C build/clang-release
	ninja -C build/clang-debug
	ninja -C build/gcc-release
	ninja -C build/gcc-debug
meson_mips:
	mkdir -p build
	meson build/mips --cross-file mips_arch
ninja_mips:
	ninja -C build/mips
all: zpn512sum encrypt_test ZPN_RNG zaphnath.S
mips: zpn512sum_MIPS ZPN_RNG_MIPS encrypt_test_MIPS

encrypt_test.o: encrypt_test.c zaphnath.h
zaphnath.o: zaphnath.c zaphnath.h zpn_common.h zpn_hash.h
zpn512sum.o: zpn512sum.c zpn_common.h zpn_hash.h i_stream.h
i_stream.o: i_stream.c i_stream.h
zpn_hash_rng.o: zpn_hash_rng.c zpn_common.h zpn_hash.h
zpn_hash.o: zpn_hash.c zpn_common.h zpn_hash.h
zaphnath.S: zaphnath.c zaphnath.h
	$(CC) -S -g zaphnath.c  -o zaphnath.S 
encrypt_test: encrypt_test.o zaphnath.o zpn_hash.o
	$(CC) encrypt_test.o zaphnath.o zpn_hash.o -o encrypt_test 
	#$(CC) encrypt_test.o zaphnath.o -lasan -lubsan -o encrypt_test 
zpn512sum: zpn512sum.o zpn_hash.o i_stream.o
	$(CC) zpn512sum.o zpn_hash.o i_stream.o -o zpn512sum
ZPN_RNG: zpn_hash_rng.o zpn_hash.o
	$(CC) zpn_hash_rng.o zpn_hash.o -o ZPN_RNG
#these are used for big endian testings... now obsolete when moving to meson
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
