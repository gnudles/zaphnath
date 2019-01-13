#include <stdint.h>
#define MAX_CYCLES 50

struct zpn_key
{
	uint32_t cycles;
	uint32_t shifts;
	uint64_t counter_mask[2];
	uint64_t final_xor[4];
	uint8_t lookup[2][256];
	uint64_t cxor[MAX_CYCLES][4];
};
int zpn_expand_key(uint8_t *key, uint32_t length, uint32_t cycles, struct zpn_key * ts);

void zpn_encrypt(uint64_t nounce, uint64_t counter, struct zpn_key *key, uint8_t *data, uint8_t *out);
void zpn_decrypt(uint64_t nounce, uint64_t counter, struct zpn_key *key, uint8_t *data, uint8_t *out);

