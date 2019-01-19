
#include <malloc.h>
#include <string.h>
#include "zaphnath.h"

//clang queries
#ifndef __has_feature         // Optional of course.
  #define __has_feature(x) 0  // Compatibility with non-clang compilers.
#endif
#ifndef __has_extension
  #define __has_extension __has_feature // Compatibility with pre-3.0 compilers.
#endif
#ifndef __has_builtin    
#define __has_builtin(x) 0  // Compatibility with non-clang compilers. 
#endif 


typedef uint8_t v32b __attribute__((__vector_size__(32)));
typedef uint16_t v16w __attribute__((__vector_size__(32)));
typedef uint32_t v8dw __attribute__((__vector_size__(32)));
typedef uint64_t v4qw __attribute__((__vector_size__(32)));

#if __has_builtin(__builtin_rotateleft16)
#define ROTL16(Y,B) (__builtin_rotateleft16(Y,B))
#else
#define ROTL16(Y,B) ((Y<<(B))|(Y>>(16-(B))))
#endif

#if __has_builtin(__builtin_rotateleft32)
#define ROTL32(Y,B) (__builtin_rotateleft32(Y,B))
#else
#define ROTL32(Y,B) ((Y<<(B))|(Y>>(32-(B))))
#endif

#if __has_builtin(__builtin_rotateleft64)
#define ROTL64(Y,B) (__builtin_rotateleft64(Y,B))
#else
#define ROTL64(Y,B) ((Y<<(B))|(Y>>(64-(B))))
#endif

#define ROTL128(Y,B)\
{\
uint64_t temp;\
if(B < 64){temp=Y[0];Y[0]=(Y[0]<<(B))|(Y[1]>>(64-B));Y[1]=(Y[1]<<(B))|(temp>>(64-B));}\
else{temp=Y[0];Y[0]=(Y[1]<<(B-64))|(Y[0]>>(128-B));Y[1]=(temp<<(B-64))|(Y[1]>>(128-B));}\
}

#define ZPN_DIFFUSE_ROUND(A, B, C, D, R, T)\
A+=ROTL64(B,5)-B; C^=D+T; B=ROTL64(B,8); D+=(A);A=ROTL64(A,R);

#define REV_ZPN_DIFFUSE_ROUND(A, B, C, D, R, T)\
A=ROTL64(A,64-R);D-=(A);B=ROTL64(B,56);C^=D+T;A-=ROTL64(B,5)-B;

#define ZPN_DIFFUSE256(A,B,C,D,I,J,K,L) \
ZPN_DIFFUSE_ROUND(A,B,C,D,18,I)\
ZPN_DIFFUSE_ROUND(B,C,D,A,1,J)\
ZPN_DIFFUSE_ROUND(C,D,A,B,35,K)\
ZPN_DIFFUSE_ROUND(D,A,B,C,52,L)

#define REV_ZPN_DIFFUSE256(A,B,C,D,I,J,K,L) \
REV_ZPN_DIFFUSE_ROUND(D,A,B,C,52,L);\
REV_ZPN_DIFFUSE_ROUND(C,D,A,B,35,K);\
REV_ZPN_DIFFUSE_ROUND(B,C,D,A,1,J);\
REV_ZPN_DIFFUSE_ROUND(A,B,C,D,18,I);

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

static inline void zpn_mat(uint64_t *in, uint64_t *out)
{
/*
-1 -1  2  1
 2 -2 -1  2
 1  2  1 -1
 0  1 -1 -1
*/
	out[0]=-in[0]-in[1]+(in[2]<<1)+in[3];
	out[1]=((in[0]-in[1]+in[3])<<1)-in[2];
	out[2]=in[0]+(in[1]<<1)+in[2]-in[3];
	out[3]=in[1]-in[2]-in[3];
}
static inline void zpn_imat(uint64_t *in, uint64_t *out)
{
/*
-3 -1  0 -5
 7  3  1 12
-2 -1  0 -4
 9  4  1 15
*/
        out[0]=-in[0]-(in[0]<<1)-in[1]-in[3]-(in[3]<<2);
        out[1]=-in[0]+((in[0]+in[3])<<3)-in[1]+((in[1]+in[3])<<2)+in[2];
        out[2]=-(in[0]<<1)-in[1]-(in[3]<<2);
        out[3]=in[0]+(in[0]<<3)+(in[1]<<2)+in[2]+(in[3]<<4)-in[3];
}

static inline void zpn_mixbits(uint64_t* in,uint64_t *out, uint64_t b1, uint64_t b2)
{
	uint64_t m[4]={b1&b2,b1&(~b2),b2&(~b1),~b1&~b2};
	out[0]=(m[0]&in[0])|(m[1]&in[1])|(m[2]&in[2])|(m[3]&in[3]);
	out[1]=(m[0]&in[1])|(m[1]&in[0])|(m[2]&in[3])|(m[3]&in[2]);
	out[2]=(m[0]&in[2])|(m[1]&in[3])|(m[2]&in[0])|(m[3]&in[1]);
	out[3]=(m[0]&in[3])|(m[1]&in[2])|(m[2]&in[1])|(m[3]&in[0]);
}// this function is the inverse of itself

#define BYTE_SHUFFLE 21, 29, 10, 4, 20, 30, 2, 12, 14, 1, 27, 8, 16, 31, 7, 18, 24, 5, 25, 0, 23, 17, 9, 11, 22, 26, 19, 3, 6, 28, 15, 13
#define INV_BYTE_SHUFFLE 19, 9, 6, 27, 3, 17, 28, 14, 11, 22, 2, 23, 7, 31, 8, 30, 12, 21, 15, 26, 4, 0, 24, 20, 16, 18, 25, 10, 29, 1, 5, 13
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
		zpn_feed_sponge(sponge,sponge_len,0);
		exp_key->final_add[0] = ((uint64_t*)sponge)[0];
		zpn_feed_sponge(sponge,sponge_len,0);
		exp_key->final_add[1] = ((uint64_t*)sponge)[0];
		zpn_feed_sponge(sponge,sponge_len,0);
		exp_key->final_add[2] = ((uint64_t*)sponge)[0];
		zpn_feed_sponge(sponge,sponge_len,0);
		exp_key->final_add[3] = ((uint64_t*)sponge)[0];
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
			zpn_feed_sponge(sponge,sponge_len,0);
			exp_key->cshifts_mixbits[i][0]=((uint64_t*)sponge)[0];
			zpn_feed_sponge(sponge,sponge_len,0);
			exp_key->cshifts_mixbits[i][1]=((uint64_t*)sponge)[0];
			zpn_feed_sponge(sponge,sponge_len,0);
			exp_key->cshifts_mixbits[i][2]=((uint64_t*)sponge)[0];
			zpn_feed_sponge(sponge,sponge_len,0);
			exp_key->cshifts_mixbits[i][3]=((uint64_t*)sponge)[0];
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
#define DEBUG
//#define DEBUG2
//#define DEBUG3
void zpn_encrypt(uint64_t nounce, uint64_t counter, struct zpn_key *key, uint8_t *raw, uint8_t *enc)
{
	uint64_t counter_df[4];
	uint64_t temp[4];
	uint64_t shifts[2];
	uint64_t mixbits[2];
	int32_t i, j, b, step;
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
	zpn_imat(counter_df,temp);
	ZPN_DIFFUSE256(temp[0], temp[1], temp[2], temp[3],
	0xa5a5a5a578787878,0x2d2d2d2d1e1e1e1e,0x969696964b4b4b4b,0x3c3c3c3cf0f0f0f0);
	zpn_imat(temp,counter_df);
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
/*
	enc64[0]=((uint64_t*)raw)[0]+counter_df[0];
	enc64[1]=((uint64_t*)raw)[1]+counter_df[1];
	enc64[2]=((uint64_t*)raw)[2]+counter_df[2];
	enc64[3]=((uint64_t*)raw)[3]+counter_df[3];
*/
	for (i =0 ; i< key->cycles ; ++i)
	{
		for (step=0;step<4;step+=2)
		{
#ifdef DEBUG
		printf("beginning of cycle %d, step %d \n",i,step);
		for (j=0;j<32;++j)
			printf("%02x ",enc8[j]);
		printf("\n");
#endif
		shifts[0]=counter_df[0] ^ key->cshifts_mixbits[i][step+0]; // for 4*64bit (4*6=24) and for 8*32bit (8*5=40) (24+40=64)
		shifts[1]=counter_df[1] ^ key->cshifts_mixbits[i][step+1]; // for 16*16 (16*4=64)
		
		mixbits[0]=counter_df[2] ^ key->cshifts_mixbits[i][(step+2)&3];
		mixbits[1]=counter_df[3] ^ key->cshifts_mixbits[i][(step+3)&3];
		zpn_imat(enc64,temp); // temp = imat*enc;
		
#ifdef DEBUG3
		printf("rotating 4x64: %d, %d, %d, %d\n",(int)(shifts[0]&0x3f)
				,(int)((shifts[0]>>6)&0x3f),(int)((shifts[0]>>12)&0x3f)
				,(int)((shifts[0]>>18)&0x3f));
#endif
#define CUR_W temp64
		CUR_W[0]=ROTL64(CUR_W[0],shifts[0]&0x3f);shifts[0]>>=6;
		CUR_W[1]=ROTL64(CUR_W[1],shifts[0]&0x3f);shifts[0]>>=6;
		CUR_W[2]=ROTL64(CUR_W[2],shifts[0]&0x3f);shifts[0]>>=6;
		CUR_W[3]=ROTL64(CUR_W[3],shifts[0]&0x3f);shifts[0]>>=6;
		zpn_imat(temp,enc64); // temp = imat*enc;
		for (b =0 ; b < 16 ; b+=4)
		{
			enc16[b+0]=ROTL16(enc16[b+0],shifts[1]&0xf);shifts[1]>>=4;
			enc16[b+1]=ROTL16(enc16[b+1],shifts[1]&0xf);shifts[1]>>=4;
			enc16[b+2]=ROTL16(enc16[b+2],shifts[1]&0xf);shifts[1]>>=4;
			enc16[b+3]=ROTL16(enc16[b+3],shifts[1]&0xf);shifts[1]>>=4;
		}
#ifdef DEBUG3
		printf ("encrypt: before mat mul\n");
		for (j=0;j<32;++j)
			printf("%02x ",((uint8_t*)enc8)[j]);
		printf("\n");
#endif
		zpn_imat(enc64,temp); // temp = imat*enc;
		#ifdef DEBUG3
		printf ("encrypt: before temp addition\n");
		for (j=0;j<32;++j)
			printf("%02x ",((uint8_t*)temp)[j]);
		printf("\n");
		#endif
		
		temp[0]+=0xa5a5a5a578787878; // propagate bits by addition.
		temp[1]+=0x2d2d2d2d1e1e1e1e;
		temp[2]+=0x969696964b4b4b4b;
		temp[3]+=0x3c3c3c3cf0f0f0f0;

#ifdef DEBUG3
	printf ("encrypt: before shuffle\n");
	for (j=0;j<32;++j)
		printf("%02x ",((uint8_t*)temp)[j]);
	printf("\n");
#endif
#if __has_builtin(__builtin_shufflevector)
		*((v32b*)enc)=__builtin_shufflevector(*(v32b*)(temp),*(v32b*)(temp), BYTE_SHUFFLE);
#else
		*((v32b*)enc)=__builtin_shuffle (*(v32b*)(temp), (v32b){BYTE_SHUFFLE});
#endif
#ifdef DEBUG3
	printf ("encrypt: after shuffle\n");
	for (j=0;j<32;++j)
		printf("%02x ",enc8[j]);
	printf("\n");
#endif
		for (b =0 ; b < 8 ; b+=4)
		{
			enc32[b+0]=ROTL32(enc32[b+0],shifts[0]&0x1f);shifts[0]>>=5;
			enc32[b+1]=ROTL32(enc32[b+1],shifts[0]&0x1f);shifts[0]>>=5;
			enc32[b+2]=ROTL32(enc32[b+2],shifts[0]&0x1f);shifts[0]>>=5;
			enc32[b+3]=ROTL32(enc32[b+3],shifts[0]&0x1f);shifts[0]>>=5;
		}
#ifdef DEBUG4
	printf ("encrypt: after 8x32 rotation\n");
	for (j=0;j<32;++j)
		printf("%02x ",enc8[j]);
	printf("\n");
#endif
		zpn_mixbits(enc64,temp,mixbits[0],mixbits[1]);
#ifdef DEBUG5
	printf ("encrypt: after mixbit\n");
	for (j=0;j<32;++j)
		printf("%02x ",((uint8_t*)temp)[j]);
	printf("\n");
#endif
		zpn_imat(temp,enc64);
		*(v4qw*)enc64=(*(v4qw*)enc64)+*(v4qw*)key->cadd[i];
#ifdef DEBUG6
	printf ("encrypt: cycle final addition\n");
	for (j=0;j<32;++j)
		printf("%02x ",enc8[j]);
	printf("\n");
#endif
		}
	}
#ifdef DEBUG2
	for (j=0;j<32;++j)
		printf("%02x ",enc8[j]);
	printf("\n");
#endif
	zpn_imat(enc64,temp);
#ifdef DEBUG2
	printf ("before final addition:\n");
	for (j=0;j<32;++j)
		printf("%02x ",((uint8_t*)temp)[j]);
	printf("\n");
	printf ("added with\n");
	for (j=0;j<32;++j)
		printf("%02x ",((uint8_t*)key->final_add)[j]);
	printf("\n");
#endif
	*(v4qw*)enc64 = (*(v4qw*)temp) + (*(v4qw*)key->final_add);
#ifdef DEBUG
	printf ("output from encrypt:\n");
	for (j=0;j<32;++j)
		printf("%02x ",enc8[j]);
	printf("\n");
#endif
#undef enc64
#undef enc32
#undef enc16
}

void zpn_decrypt(uint64_t nounce, uint64_t counter, struct zpn_key *key, uint8_t *raw, uint8_t *enc)
{
	uint64_t temp[4];
	uint64_t shifts[2];
	uint64_t shifts32;
	uint64_t mixbits[2];
	uint64_t counter_df[4];
	counter_df[0]=counter + key->counter_mask[0];
	counter_df[1]=key->counter_mask[1];
	counter_df[2]=nounce + key->counter_mask[2];
	counter_df[3]=key->counter_mask[3];
	zpn_imat(counter_df,temp);
	ZPN_DIFFUSE256(temp[0], temp[1], temp[2], temp[3],
	0xa5a5a5a578787878,0x2d2d2d2d1e1e1e1e,0x969696964b4b4b4b,0x3c3c3c3cf0f0f0f0);
	zpn_imat(temp,counter_df);
	

#define raw64 ((uint64_t*)raw)
#define raw32 ((uint32_t*)raw)
#define raw16 ((uint16_t*)raw)
#define raw8 ((uint8_t*)raw)
	*(v4qw*)temp = (*(v4qw*)enc) - (*(v4qw*)key->final_add);

	int32_t i, j, b, step;
	zpn_mat(temp, raw64);
#define t64 temp
	for (i = key->cycles - 1 ; i >= 0 ; --i)
	{
		for (step=2;step>=0;step-=2)
		{
		shifts[0]=counter_df[0] ^ key->cshifts_mixbits[i][step+0]; // for 4*64bit (4*6=24) and for 8*32bit (8*5=40) (24+40=64)
		shifts[1]=counter_df[1] ^ key->cshifts_mixbits[i][step+1]; // for 16*16 (16*4=64)
		mixbits[0]=counter_df[2] ^ key->cshifts_mixbits[i][(step+2)&3];
		mixbits[1]=counter_df[3] ^ key->cshifts_mixbits[i][(step+3)&3];
		shifts32 = shifts[0] >> 24;
		
		*(v4qw*)raw64=(*(v4qw*)raw64)-*(v4qw*)key->cadd[i];
		zpn_mat(raw64,temp);
		zpn_mixbits(temp,raw64,mixbits[0],mixbits[1]);
		for (b =0 ; b < 8 ; b+=4)
		{
			raw32[b+0]=ROTL32(raw32[b+0],(-shifts32)&0x1f);shifts32>>=5;
			raw32[b+1]=ROTL32(raw32[b+1],(-shifts32)&0x1f);shifts32>>=5;
			raw32[b+2]=ROTL32(raw32[b+2],(-shifts32)&0x1f);shifts32>>=5;
			raw32[b+3]=ROTL32(raw32[b+3],(-shifts32)&0x1f);shifts32>>=5;
		}
#ifdef DEBUG
	printf ("decrypt: before shuffle\n");
	for (j=0;j<32;++j)
		printf("%02x ",raw8[j]);
	printf("\n");
#endif
#if __has_builtin(__builtin_shufflevector)
		*((v32b*)temp)=__builtin_shufflevector(*(v32b*)(raw),*(v32b*)(raw), INV_BYTE_SHUFFLE);
#else
		*((v32b*)temp)=__builtin_shuffle (*(v32b*)(raw), (v32b){INV_BYTE_SHUFFLE});
#endif
#ifdef DEBUG
	printf ("decrypt: after shuffle\n");
	for (j=0;j<32;++j)
		printf("%02x ",((uint8_t*)temp)[j]);
	printf("\n");
#endif
		temp[0]-=0xa5a5a5a578787878;
		temp[1]-=0x2d2d2d2d1e1e1e1e;
		temp[2]-=0x969696964b4b4b4b;
		temp[3]-=0x3c3c3c3cf0f0f0f0;
		#ifdef DEBUG
		printf ("decrypt: after temp addition\n");
		for (j=0;j<32;++j)
			printf("%02x ",((uint8_t*)temp)[j]);
		printf("\n");
		#endif
		zpn_mat(temp,raw64);
		#ifdef DEBUG
		printf ("decrypt: after mat mul\n");
		for (j=0;j<32;++j)
			printf("%02x ",((uint8_t*)raw8)[j]);
		printf("\n");
		#endif
		for (b =0 ; b < 16 ; b+=4)
		{
			raw16[b+0]=ROTL16(raw16[b+0],(-shifts[1])&0xf);shifts[1]>>=4;
			raw16[b+1]=ROTL16(raw16[b+1],(-shifts[1])&0xf);shifts[1]>>=4;
			raw16[b+2]=ROTL16(raw16[b+2],(-shifts[1])&0xf);shifts[1]>>=4;
			raw16[b+3]=ROTL16(raw16[b+3],(-shifts[1])&0xf);shifts[1]>>=4;
		}
		zpn_mat(raw64,temp64); // temp = imat*enc;
#define CUR_W temp64
		CUR_W[0]=ROTL64(CUR_W[0],(-shifts[0])&0x3f);shifts[0]>>=6;
		CUR_W[1]=ROTL64(CUR_W[1],(-shifts[0])&0x3f);shifts[0]>>=6;
		CUR_W[2]=ROTL64(CUR_W[2],(-shifts[0])&0x3f);shifts[0]>>=6;
		CUR_W[3]=ROTL64(CUR_W[3],(-shifts[0])&0x3f);shifts[0]>>=6;
		zpn_mat(temp,raw64); // temp = imat*enc;
		}
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


