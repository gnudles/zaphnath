#include <stdint.h>
#define MAX_CYCLES 20

struct zpn_key
{
	uint64_t counter_mask[4];
	uint64_t final_add[4];
//	uint64_t all_shifts_xor[4]; // this is the xor of all cycles of cshifts and cmixbits
	uint64_t cshifts_mixbits[MAX_CYCLES][4];
	uint64_t cadd[MAX_CYCLES][4];
	uint32_t cycles;
};
int zpn_expand_key(uint8_t *key, uint32_t length, uint32_t cycles, struct zpn_key * exp_key);

void zpn_encrypt(uint64_t nounce, uint64_t counter, struct zpn_key *key, uint8_t *raw, uint8_t *enc);
void zpn_decrypt(uint64_t nounce, uint64_t counter, struct zpn_key *key, uint8_t *raw, uint8_t *enc);

