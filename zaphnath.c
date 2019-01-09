
#include <malloc.h>
#include <string.h>
#include "zaphnath.h"
#define ROTL32(Y,B) ((Y<<(B))|(Y>>(32-(B))))
#define ROTL64(Y,B) ((Y<<(B))|(Y>>(64-(B))))
#define ROTL128(Y,B)\
{\
uint64_t temp;\
if(B < 64){temp=Y[0];Y[0]=(Y[0]<<(B))|(Y[1]>>(64-B));Y[1]=(Y[1]<<(B))|(temp>>(64-B));}\
else{temp=Y[0];Y[0]=(Y[1]<<(B-64))|(Y[0]>>(128-B));Y[1]=(temp<<(B-64))|(Y[1]>>(128-B));}\
}
#define CHA_R(A,B,C,R)\
A+=B; C^=A; C=ROTL32(C,R)
#define REV_CHA_R(A,B,C,R)\
C=ROTL32(C,32-R);C^=A; A-=B;
#define CHACHA(A,B,C,D)\
CHA_R(A,B,D,16);\
CHA_R(C,D,B,12);\
CHA_R(A,B,D,8);\
CHA_R(C,D,B,7);
#define REV_CHACHA(A,B,C,D)\
REV_CHA_R(C,D,B,7);\
REV_CHA_R(A,B,D,8);\
REV_CHA_R(C,D,B,12);\
REV_CHA_R(A,B,D,16);
#define ZAF_DIFFUSE_ROUND(A, B, C, R, T)\
A+=B; C^=A; C=ROTL32(C,R)+T;C-=ROTL32(B,5)

#define REV_ZAF_DIFFUSE_ROUND(A, B, C, R, T)\
C+=ROTL32(B,5);C=ROTL32((C-T),32-R);C^=A; A-=B

#define ZAF_DIFFUSE(A,B,C,D) \
ZAF_DIFFUSE_ROUND(D,A,B,15,0x5A5A5A5A);\
ZAF_DIFFUSE_ROUND(C,B,A,11,0x78787878);\
ZAF_DIFFUSE_ROUND(C,D,B,6,0x69696969);\
ZAF_DIFFUSE_ROUND(A,B,D,4,0x1E1E1E1E);

#define REV_ZAF_DIFFUSE(A,B,C,D) \
REV_ZAF_DIFFUSE_ROUND(A,B,D,4,0x1E1E1E1E);\
REV_ZAF_DIFFUSE_ROUND(C,D,B,6,0x69696969);\
REV_ZAF_DIFFUSE_ROUND(C,B,A,11,0x78787878);\
REV_ZAF_DIFFUSE_ROUND(D,A,B,15,0x5A5A5A5A);


#define bjenkins_mix2(a,b,c) \
{ \
  a -= b; a -= c; a ^= (c>>13); \
  b -= c; b -= a; b ^= (a<<8); \
  c -= a; c -= b; c ^= (b>>13); \
  a -= b; a -= c; a ^= (c>>12);  \
  b -= c; b -= a; b ^= (a<<16); \
  c -= a; c -= b; c ^= (b>>5); \
  a -= b; a -= c; a ^= (c>>3);  \
  b -= c; b -= a; b ^= (a<<10); \
  c -= a; c -= b; c ^= (b>>15); \
}

#define bjenkins_mix3(a,b,c) \
{ \
  a -= c;  a ^= ROTL32(c, 4);  c += b; \
  b -= a;  b ^= ROTL32(a, 6);  a += c; \
  c -= b;  c ^= ROTL32(b, 8);  b += a; \
  a -= c;  a ^= ROTL32(c,16);  c += b; \
  b -= a;  b ^= ROTL32(a,19);  a += c; \
  c -= b;  c ^= ROTL32(b, 4);  b += a; \
}


extern uint8_t zpn_lookups[1024][256];
extern uint8_t rev_zpn_lookups[1024][256];

uint32_t max_ui32(uint32_t a, uint32_t b)
{ if (a>b) return a; return b; }
void zpn_feed_sponge(uint8_t* sponge, uint32_t length, uint32_t feed)
{
	((uint32_t*)sponge)[0]^= feed;
	int i,j,k;
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
int zpn_expand_key(uint8_t *key, uint32_t length, uint32_t cycles, struct time_schedule * ts)
{
	uint32_t i;
	if (cycles> MAX_CYCLES)
		cycles= MAX_CYCLES;
	ts->cycles=cycles;
	if (key)
	{
		uint32_t sponge_len =
		 (max_ui32(2*length, 64)+7)&(~0x00000007);
		uint8_t * sponge = (uint8_t*)malloc(sponge_len);
	
		if (sponge == 0)
			return -1; // allocation error
		memset(sponge, 0, sponge_len);

		for (i=0; i < length; ++i)
		{
			zpn_feed_sponge(sponge,sponge_len,key[i]);
		}
		ts->counter_mask[0] = ((uint64_t*)sponge)[0];
		zpn_feed_sponge(sponge,sponge_len,0);
		ts->counter_mask[1] = ((uint64_t*)sponge)[0];
		zpn_feed_sponge(sponge,sponge_len,0);
		ts->final_xor[0] = ((uint64_t*)sponge)[0];
		zpn_feed_sponge(sponge,sponge_len,0);
		ts->final_xor[1] = ((uint64_t*)sponge)[0];
		int sponge_counter=6;
		uint64_t sponge_key=0;
		for (i=0;i<cycles*16;++i)
		{
			if (sponge_counter==6)
			{
				zpn_feed_sponge(sponge,sponge_len,0);
				sponge_key ^= ((uint64_t*)sponge)[0];
				sponge_counter=0;
			}
			ts->index[i/16][i%16]=sponge_key&0x3ff;
			sponge_key >>= 10;
			++sponge_counter;
		}
		free(sponge);
	}
	else
	{
		ts->counter_mask[0] = 0x12345faa0987efab;
		ts->counter_mask[1] = 0xabcd4567f123a5a5;
		ts->final_xor[0] = 0x88a456246ef87d35;
		ts->final_xor[1] = 0x762dfe7a0bc41768;

		for (i=0; i < cycles*16; ++i)
		{
			ts->index[i/16][i%16]=i;
		}
	}
	return 0;
}

//todo: add byte rotation
/*
0123
4567
89ab
cdef

will be
8c4a
06f1
359e
2b7d
*/
/* multiplication pairs:
17:241
25:41
5:205
9:57
27:19
3:171
145:113
*/
void rotate_bytes(uint8_t * x)
{
	uint8_t t;
	t=x[0];
	x[0]=x[8]*5;
	x[8]=x[3]*205;
	x[3]=x[10]*241;
	x[10]=x[9]*41;
	x[9]=x[5]*145;
	x[5]=x[6]*113;
	x[6]=x[15]*3;
	x[15]=x[13]*171;
	x[13]=x[11]*27;
	x[11]=x[14]*19;
	x[14]=x[7]*9;
	x[7]=x[1]*57;
	x[1]=x[12]*25;
	x[12]=x[2]*17;
	x[2]=x[4]*241;
	x[4]=t;
}
void rev_rotate_bytes(uint8_t * x)
{
	uint8_t t;
	t=x[4];
	x[4]=x[2]*17;
	x[2]=x[12]*241;
	x[12]=x[1]*41;
	x[1]=x[7]*9;
	x[7]=x[14]*57;
	x[14]=x[11]*27;
	x[11]=x[13]*19;
	x[13]=x[15]*3;
	x[15]=x[6]*171;
	x[6]=x[5]*145;
	x[5]=x[9]*113;
	x[9]=x[10]*25;
	x[10]=x[3]*17;
	x[3]=x[8]*5;
	x[8]=x[0]*205;
	x[0]=t;
}
void zpn_encrypt(uint64_t nounce, uint64_t counter, struct time_schedule *ts, uint8_t *data, uint8_t *out)
{
	uint32_t counter_df[4];
	counter ^= ts->counter_mask[0];
	nounce ^= ts->counter_mask[1];
	counter_df[0]=counter & 0xffffffff;
	counter_df[1]=counter>>32;
	counter_df[2]=nounce &0xffffffff;
	counter_df[3]=nounce>>32;

	ZAF_DIFFUSE(counter_df[0], counter_df[1], counter_df[2], counter_df[3]);
	uint8_t * ct=(uint8_t*)counter_df;
	uint32_t i, b;
	uint32_t *out32=(uint32_t *)out;
	b=0;
	out[b] = zpn_lookups[(ts->index[0][b]+ct[b])&0x3ff][data[b]];++b;
	out[b] = zpn_lookups[(ts->index[0][b]+ct[b])&0x3ff][data[b]];++b;
	out[b] = zpn_lookups[(ts->index[0][b]+ct[b])&0x3ff][data[b]];++b;
	out[b] = zpn_lookups[(ts->index[0][b]+ct[b])&0x3ff][data[b]];++b;
	out[b] = zpn_lookups[(ts->index[0][b]+ct[b])&0x3ff][data[b]];++b;
	out[b] = zpn_lookups[(ts->index[0][b]+ct[b])&0x3ff][data[b]];++b;
	out[b] = zpn_lookups[(ts->index[0][b]+ct[b])&0x3ff][data[b]];++b;
	out[b] = zpn_lookups[(ts->index[0][b]+ct[b])&0x3ff][data[b]];++b;
	out[b] = zpn_lookups[(ts->index[0][b]+ct[b])&0x3ff][data[b]];++b;
	out[b] = zpn_lookups[(ts->index[0][b]+ct[b])&0x3ff][data[b]];++b;
	out[b] = zpn_lookups[(ts->index[0][b]+ct[b])&0x3ff][data[b]];++b;
	out[b] = zpn_lookups[(ts->index[0][b]+ct[b])&0x3ff][data[b]];++b;
	out[b] = zpn_lookups[(ts->index[0][b]+ct[b])&0x3ff][data[b]];++b;
	out[b] = zpn_lookups[(ts->index[0][b]+ct[b])&0x3ff][data[b]];++b;
	out[b] = zpn_lookups[(ts->index[0][b]+ct[b])&0x3ff][data[b]];++b;
	out[b] = zpn_lookups[(ts->index[0][b]+ct[b])&0x3ff][data[b]];

	for (i =1 ; i< ts->cycles ; ++i)
	{
		ZAF_DIFFUSE(out32[0], out32[1], out32[2], out32[3]);
		rotate_bytes(out);
		ROTL128(((uint64_t*)out),5)
		b=0;
		out[b] = zpn_lookups[(ts->index[i][b])&0x3ff][out[b]];++b;
		out[b] = zpn_lookups[(ts->index[i][b])&0x3ff][out[b]];++b;
		out[b] = zpn_lookups[(ts->index[i][b])&0x3ff][out[b]];++b;
		out[b] = zpn_lookups[(ts->index[i][b])&0x3ff][out[b]];++b;
		out[b] = zpn_lookups[(ts->index[i][b])&0x3ff][out[b]];++b;
		out[b] = zpn_lookups[(ts->index[i][b])&0x3ff][out[b]];++b;
		out[b] = zpn_lookups[(ts->index[i][b])&0x3ff][out[b]];++b;
		out[b] = zpn_lookups[(ts->index[i][b])&0x3ff][out[b]];++b;
		out[b] = zpn_lookups[(ts->index[i][b])&0x3ff][out[b]];++b;
		out[b] = zpn_lookups[(ts->index[i][b])&0x3ff][out[b]];++b;
		out[b] = zpn_lookups[(ts->index[i][b])&0x3ff][out[b]];++b;
		out[b] = zpn_lookups[(ts->index[i][b])&0x3ff][out[b]];++b;
		out[b] = zpn_lookups[(ts->index[i][b])&0x3ff][out[b]];++b;
		out[b] = zpn_lookups[(ts->index[i][b])&0x3ff][out[b]];++b;
		out[b] = zpn_lookups[(ts->index[i][b])&0x3ff][out[b]];++b;
		out[b] = zpn_lookups[(ts->index[i][b])&0x3ff][out[b]];
	}
	ZAF_DIFFUSE(out32[0], out32[1], out32[2], out32[3]);
	((uint64_t*)out)[0]^=ts->final_xor[0];
	((uint64_t*)out)[1]^=ts->final_xor[1];
}

void zpn_decrypt(uint64_t nounce, uint64_t counter, struct time_schedule *ts, uint8_t *data, uint8_t *out)
{
	uint32_t *out32=(uint32_t *)out;
	((uint64_t*)out)[0]^=ts->final_xor[0];
	((uint64_t*)out)[1]^=ts->final_xor[1];
	REV_ZAF_DIFFUSE(out32[0], out32[1], out32[2], out32[3]);

	uint32_t i, b;

	for (i = ts->cycles - 1 ; i>=1 ; --i)
	{
		b=0;
		out[b] = rev_zpn_lookups[(ts->index[i][b])&0x3ff][out[b]];++b;
		out[b] = rev_zpn_lookups[(ts->index[i][b])&0x3ff][out[b]];++b;
		out[b] = rev_zpn_lookups[(ts->index[i][b])&0x3ff][out[b]];++b;
		out[b] = rev_zpn_lookups[(ts->index[i][b])&0x3ff][out[b]];++b;
		out[b] = rev_zpn_lookups[(ts->index[i][b])&0x3ff][out[b]];++b;
		out[b] = rev_zpn_lookups[(ts->index[i][b])&0x3ff][out[b]];++b;
		out[b] = rev_zpn_lookups[(ts->index[i][b])&0x3ff][out[b]];++b;
		out[b] = rev_zpn_lookups[(ts->index[i][b])&0x3ff][out[b]];++b;
		out[b] = rev_zpn_lookups[(ts->index[i][b])&0x3ff][out[b]];++b;
		out[b] = rev_zpn_lookups[(ts->index[i][b])&0x3ff][out[b]];++b;
		out[b] = rev_zpn_lookups[(ts->index[i][b])&0x3ff][out[b]];++b;
		out[b] = rev_zpn_lookups[(ts->index[i][b])&0x3ff][out[b]];++b;
		out[b] = rev_zpn_lookups[(ts->index[i][b])&0x3ff][out[b]];++b;
		out[b] = rev_zpn_lookups[(ts->index[i][b])&0x3ff][out[b]];++b;
		out[b] = rev_zpn_lookups[(ts->index[i][b])&0x3ff][out[b]];++b;
		out[b] = rev_zpn_lookups[(ts->index[i][b])&0x3ff][out[b]];
		ROTL128(((uint64_t*)out),123)
		rev_rotate_bytes(out);
		REV_ZAF_DIFFUSE(out32[0], out32[1], out32[2], out32[3]);
	}
	
	uint32_t counter_df[4];
	counter ^= ts->counter_mask[0];
	nounce ^= ts->counter_mask[1];
	counter_df[0]=counter & 0xffffffff;
	counter_df[1]=counter>>32;
	counter_df[2]=nounce &0xffffffff;
	counter_df[3]=nounce>>32;
	ZAF_DIFFUSE(counter_df[0], counter_df[1], counter_df[2], counter_df[3]);
	uint8_t * ct=(uint8_t*)counter_df;
	b=0;
	data[b] = rev_zpn_lookups[(ts->index[0][b]+ct[b])&0x3ff][out[b]];++b;
	data[b] = rev_zpn_lookups[(ts->index[0][b]+ct[b])&0x3ff][out[b]];++b;
	data[b] = rev_zpn_lookups[(ts->index[0][b]+ct[b])&0x3ff][out[b]];++b;
	data[b] = rev_zpn_lookups[(ts->index[0][b]+ct[b])&0x3ff][out[b]];++b;
	data[b] = rev_zpn_lookups[(ts->index[0][b]+ct[b])&0x3ff][out[b]];++b;
	data[b] = rev_zpn_lookups[(ts->index[0][b]+ct[b])&0x3ff][out[b]];++b;
	data[b] = rev_zpn_lookups[(ts->index[0][b]+ct[b])&0x3ff][out[b]];++b;
	data[b] = rev_zpn_lookups[(ts->index[0][b]+ct[b])&0x3ff][out[b]];++b;
	data[b] = rev_zpn_lookups[(ts->index[0][b]+ct[b])&0x3ff][out[b]];++b;
	data[b] = rev_zpn_lookups[(ts->index[0][b]+ct[b])&0x3ff][out[b]];++b;
	data[b] = rev_zpn_lookups[(ts->index[0][b]+ct[b])&0x3ff][out[b]];++b;
	data[b] = rev_zpn_lookups[(ts->index[0][b]+ct[b])&0x3ff][out[b]];++b;
	data[b] = rev_zpn_lookups[(ts->index[0][b]+ct[b])&0x3ff][out[b]];++b;
	data[b] = rev_zpn_lookups[(ts->index[0][b]+ct[b])&0x3ff][out[b]];++b;
	data[b] = rev_zpn_lookups[(ts->index[0][b]+ct[b])&0x3ff][out[b]];++b;
	data[b] = rev_zpn_lookups[(ts->index[0][b]+ct[b])&0x3ff][out[b]];
}


