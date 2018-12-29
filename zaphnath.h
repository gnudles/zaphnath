#include <stdint.h>
#define MAX_CYCLES 50

struct time_schedule
{
	uint64_t counter_mask[2];
	uint64_t final_xor[2];
	uint32_t cycles;
	uint16_t index[MAX_CYCLES][16];
};
int zpn_expand_key(uint8_t *key, uint32_t length, uint32_t cycles, struct time_schedule * ts);

void zpn_encrypt(uint64_t nounce, uint64_t counter, struct time_schedule *ts, uint8_t *data, uint8_t *out);
void zpn_decrypt(uint64_t nounce, uint64_t counter, struct time_schedule *ts, uint8_t *data, uint8_t *out);
