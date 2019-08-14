#include <stdint.h>
#define MAX_CYCLES 32

struct zpn_key
{
	uint64_t counter_mask[4];
	uint64_t cadd[MAX_CYCLES][4];
	uint32_t cycles;
};
typedef uint64_t data256[4]; //256bit
void zpn_bytes_to_data256(uint8_t *bytes, data256 x);
int zpn_expand_key(uint8_t *key, int64_t length, int64_t cycles, struct zpn_key * exp_key);

void zpn_encrypt(uint64_t nounce, uint64_t counter, const struct zpn_key *key, const data256 raw, data256 enc);
void zpn_decrypt(uint64_t nounce, uint64_t counter, const struct zpn_key *key, data256 raw, const data256 enc);

