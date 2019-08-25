#include <stdint.h>
#include "rotate_macro.h"
typedef uint64_t hash_sponge[34];//2176bit
typedef uint64_t hash_chunk[8]; //512bit
#define ZPN_DEFAULT_STEP_INC  0x5d
#define ZPN_OBSCURE_STEPS 4
struct i_stream;
void zpn_hash_calculate(struct i_stream *stream, hash_chunk hash_out);
void zpn_hash_init_sponge(hash_sponge sponge);
void zpn_hash_bytes_to_chunk(uint8_t *bytes, hash_chunk chunk);
void zpn_hash_nbytes_to_chunk(uint8_t *bytes, int length, hash_chunk chunk);
void zpn_hash_vertical_tilt(hash_sponge sponge, int step888);
void zpn_hash_sponge_obscure(hash_sponge sponge, int step888);
void zpn_hash_pad_last_chunk(hash_chunk chunk,unsigned int length_of_last_chunk);

#if defined( __GNUC__) && !defined(__clang__)
__attribute__((optimize("unroll-loops")))
#endif

static inline void zpn_hash_absorb(hash_sponge sponge, hash_chunk chunk)
{
#ifdef __clang__
#pragma clang loop unroll(full)
#endif
    for (int i=0;i<8;++i)
    {
		sponge[33]=ROTL64(sponge[33]+chunk[i],1);
		sponge[i]^=chunk[i];
                sponge[32]^=chunk[i];

    }
}
