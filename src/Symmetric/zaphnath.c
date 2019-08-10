
#include <malloc.h>

#include "zaphnath.h"
#include "zpn_common.h"

//this is for meanwhile, until I add a better sponge function.
#define bjenkins_mix3(a,b,c) \
{ \
  a -= c;  a ^= ROTL32(c, 4);  c += b; \
  b -= a;  b ^= ROTL32(a, 6);  a += c; \
  c -= b;  c ^= ROTL32(b, 8);  b += a; \
  a -= c;  a ^= ROTL32(c,16);  c += b; \
  b -= a;  b ^= ROTL32(a,19);  a += c; \
  c -= b;  c ^= ROTL32(b, 4);  b += a; \
}



static inline void zpn_mixbits(uint64_t* in,uint64_t *out, uint64_t b1, uint64_t b2)
{
	uint64_t m[4]={b1&b2,b1&(~b2),b2&(~b1),~b1&~b2};
	out[0]=(m[0]&in[0])|(m[1]&in[1])|(m[2]&in[2])|(m[3]&in[3]);
	out[1]=(m[0]&in[1])|(m[1]&in[0])|(m[2]&in[3])|(m[3]&in[2]);
	out[2]=(m[0]&in[2])|(m[1]&in[3])|(m[2]&in[0])|(m[3]&in[1]);
	out[3]=(m[0]&in[3])|(m[1]&in[2])|(m[2]&in[1])|(m[3]&in[0]);
}// this function is the inverse of itself

static inline uint32_t max_ui32(uint32_t a, uint32_t b)
{ if (a>b) return a; return b; }



//this is for meanwhile, until I add a better sponge function.
void zpn_feed_sponge(uint8_t* sponge, uint32_t length, uint64_t feed)
{
	((uint64_t*)sponge)[0]^= feed;
	uint32_t i,j,k;
	for (i=0;i<length/4;++i)
	{
		for (j=0;j<length/4;++j)
		{
			for (k=0;k<length/4;++k)
			{
				if(i!=k && j!=k && i!=j)
				{
					bjenkins_mix3(((uint32_t*)sponge)[i],
					((uint32_t*)sponge)[j],
					((uint32_t*)sponge)[k])
				}
			}
		}
	}
}

int zpn_expand_key(uint8_t *key, uint32_t length, uint32_t cycles, struct zpn_key * exp_key)
{
	uint32_t i;
	uint64_t feeder;
	if (cycles> MAX_CYCLES)
		cycles= MAX_CYCLES;
	exp_key->cycles=cycles;
	if (key && length>1)
	{
		uint32_t sponge_len =
		 (max_ui32(4*length, 256)+0xf)&(~0x0000000f);
		uint8_t * sponge = (uint8_t*)malloc(sponge_len);
	
		if (sponge == 0)
			return -1; // allocation error
		memset(sponge, 0, sponge_len);
		feeder = 0;
		for (i=0; i < length; ++i)
		{
			feeder = ROTL64(feeder,8)+key[i];
			if ((i & 7) == 0)
				zpn_feed_sponge(sponge,sponge_len,feeder);
		}
		zpn_feed_sponge(sponge,sponge_len,feeder);
		exp_key->counter_mask[0] = ((uint64_t*)sponge)[0];
		zpn_feed_sponge(sponge,sponge_len,0);
		exp_key->counter_mask[1] = ((uint64_t*)sponge)[0];
		zpn_feed_sponge(sponge,sponge_len,0);
		exp_key->counter_mask[2] = ((uint64_t*)sponge)[0];
		zpn_feed_sponge(sponge,sponge_len,0);
		exp_key->counter_mask[3] = ((uint64_t*)sponge)[0];
//		ts->all_shifts_xor[0]=ts->all_shifts_xor[1]=ts->all_shifts_xor[2]=ts->all_shifts_xor[3]=0;
		for (i=0;i<cycles;++i)
		{
			zpn_feed_sponge(sponge,sponge_len,0);
			exp_key->cadd[i][0]=((uint64_t*)sponge)[0];
			zpn_feed_sponge(sponge,sponge_len,0);
			exp_key->cadd[i][1]=((uint64_t*)sponge)[0];
			zpn_feed_sponge(sponge,sponge_len,0);
			exp_key->cadd[i][2]=((uint64_t*)sponge)[0];
			zpn_feed_sponge(sponge,sponge_len,0);
			exp_key->cadd[i][3]=((uint64_t*)sponge)[0];
//			zpn_feed_sponge(sponge,sponge_len,0);
//			exp_key->cshifts_mixbits[i][0]=((uint64_t*)sponge)[0];
//			zpn_feed_sponge(sponge,sponge_len,0);
//			exp_key->cshifts_mixbits[i][1]=((uint64_t*)sponge)[0];
/*			ts->all_shifts_xor[0]^=ts->cshifts[i][0];
			ts->all_shifts_xor[1]^=ts->cshifts[i][1];
			ts->all_shifts_xor[2]^=ts->cmixbits[i][0];
			ts->all_shifts_xor[3]^=ts->cmixbits[i][1];*/
		}
		free(sponge);
	}
	else
	{
		uint8_t test_key[32]={ 79, 231, 208, 186, 130, 112, 103, 220, 86, 139, 86, 168, 92, 183, 249, 23, 
							153, 227, 4, 202, 104, 80, 73, 81, 28, 44, 43, 164, 92, 181, 125, 200 };
		zpn_expand_key(test_key, 32, cycles, exp_key);
	}
	return 0;
}

static inline void zpn_mat(uint8_t *in, uint8_t *out)
{
/*
-1 -1  2  1
 2 -2 -1  2
 1  2  1 -1
 3 -3  2  4
*/
	uint64_t temp_in[4];
	uint64_t temp_out[4];
	memcpy(temp_in, in, 4*8);
	temp_out[0]=-temp_in[0]-temp_in[1]+(temp_in[2]<<1)+temp_in[3];
	temp_out[1]=((temp_in[0]-temp_in[1]+temp_in[3])<<1)-temp_in[2];
	temp_out[2]=temp_in[0]+(temp_in[1]<<1)+temp_in[2]-temp_in[3];

	temp_out[3]=(temp_in[0]-temp_in[1])+((temp_in[0]-temp_in[1]+temp_in[2])<<1)+(temp_in[3]<<2);
	memcpy(out, temp_out, 4*8);
}
static inline void zpn_imat(uint8_t *in, uint8_t *out)
{
	uint64_t temp_in[4];
	uint64_t temp_out[4];
	memcpy(temp_in, in, 4*8);
/*
  7   9   5  -5
-17 -21 -11  12
  6   7   4  -4
-21 -26 -14  15
*/
        temp_out[0]=((temp_in[0]+temp_in[1])<<3)-temp_in[0]+temp_in[1]+((temp_in[2]-temp_in[3])<<2)+temp_in[2]-temp_in[3];
        temp_out[1]=((-temp_in[0]-temp_in[1])<<4)-temp_in[0]-temp_in[1]+((temp_in[3]-temp_in[1]-temp_in[2])<<2)+((temp_in[3]-temp_in[2])<<3)+temp_in[2];
        //[0]: -16 -1 = -17
        //[1]: -16 -1 -4 = -21
        //[2]: -4 -8 + 1 = -11
        //[3]: 8 + 4 =12
        temp_out[2]=((temp_in[0]+temp_in[2]-temp_in[3])<<2) + (temp_in[1]<<3)-temp_in[1] +(temp_in[0]<<1);
        temp_out[3]=((-temp_in[0]-temp_in[1]-temp_in[2]+temp_in[3])<<4)-temp_in[3]+((temp_in[2]-temp_in[1])<<1) - (temp_in[1]<<3) -temp_in[0] -(temp_in[0]<<2);
	memcpy(out, temp_out, 4*8);
}


static inline void free_rotate64(uint8_t *raw, uint64_t shifts)//only 24 bits from shifts are used.
{
	uint64_t temp[4];
	memcpy(temp,raw, 4*8);
	temp[0]=ROTL64(temp[0],shifts&0x3f);shifts>>=6;
	temp[1]=ROTL64(temp[1],shifts&0x3f);shifts>>=6;
	temp[2]=ROTL64(temp[2],shifts&0x3f);shifts>>=6;
	temp[3]=ROTL64(temp[3],shifts&0x3f);shifts>>=6;
	memcpy(raw,temp, 4*8);
}
static inline void free_rotate32(uint8_t *raw, uint64_t shifts) //only 40 bits from shifts are used.
{
	uint32_t temp[8];
	memcpy(temp,raw, 8*4);
	int b;
	for (b =0 ; b < 8 ; b+=4)
	{
		temp[b+0]=ROTL32(temp[b+0],shifts&0x1f);shifts>>=5;
		temp[b+1]=ROTL32(temp[b+1],shifts&0x1f);shifts>>=5;
		temp[b+2]=ROTL32(temp[b+2],shifts&0x1f);shifts>>=5;
		temp[b+3]=ROTL32(temp[b+3],shifts&0x1f);shifts>>=5;
	}
	memcpy(raw,temp, 8*4);
}
static inline void free_rotate16(uint8_t *raw, uint64_t shifts) //64 bits from shifts are used.
{
	uint16_t temp[16];
	memcpy(temp,raw, 16*2);
	int b;
	for (b =0 ; b < 16 ; b+=4)
	{
		temp[b+0]=ROTL16(temp[b+0],shifts&0xf);shifts>>=4;
		temp[b+1]=ROTL16(temp[b+1],shifts&0xf);shifts>>=4;
		temp[b+2]=ROTL16(temp[b+2],shifts&0xf);shifts>>=4;
		temp[b+3]=ROTL16(temp[b+3],shifts&0xf);shifts>>=4;
	}
	memcpy(raw,temp, 16*2);
}

static inline void rev_free_rotate64(uint8_t *raw, uint64_t shifts)//only 24 bits from shifts are used.
{
	uint64_t temp[4];
	memcpy(temp,raw, 4*8);
	temp[0]=ROTR64(temp[0],shifts&0x3f);shifts>>=6;
	temp[1]=ROTR64(temp[1],shifts&0x3f);shifts>>=6;
	temp[2]=ROTR64(temp[2],shifts&0x3f);shifts>>=6;
	temp[3]=ROTR64(temp[3],shifts&0x3f);shifts>>=6;
	memcpy(raw,temp, 4*8);
}
static inline void rev_free_rotate32(uint8_t *raw, uint64_t shifts) //only 40 bits from shifts are used.
{
	uint32_t temp[8];
	memcpy(temp,raw, 8*4);
	int b;
	for (b =0 ; b < 8 ; b+=4)
	{
		temp[b+0]=ROTR32(temp[b+0],shifts&0x1f);shifts>>=5;
		temp[b+1]=ROTR32(temp[b+1],shifts&0x1f);shifts>>=5;
		temp[b+2]=ROTR32(temp[b+2],shifts&0x1f);shifts>>=5;
		temp[b+3]=ROTR32(temp[b+3],shifts&0x1f);shifts>>=5;
	}
	memcpy(raw,temp, 8*4);
}
static inline void rev_free_rotate16(uint8_t *raw, uint64_t shifts) //64 bits from shifts are used.
{
	uint16_t temp[16];
	memcpy(temp,raw, 16*2);
	int b;
	for (b =0 ; b < 16 ; b+=4)
	{
		temp[b+0]=ROTR16(temp[b+0],shifts&0xf);shifts>>=4;
		temp[b+1]=ROTR16(temp[b+1],shifts&0xf);shifts>>=4;
		temp[b+2]=ROTR16(temp[b+2],shifts&0xf);shifts>>=4;
		temp[b+3]=ROTR16(temp[b+3],shifts&0xf);shifts>>=4;
	}
	memcpy(raw,temp, 16*2);
}

#define ROR64(x, r) ((x >> (r)) | (x << (64 - (r))))
#define ROL64(x, r) ((x << (r)) | (x >> (64 - (r))))
#define SPECK_R(x, y, k) (x = ROR64(x, 8+(y&15)), x += y, x ^= k, y = ROL64(y, 3+(x&31)), y ^= x)
#define REV_SPECK_R(x, y, k) (y ^= x, y = ROR64(y, 3+(x&31)), x ^= k, x -= y, x = ROL64(x, 8+(y&15)))

#define DEBUG
void zpn_encrypt(uint64_t nounce, uint64_t counter, struct zpn_key *key, uint8_t *raw, uint8_t *enc)
{
	uint64_t counter_df[4];
	uint64_t temp[4];
	uint64_t shifts[4];
	int32_t i;
#ifdef DEBUG
	int32_t j;
#endif	
#ifdef DEBUG
	printf("zpn_encrypt: nounce: %016lx counter: %016lx raw:\n",nounce,counter);
	for (j=0;j<32;++j)
		printf("%02x ",raw[j]);
	printf("\n");
#endif
	counter_df[0]=counter + key->counter_mask[0];
	counter_df[1]=key->counter_mask[1];
	counter_df[2]=nounce + key->counter_mask[2];
	counter_df[3]=key->counter_mask[3];
#ifdef DEBUG
	printf("counter before mix\n");
	for (j=0;j<32;++j)
		printf("%02x ",((uint8_t*)counter_df)[j]);
	printf("\n");
#endif
	zpn_imat((uint8_t*)counter_df,(uint8_t*)temp);
	ZPN_DIFFUSE256(temp[0], temp[1], temp[2], temp[3],
	0xa5a5a5a578787878,0x2d2d2d2d1e1e1e1e,0x969696964b4b4b4b,0x3c3c3c3cf0f0f0f0);
	zpn_imat((uint8_t*)temp,(uint8_t*)counter_df);
#ifdef DEBUG
	printf("counter after mix\n");
	for (j=0;j<32;++j)
		printf("%02x ",((uint8_t*)counter_df)[j]);
	printf("\n");
#endif

#define temp64 ((uint64_t*)temp)
#define temp32 ((uint32_t*)temp)
#define temp16 ((uint16_t*)temp)
#define temp8  ((uint8_t*) temp)
#define enc64 ((uint64_t*)enc)
#define enc32 ((uint32_t*)enc)
#define enc16 ((uint16_t*)enc)
#define enc8 ((uint8_t*)enc)
	*((v4qw*)enc64)=(*((v4qw*)raw))+(*((v4qw*)counter_df)); // enc = raw + counter_df
#ifdef DEBUG
	printf("input added with counter\n");
	for (j=0;j<32;++j)
		printf("%02x ",((uint8_t*)enc8)[j]);
	printf("\n");
#endif

	for (i =0 ; (uint32_t)i< key->cycles ; ++i)
	{
		shifts[0]=counter_df[0] ^ key->cadd[i][0]; // for 4*64bit (4*6=24) and for 8*32bit (8*5=40) (24+40=64)
		shifts[1]=counter_df[1] ^ key->cadd[i][1]; // for 16*16 (16*4=64)
		shifts[2]=counter_df[2] ^ key->cadd[i][2]; // for 4*64bit (4*6=24) and for 8*32bit (8*5=40) (24+40=64)
		shifts[3]=counter_df[3] ^ key->cadd[i][3]; // for 16*16 (16*4=64)

		free_rotate16(enc,shifts[1]);
		free_rotate64(enc,shifts[2]);
		SPECK_R(enc64[0],enc64[1],key->cadd[i][0]);
		SPECK_R(enc64[2],enc64[3],key->cadd[i][1]);
		free_rotate32(enc,shifts[2]>>24);
		free_rotate64(enc,shifts[0]);
		SPECK_R(enc64[2],enc64[1],key->cadd[i][3]);
		SPECK_R(enc64[0],enc64[3],key->cadd[i][2]);
		free_rotate16(enc,shifts[3]);
		free_rotate32(enc,shifts[0]>>24);
		zpn_imat(enc8,(uint8_t*)temp); // temp = imat*enc;
		//zpn_imat(temp,enc8); // temp = imat*enc;
		*(v4qw*)enc64=(*(v4qw*)temp)+*(v4qw*)key->cadd[i];
		
	}


#undef enc64
#undef enc32
#undef enc16
	
#ifdef DEBUG
	printf("encrypt: output\n");
	for (j=0;j<32;++j)
		printf("%02x ",((uint8_t*)enc8)[j]);
	printf("\n");
#endif
}

void zpn_decrypt(uint64_t nounce, uint64_t counter, struct zpn_key *key, uint8_t *raw, uint8_t *enc)
{
	uint64_t temp[4];
	uint64_t shifts[4];
	uint64_t counter_df[4];
#ifdef DEBUG
	int32_t j;
#endif	
#ifdef DEBUG
	printf("zpn_decrypt: nounce: %016lx counter: %016lx enc:\n",nounce,counter);
	for (j=0;j<32;++j)
		printf("%02x ",enc[j]);
	printf("\n");
#endif
	counter_df[0]=counter + key->counter_mask[0];
	counter_df[1]=key->counter_mask[1];
	counter_df[2]=nounce + key->counter_mask[2];
	counter_df[3]=key->counter_mask[3];
	zpn_imat((uint8_t*)counter_df,(uint8_t*)temp);
	ZPN_DIFFUSE256(temp[0], temp[1], temp[2], temp[3],
	0xa5a5a5a578787878,0x2d2d2d2d1e1e1e1e,0x969696964b4b4b4b,0x3c3c3c3cf0f0f0f0);
	zpn_imat((uint8_t*)temp,(uint8_t*)counter_df);
#ifdef DEBUG
	printf("decrypt: counter after mix\n");
	for (j=0;j<32;++j)
		printf("%02x ",((uint8_t*)counter_df)[j]);
	printf("\n");
#endif

#define raw64 ((uint64_t*)raw)
#define raw32 ((uint32_t*)raw)
#define raw16 ((uint16_t*)raw)
#define raw8 ((uint8_t*)raw)

	int32_t i;
#define t64 temp
	memcpy(raw,enc,4*8); //256bit
	for (i = key->cycles - 1 ; i >= 0 ; --i)
	{
		shifts[0]=counter_df[0] ^ key->cadd[i][0]; // for 4*64bit (4*6=24) and for 8*32bit (8*5=40) (24+40=64)
		shifts[1]=counter_df[1] ^ key->cadd[i][1]; // for 16*16 (16*4=64)
		shifts[2]=counter_df[2] ^ key->cadd[i][2]; // for 4*64bit (4*6=24) and for 8*32bit (8*5=40) (24+40=64)
		shifts[3]=counter_df[3] ^ key->cadd[i][3]; // for 16*16 (16*4=64)
		*(v4qw*)temp = (*(v4qw*)raw64) - *(v4qw*)key->cadd[i];
		//zpn_mat(raw8,temp8);
		zpn_mat(temp8,raw8);
		rev_free_rotate32(raw,shifts[0]>>24);
		rev_free_rotate16(raw,shifts[3]);
		REV_SPECK_R(raw64[0],raw64[3],key->cadd[i][2]);
		REV_SPECK_R(raw64[2],raw64[1],key->cadd[i][3]);
		rev_free_rotate64(raw,shifts[0]);
		rev_free_rotate32(raw,shifts[2]>>24);
		REV_SPECK_R(raw64[2],raw64[3],key->cadd[i][1]);
		REV_SPECK_R(raw64[0],raw64[1],key->cadd[i][0]);
		rev_free_rotate64(raw,shifts[2]);
		rev_free_rotate16(raw,shifts[1]);
		

	}
	
	*((v4qw*)raw64)=(*((v4qw*)raw64))-(*((v4qw*)counter_df));
#ifdef DEBUG
printf ("decrypt: output\n");
for (j=0;j<32;++j)
	printf("%02x ",((uint8_t*)raw8)[j]);
printf("\n");
#endif
#undef raw64
#undef raw32
#undef raw16
}

