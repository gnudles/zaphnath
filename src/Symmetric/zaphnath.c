
#include <malloc.h>
#include <assert.h>

#include "zaphnath.h"
#include "zpn_common.h"
#include "zpn_hash.h"





static inline uint32_t max_ui32(uint32_t a, uint32_t b)
{ if (a>b) return a; return b; }

void zpn_bytes_to_data256(uint8_t *bytes, data256 x)
{
    for (int i = 0; i < 4; ++i)
    {
        x[i] = 0;
    }
    for (int i = 0; i < 32; ++i)
    {
        x[i/8] |= ((uint64_t)bytes[i])<<((i%8)*8);
    }
}


int zpn_expand_key(uint8_t *key, int64_t length, int64_t cycles, struct zpn_key * exp_key)
{
	assert(length>=0);
	uint32_t i;
	hash_chunk chunk;
	hash_sponge sponge;
	int step888 = 0;
    zpn_hash_init_sponge(sponge);

	while (length>=0)
	{
		if (length != 0)
		{
			zpn_hash_nbytes_to_chunk(key, (length<=64)?length:64, chunk);
		}
		if (length < 64)
		{
			zpn_hash_pad_last_chunk(chunk,length);
		}
		zpn_hash_absorb(sponge,chunk);
        zpn_hash_vertical_tilt(sponge,step888);
        step888+=ZPN_DEFAULT_STEP_INC;
		length -= 64;
	}
	if (cycles> MAX_CYCLES)
		cycles= MAX_CYCLES;
	exp_key->cycles=cycles;

	zpn_hash_sponge_obscure(sponge,step888);
	step888+=ZPN_DEFAULT_STEP_INC*ZPN_OBSCURE_STEPS;
	exp_key->counter_mask[0] = sponge[0];
	exp_key->counter_mask[1] = sponge[1];
	exp_key->counter_mask[2] = sponge[2];
	exp_key->counter_mask[3] = sponge[3];
	zpn_hash_sponge_obscure(sponge,step888);
	step888+=ZPN_DEFAULT_STEP_INC*ZPN_OBSCURE_STEPS;
	exp_key->counter_mask[0] ^= sponge[0];
	exp_key->counter_mask[1] ^= sponge[1];
	exp_key->counter_mask[2] ^= sponge[2];
	exp_key->counter_mask[3] ^= sponge[3];

		for (i=0;i<cycles;++i)
		{
			zpn_hash_sponge_obscure(sponge,step888);
			step888+=ZPN_DEFAULT_STEP_INC*ZPN_OBSCURE_STEPS;
			exp_key->cadd[i][0]=sponge[0];
			exp_key->cadd[i][1]=sponge[1];
			exp_key->cadd[i][2]=sponge[2];
			exp_key->cadd[i][3]=sponge[3];
		}
		
	

	return 0;
}

static inline void zpn_mixbits(uint64_t* in, uint64_t b1, uint64_t b2)
{
	uint64_t m[4]={b1&b2,b1&(~b2),b2&(~b1),~b1&~b2};
	uint64_t temp_in[4]={in[0],in[1],in[2],in[3]};
	in[0]=(m[0]&temp_in[0])|(m[1]&temp_in[1])|(m[2]&temp_in[2])|(m[3]&temp_in[3]);
	in[1]=(m[0]&temp_in[1])|(m[1]&temp_in[0])|(m[2]&temp_in[3])|(m[3]&temp_in[2]);
	in[2]=(m[0]&temp_in[2])|(m[1]&temp_in[3])|(m[2]&temp_in[0])|(m[3]&temp_in[1]);
	in[3]=(m[0]&temp_in[3])|(m[1]&temp_in[2])|(m[2]&temp_in[1])|(m[3]&temp_in[0]);
}// this function is the inverse of itself

static inline void zpn_mat(uint64_t *in)
{
/*
-1 -1  2  1
 2 -2 -1  2
 1  2  1 -1
 3 -3  2  4
*/
	uint64_t temp_in[4]={in[0],in[1],in[2],in[3]};

	in[0]=-temp_in[0]-temp_in[1]+(temp_in[2]<<1)+temp_in[3];
	in[1]=((temp_in[0]-temp_in[1]+temp_in[3])<<1)-temp_in[2];
	in[2]=temp_in[0]+(temp_in[1]<<1)+temp_in[2]-temp_in[3];

	in[3]=(temp_in[0]-temp_in[1])+((temp_in[0]-temp_in[1]+temp_in[2])<<1)+(temp_in[3]<<2);
}
static inline void zpn_imat(uint64_t *in)
{
	uint64_t temp_in[4]={in[0],in[1],in[2],in[3]};
/*
  7   9   5  -5
-17 -21 -11  12
  6   7   4  -4
-21 -26 -14  15
*/
        in[0]=((temp_in[0]+temp_in[1])<<3)-temp_in[0]+temp_in[1]+((temp_in[2]-temp_in[3])<<2)+temp_in[2]-temp_in[3];
        in[1]=((-temp_in[0]-temp_in[1])<<4)-temp_in[0]-temp_in[1]+((temp_in[3]-temp_in[1]-temp_in[2])<<2)+((temp_in[3]-temp_in[2])<<3)+temp_in[2];
        //[0]: -16 -1 = -17
        //[1]: -16 -1 -4 = -21
        //[2]: -4 -8 + 1 = -11
        //[3]: 8 + 4 =12
        in[2]=((temp_in[0]+temp_in[2]-temp_in[3])<<2) + (temp_in[1]<<3)-temp_in[1] +(temp_in[0]<<1);
        in[3]=((-temp_in[0]-temp_in[1]-temp_in[2]+temp_in[3])<<4)-temp_in[3]+((temp_in[2]-temp_in[1])<<1) - (temp_in[1]<<3) -temp_in[0] -(temp_in[0]<<2);

}


static inline void free_rotate64(uint64_t *raw, uint64_t shifts)//only 24 bits from shifts are used.
{
	
	raw[0]=ROTL64(raw[0],shifts&0x3f);shifts>>=6;
	raw[1]=ROTL64(raw[1],shifts&0x3f);shifts>>=6;
	raw[2]=ROTL64(raw[2],shifts&0x3f);shifts>>=6;
	raw[3]=ROTL64(raw[3],shifts&0x3f);shifts>>=6;
	
}
static inline void free_rotate32(uint64_t *raw, uint64_t shifts) //only 40 bits from shifts are used.
{
	uint64_t s1,s2;
	for (int i = 0; i< 4; ++i)
	{
		s1 = shifts&0x1f;shifts>>=5;s2 = shifts&0x1f;shifts>>=5;
		raw[i] = ROTL32(raw[i]&0xffffffff,s1) | ((uint64_t)ROTL32((raw[i]>>32)&0xffffffff,s2) << 32);
	}
	
}
static inline void free_rotate16(uint64_t *raw, uint64_t shifts) //64 bits from shifts are used.
{
	uint64_t s1,s2,s3,s4;
	for (int i = 0; i< 4; ++i)
	{
		s1 = shifts&0xf;shifts>>=4;s2 = shifts&0xf;shifts>>=4;s3 = shifts&0xf;shifts>>=4;s4 = shifts&0xf;shifts>>=4;
		raw[i] = ((uint64_t)ROTL16(raw[i]&0xffff,s1)) | ((uint64_t)ROTL16((raw[i]>>16)&0xffff,s2) << 16)
		| ((uint64_t)ROTL16((raw[i]>>32)&0xffff,s3) << 32) | ((uint64_t)ROTL16((raw[i]>>48)&0xffff,s4) << 48);
	}
	
}

static inline void rev_free_rotate64(uint64_t *raw, uint64_t shifts)//only 24 bits from shifts are used.
{
	raw[0]=ROTR64(raw[0],shifts&0x3f);shifts>>=6;
	raw[1]=ROTR64(raw[1],shifts&0x3f);shifts>>=6;
	raw[2]=ROTR64(raw[2],shifts&0x3f);shifts>>=6;
	raw[3]=ROTR64(raw[3],shifts&0x3f);shifts>>=6;
}
static inline void rev_free_rotate32(uint64_t *raw, uint64_t shifts) //only 40 bits from shifts are used.
{
	uint64_t s1,s2;
	for (int i = 0; i< 4; ++i)
	{
		s1 = shifts&0x1f;shifts>>=5;s2 = shifts&0x1f;shifts>>=5;
		raw[i] = ROTR32(raw[i]&0xffffffff,s1) | ((uint64_t)ROTR32((raw[i]>>32)&0xffffffff,s2) << 32);
	}
}
static inline void rev_free_rotate16(uint64_t *raw, uint64_t shifts) //64 bits from shifts are used.
{
uint64_t s1,s2,s3,s4;
	for (int i = 0; i< 4; ++i)
	{
		s1 = shifts&0xf;shifts>>=4;s2 = shifts&0xf;shifts>>=4;s3 = shifts&0xf;shifts>>=4;s4 = shifts&0xf;shifts>>=4;
		raw[i] = ((uint64_t)ROTR16(raw[i]&0xffff,s1)) | ((uint64_t)ROTR16((raw[i]>>16)&0xffff,s2) << 16)
		| ((uint64_t)ROTR16((raw[i]>>32)&0xffff,s3) << 32) | ((uint64_t)ROTR16((raw[i]>>48)&0xffff,s4) << 48);
	}
}

#define ROR64(x, r) ((x >> (r)) | (x << (64 - (r))))
#define ROL64(x, r) ((x << (r)) | (x >> (64 - (r))))
#define SPECK_R(x, y, k) (x = ROR64(x, 8+(y&15)), x += y, x ^= k, y = ROL64(y, 3+(x&31)), y ^= x)
#define REV_SPECK_R(x, y, k) (y ^= x, y = ROR64(y, 3+(x&31)), x ^= k, x -= y, x = ROL64(x, 8+(y&15)))

void print_uint64_array(const uint64_t *x, int l)
{
    char temp[17];
    temp[16] = 0;
    for (int i= l-1; i>=0; --i)
    {
        int64toHex(x[i],temp);
        printf("%s",temp);
    }
}
#define DEBUG
void zpn_encrypt(uint64_t nounce, uint64_t counter, const struct zpn_key *key, const data256 raw, data256 enc)
{
	uint64_t counter_df[4];
	uint64_t bitmixer[2];
	uint64_t shifts[2];
	int32_t i;

#ifdef DEBUG
	printf("zpn_encrypt: nounce: %016lx counter: %016lx raw:\n",nounce,counter);
	print_uint64_array(raw,4);
	printf("\n");
#endif
	counter_df[0]=counter + key->counter_mask[0];
	counter_df[1]=key->counter_mask[1];
	counter_df[2]=nounce + key->counter_mask[2];
	counter_df[3]=key->counter_mask[3];
#ifdef DEBUG
	printf("counter before mix\n");
	print_uint64_array(counter_df,4);
	printf("\n");
#endif
	ZPN_TWISTER(counter_df[0], counter_df[1], counter_df[2], counter_df[3])
	ZPN_TWISTER_MULTI(counter_df[0], counter_df[1], counter_df[2], counter_df[3])
	zpn_mat(counter_df);

#ifdef DEBUG
	printf("counter after mix\n");
	print_uint64_array(counter_df,4);
	printf("\n");
#endif


	*((v4qw*)enc)=(*((v4qw*)raw))+(*((v4qw*)key->counter_mask)); // enc = raw + counter_df
#ifdef DEBUG
	printf("input added with counter\n");
	print_uint64_array(enc,4);
	printf("\n");
#endif

	shifts[0]=counter_df[0]; // for 4*64bit (4*6=24) and for 8*32bit (8*5=40) (24+40=64)
	shifts[1]=counter_df[1]; // for 16*16 (16*4=64)
	bitmixer[0]=counter_df[2]; 
	bitmixer[1]=counter_df[3];
	zpn_mixbits(enc, bitmixer[0], bitmixer[1]);
	free_rotate16(enc,shifts[1]);
	free_rotate64(enc,shifts[0]);
	free_rotate32(enc,shifts[0]>>24);
	for (i =0 ; (uint32_t)i< key->cycles ; ++i)
	{
		ZPN_TWISTER(enc[0],enc[1],enc[2],enc[3])
		ZPN_TWISTER_MULTI(enc[0],enc[1],enc[2],enc[3])
		zpn_imat(enc);
		*((v4qw*)enc) += (*((v4qw*)key->cadd[i]));
	}

#ifdef DEBUG
	printf("encrypt: output\n");
	print_uint64_array(enc,4);
	printf("\n");
#endif
}

void zpn_decrypt(uint64_t nounce, uint64_t counter, const struct zpn_key *key, data256 raw, const data256 enc)
{
	uint64_t counter_df[4];
	uint64_t bitmixer[2];
	uint64_t shifts[2];
	int32_t i;
	
#ifdef DEBUG
	printf("zpn_decrypt: nounce: %016lx counter: %016lx enc:\n",nounce,counter);
	print_uint64_array(enc,4);
	printf("\n");
#endif
	counter_df[0]=counter + key->counter_mask[0];
	counter_df[1]=key->counter_mask[1];
	counter_df[2]=nounce + key->counter_mask[2];
	counter_df[3]=key->counter_mask[3];
	ZPN_TWISTER(counter_df[0], counter_df[1], counter_df[2], counter_df[3])
	ZPN_TWISTER_MULTI(counter_df[0], counter_df[1], counter_df[2], counter_df[3])
	zpn_mat(counter_df);
	
#ifdef DEBUG
	printf("decrypt: counter after mix\n");
	print_uint64_array(counter_df,4);
	printf("\n");
#endif
	*((v4qw*)raw) = *((v4qw*)enc);
	for (i = key->cycles - 1 ; i >= 0 ; --i)
	{
		*(v4qw*)raw -= *(v4qw*)key->cadd[i];
		zpn_mat(raw);
		REV_ZPN_TWISTER_MULTI(raw[0],raw[1],raw[2],raw[3])
		REV_ZPN_TWISTER(raw[0],raw[1],raw[2],raw[3])
	}
	
	shifts[0]=counter_df[0]; // for 4*64bit (4*6=24) and for 8*32bit (8*5=40) (24+40=64)
	shifts[1]=counter_df[1]; // for 16*16 (16*4=64)
	bitmixer[0]=counter_df[2]; 
	bitmixer[1]=counter_df[3];
	
	rev_free_rotate32(raw,shifts[0]>>24);
	rev_free_rotate64(raw,shifts[0]);
	rev_free_rotate16(raw,shifts[1]);
	zpn_mixbits(raw, bitmixer[0], bitmixer[1]);
	
	
	*((v4qw*)raw)-=(*((v4qw*)key->counter_mask));
#ifdef DEBUG
	printf ("decrypt: output\n");
	print_uint64_array(raw,4);
	printf("\n");
#endif

}

