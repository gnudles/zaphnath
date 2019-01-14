
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

typedef uint8_t v16b __attribute__((__vector_size__(16)));
typedef uint8_t v32b __attribute__((__vector_size__(32)));
#define ROTL32(Y,B) ((Y<<(B))|(Y>>(32-(B))))
#define ROTL64(Y,B) ((Y<<(B))|(Y>>(64-(B))))
#define ROTL128(Y,B)\
{\
uint64_t temp;\
if(B < 64){temp=Y[0];Y[0]=(Y[0]<<(B))|(Y[1]>>(64-B));Y[1]=(Y[1]<<(B))|(temp>>(64-B));}\
else{temp=Y[0];Y[0]=(Y[1]<<(B-64))|(Y[0]>>(128-B));Y[1]=(temp<<(B-64))|(Y[1]>>(128-B));}\
}
#define ZPN_DIFFUSE_ROUND(A, B, C, R, T)\
C^=A; C=ROTL64(C,R)+T; A+=B; C-=ROTL64(B,5)+ROTL64(B,46)+ROTL64(B,36)+ROTL64(B,61)

#define REV_ZPN_DIFFUSE_ROUND(A, B, C, R, T)\
C+=ROTL64(B,5)+ROTL64(B,46)+ROTL64(B,36)+ROTL64(B,61); A-=B; C=ROTL64((C-T),64-R); C^=A

#define ZPN_DIFFUSE256(A,B,C,D,I,J,K,L) \
ZPN_DIFFUSE_ROUND(D,A,B,20,I);\
ZPN_DIFFUSE_ROUND(C,B,A,11,J);\
ZPN_DIFFUSE_ROUND(B,D,C,30,K);\
ZPN_DIFFUSE_ROUND(A,B,D,51,L);

#define REV_ZPN_DIFFUSE256(A,B,C,D,I,J,K,L) \
REV_ZPN_DIFFUSE_ROUND(A,B,D,51,L);\
REV_ZPN_DIFFUSE_ROUND(B,D,C,30,K);\
REV_ZPN_DIFFUSE_ROUND(C,B,A,11,J);\
REV_ZPN_DIFFUSE_ROUND(D,A,B,20,I);

//This is for those who concern about multiplication
#define MUL18150808750145(V,TEMP)\
TEMP=(V)+((V)<<5);\
TEMP=(TEMP<<6)+(TEMP<<23)+(TEMP<<39);\
V=V+TEMP+(V<<17)+(V<<33)
#define MUL4065217(V,TEMP)\
TEMP=(V<<1)+(V<<2);\
TEMP+=(TEMP<<2)+V;\
TEMP+=(TEMP<<11);\
V=(TEMP<<6)+V

/* there are exactly 128 primitive roots of 257 */

uint8_t prim_root[128]={3 , 5 , 6 , 7 , 10 , 12 , 14 , 19 , 20 , 24 , 27 , 28 , 33 , 37 , 38 , 39 , 40 , 41 , 43 , 45 , 47 , 48 , 51 , 53 , 54 , 55 , 56 , 63 , 65 , 66 , 69 , 71 , 74 , 75 , 76 , 77 , 78 , 80 , 82 , 83 , 85 , 86 , 87 , 90 , 91 , 93 , 94 , 96 , 97 , 101 , 102 , 103 , 105 , 106 , 107 , 108 , 109 , 110 , 112 , 115 , 119 , 125 , 126 , 127 , 130 , 131 , 132 , 138 , 142 , 145 , 147 , 148 , 149 , 150 , 151 , 152 , 154 , 155 , 156 , 160 , 161 , 163 , 164 , 166 , 167 , 170 , 171 , 172 , 174 , 175 , 177 , 179 , 180 , 181 , 182 , 183 , 186 , 188 , 191 , 192 , 194 , 201 , 202 , 203 , 204 , 206 , 209 , 210 , 212 , 214 , 216 , 217 , 218 , 219 , 220 , 224 , 229 , 230 , 233 , 237 , 238 , 243 , 245 , 247 , 250 , 251 , 252 , 254 };
// you get the inverse by pow(p,255)%257
uint8_t inv_prim_root[128]={86, 103, 43, 147, 180, 150, 202, 230, 90, 75, 238, 101, 148, 132, 115, 145, 45, 163, 6, 40, 175, 166, 126, 97, 119, 243, 179, 102, 87, 74, 149, 181, 66, 24, 186, 247, 201, 151, 210, 96, 127, 3, 65, 20, 209, 152, 216, 83, 53, 28, 63, 5, 164, 177, 245, 188, 224, 250, 218, 38, 54, 220, 51, 85, 172, 206, 37, 203, 219, 39, 7, 33, 69, 12, 80, 93, 252, 194, 229, 204, 174, 41, 105, 48, 237, 192, 254, 130, 161, 47, 106, 56, 10, 71, 233, 191, 76, 108, 183, 170, 155, 78, 14, 138, 160, 131, 91, 82, 217, 251, 94, 212, 112, 142, 125, 109, 156, 19, 182, 167, 27, 55, 107, 77, 110, 214, 154, 171};
//the formula for twisting with primitive roots:
// y=prim_root[i]*(x+1)%257-1;
// x=inv_prim_root[i]*(y+1)%257-1;
// you can do the same with 16 bit because (2^16+1) is prime and you have exactly 2^15 primitive roots for that prime.
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
void zpn_mat(uint64_t *in, uint64_t *out)
{
	out[0]=(in[0]<<1)+in[1]+in[2]+(in[3]<<2);
	out[1]=in[0]+in[1]+in[2]+in[3]+(in[3]<<2);
	out[2]=(in[0]<<1)+in[0]+in[1]+in[2]+(in[3]<<1);
	out[3]=(in[0]<<1)+in[2]-in[3];
}
void zpn_imat(uint64_t *in, uint64_t *out)
{
        out[0]=3*in[0]-(in[1]<<1)-in[2];
        out[1]=-in[0]-(in[0]<<3)+5*in[1]+5*in[2]-in[3];
        out[2]=-4*in[0]+3*in[1]+in[2]+in[3];
        out[3]=2*in[0]-in[1]-in[2];
}
void zpn_mixbit(uint64_t* x,uint64_t *out)
{
	uint64_t b1=0x4c6ba2e966660f0f;
	uint64_t b2=0x53a63a2b3333ff00;
	uint64_t m[4]={b1&b2,b1&(~b2),b2&(~b1),~b1&~b2};
	out[0]=(m[0]&x[0])|(m[1]&x[1])|(m[2]&x[2])|(m[3]&x[3]);
	out[1]=(m[0]&x[1])|(m[1]&x[0])|(m[2]&x[3])|(m[3]&x[2]);
	out[2]=(m[0]&x[2])|(m[1]&x[3])|(m[2]&x[0])|(m[3]&x[1]);
	out[3]=(m[0]&x[3])|(m[1]&x[2])|(m[2]&x[1])|(m[3]&x[0]);
}// this function is the inverse of itself


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
int zpn_expand_key(uint8_t *key, uint32_t length, uint32_t cycles, struct zpn_key * ts)
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
		zpn_feed_sponge(sponge,sponge_len,0);
		ts->final_xor[2] = ((uint64_t*)sponge)[0];
		zpn_feed_sponge(sponge,sponge_len,0);
		ts->final_xor[3] = ((uint64_t*)sponge)[0];
		zpn_feed_sponge(sponge,sponge_len,0);
		uint32_t prim = ((uint64_t*)sponge)[0];
		uint32_t pv = prim & 0xff;
		uint32_t zl = (prim>>8) & 0x7ff;
		uint8_t ll = zl&0xff;
		prim = prim_root[(prim >> 19) & 0x7f];
		for ( i = 0 ; i < 256 ; ++ i)
		{
			pv=((pv+1)*prim)%257 - 1;
			ts->lookup[0][i]=ll^pv;
			ts->lookup[1][ll^pv]=i;
		}
		for (i=0;i<cycles*4;++i)
		{
			zpn_feed_sponge(sponge,sponge_len,0);
			ts->cxor[i/4][i%4]=((uint64_t*)sponge)[0];
			zpn_feed_sponge(sponge,sponge_len,0);
			ts->cadd[i/4][i%4]=((uint64_t*)sponge)[0];
		}
		free(sponge);
	}
	else
	{ // dummy values for testing purpose only.
		ts->counter_mask[0] = 0x12345faa0987efab;
		ts->counter_mask[1] = 0xabcd4567f123a5a5;
		ts->final_xor[0] = 0x88a456246ef87d35;
		ts->final_xor[1] = 0x762dfe7a0bc41768;

		uint32_t prim = 555578;
		uint32_t pv = prim & 0xff;
		uint32_t zl = (prim>>8) & 0x7ff;
		uint8_t ll = zl&0xff;
		prim = prim_root[(prim >> 19) & 0x7f];
		for ( i = 0 ; i < 256 ; ++ i)
		{
			pv=((pv+1)*prim)%257 - 1;
			ts->lookup[0][i]=ll^pv;
			ts->lookup[1][ll^pv]=i;
		}
		for (i=0; i < cycles*4; ++i)
		{
			ts->cxor[i/4][i%4]=i;
			ts->cadd[i/4][i%4]=i<<5;
		}
	}
	return 0;
}

void zpn_encrypt(uint64_t nounce, uint64_t counter, struct zpn_key *key, uint8_t *raw, uint8_t *enc)
{
	uint64_t counter_df[4];
	counter_df[0]=counter;
	counter_df[1]=key->counter_mask[0];
	counter_df[2]=nounce;
	counter_df[3]=key->counter_mask[1];

	ZPN_DIFFUSE256(counter_df[0], counter_df[1], counter_df[2], counter_df[3],
	0xa5a5a5a578787878,0x2d2d2d2d1e1e1e1e,0x969696964b4b4b4b,0x3c3c3c3cf0f0f0f0);
	uint32_t i, b;
#define o64 ((uint64_t*)enc)
	o64[0]=((uint64_t*)raw)[0]^counter_df[0];
	o64[1]=((uint64_t*)raw)[1]^counter_df[1];
	o64[2]=((uint64_t*)raw)[2]^counter_df[2];
	o64[3]=((uint64_t*)raw)[3]^counter_df[3];
	uint64_t temp[4];
/*	o64[0]*=18150808750145UL;//inverse: *4065217
	o64[1]*=18150808750145UL;//inverse: *4065217
	o64[2]*=18150808750145UL;//inverse: *4065217
	o64[3]*=18150808750145UL;//inverse: *4065217
*/
	MUL18150808750145(o64[0],temp[0]);
	MUL18150808750145(o64[1],temp[0]);
	MUL18150808750145(o64[2],temp[0]);
	MUL18150808750145(o64[3],temp[0]);
	for (i =0 ; i< key->cycles ; ++i)
	{
		ZPN_DIFFUSE256(o64[0], o64[1], o64[2], o64[3],key->cxor[i][0],
		key->cxor[i][1],key->cxor[i][2],key->cxor[i][3]);
		*((v32b*)enc)=__builtin_shufflevector(*(v32b*)(enc),*(v32b*)(enc), 12, 6, 10, 16, 3, 20, 27, 23, 18, 17, 0, 9, 1, 19, 14, 5, 29, 7, 8, 26, 4, 22, 11, 2, 15, 13, 21, 31, 24, 28, 30, 25);
		zpn_mixbit(o64,temp);
		temp[0]=ROTL64(temp[0],61);
		temp[1]=ROTL64(temp[1],30);
		temp[2]=ROTL64(temp[2],41);
		temp[3]=ROTL64(temp[3],19);
		zpn_imat(temp,o64);
#define ROTF 55
		
		temp[0]=((o64[0]<<ROTF)|(o64[3]>>(64-ROTF)))+key->cadd[i][0];
		temp[1]=((o64[1]<<ROTF)|(o64[0]>>(64-ROTF)))+key->cadd[i][1];
		temp[2]=((o64[2]<<ROTF)|(o64[1]>>(64-ROTF)))+key->cadd[i][2];
		temp[3]=((o64[3]<<ROTF)|(o64[2]>>(64-ROTF)))+key->cadd[i][3];
#undef ROTF 
		zpn_mat(temp,o64);
	}

/*
	o64[0]*=4065217;
	o64[1]*=4065217;
	o64[2]*=4065217;
	o64[3]*=4065217;
*/
	MUL4065217(o64[0],temp[0]);
	MUL4065217(o64[1],temp[0]);
	MUL4065217(o64[2],temp[0]);
	MUL4065217(o64[3],temp[0]);

	o64[0]^=key->final_xor[0];
	o64[1]^=key->final_xor[1];
	o64[2]^=key->final_xor[2];
	o64[3]^=key->final_xor[3];
}

void zpn_decrypt(uint64_t nounce, uint64_t counter, struct zpn_key *key, uint8_t *raw, uint8_t *enc)
{
#define d64 ((uint64_t*)raw)
	d64[0]=o64[0]^key->final_xor[0];
	d64[1]=o64[1]^key->final_xor[1];
	d64[2]=o64[2]^key->final_xor[2];
	d64[3]=o64[3]^key->final_xor[3];
#undef o64
	uint64_t temp[4];
/*	
	d64[0]*=18150808750145UL;//inverse: *4065217
	d64[1]*=18150808750145UL;//inverse: *4065217
	d64[2]*=18150808750145UL;//inverse: *4065217
	d64[3]*=18150808750145UL;//inverse: *4065217
*/
	MUL18150808750145(d64[0],temp[0]);
	MUL18150808750145(d64[1],temp[0]);
	MUL18150808750145(d64[2],temp[0]);
	MUL18150808750145(d64[3],temp[0]);
	int32_t i, b;

#define t64 temp
	for (i = key->cycles - 1 ; i >= 0 ; --i)
	{
		zpn_imat(d64,temp);
		t64[0]-=key->cadd[i][0];
		t64[1]-=key->cadd[i][1];
		t64[2]-=key->cadd[i][2];
		t64[3]-=key->cadd[i][3];
#define ROTF 55
		d64[0]=(t64[0]>>ROTF)|(t64[1]<<(64-ROTF));
		d64[1]=(t64[1]>>ROTF)|(t64[2]<<(64-ROTF));
		d64[2]=(t64[2]>>ROTF)|(t64[3]<<(64-ROTF));
		d64[3]=(t64[3]>>ROTF)|(t64[0]<<(64-ROTF));
#undef ROTF
		zpn_mat(d64,t64);
		temp[0]=ROTL64(temp[0],3);
		temp[1]=ROTL64(temp[1],34);
		temp[2]=ROTL64(temp[2],23);
		temp[3]=ROTL64(temp[3],45);


		zpn_mixbit(temp,d64);
		*((v32b*)raw)=__builtin_shufflevector(*(v32b*)(raw),*(v32b*)(raw), 10, 12, 23, 4, 20, 15, 1, 17, 18, 11, 2, 22, 0, 25, 14, 24, 3, 9, 8, 13, 5, 26, 21, 7, 28, 31, 19, 6, 29, 16, 30, 27);
		REV_ZPN_DIFFUSE256(d64[0], d64[1], d64[2], d64[3],key->cxor[i][0],
		key->cxor[i][1],key->cxor[i][2],key->cxor[i][3]);

	}
	MUL4065217(d64[0],temp[0]);
	MUL4065217(d64[1],temp[0]);
	MUL4065217(d64[2],temp[0]);
	MUL4065217(d64[3],temp[0]);
/*
	d64[0]*=4065217UL;
	d64[1]*=4065217UL;
	d64[2]*=4065217UL;
	d64[3]*=4065217UL;
*/
	uint64_t counter_df[4];
	counter_df[0]=counter;
	counter_df[1]=key->counter_mask[0];
	counter_df[2]=nounce;
	counter_df[3]=key->counter_mask[1];

	ZPN_DIFFUSE256(counter_df[0], counter_df[1], counter_df[2], counter_df[3],
	0xa5a5a5a578787878,0x2d2d2d2d1e1e1e1e,0x969696964b4b4b4b,0x3c3c3c3cf0f0f0f0);
	d64[0]^=counter_df[0];
	d64[1]^=counter_df[1];
	d64[2]^=counter_df[2];
	d64[3]^=counter_df[3];
}


