#include <stdint.h>
typedef uint64_t hash_sponge[32];//2048bit
typedef uint64_t hash_chunk[8]; //512bit
#define ZPN_DEFAULT_STEP_INC  0x5d
#define ZPN_OBSCURE_STEPS 4
void zpn_hash_init_sponge(hash_sponge sponge);
void zpn_hash_bytes_to_chunk(uint8_t *bytes, hash_chunk chunk);
void zpn_hash_nbytes_to_chunk(uint8_t *bytes, unsigned int length, hash_chunk chunk);
void zpn_hash_vertical_tilt(hash_sponge sponge, int step888);
void zpn_hash_sponge_obscure(hash_sponge sponge, int step888);
void zpn_hash_pad_last_chunk(hash_chunk chunk,unsigned int length_of_last_chunk);
inline void zpn_hash_absorb(hash_sponge sponge, hash_chunk chunk)
{
    for (int i=0;i<8;++i)
		sponge[i]^=chunk[i];
}