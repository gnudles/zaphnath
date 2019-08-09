#include <stdint.h>
typedef uint64_t hash_sponge[32];//2048bit
typedef uint64_t hash_chunk[8]; //512bit
#define DEFAULT_STEP_INC  0x57
void init_sponge(hash_sponge sponge);
void bytes_to_chunk(uint8_t *bytes, hash_chunk chunk);
void zpn_feed_hash_sponge(hash_sponge sponge, hash_chunk chunk, int step888);
void zpn_hash_sponge_obscure(hash_sponge sponge);
void pad_last_chunk(hash_chunk chunk,unsigned int length_of_last_chunk);