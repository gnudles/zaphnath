#include <stdint.h>
#include <string.h>
#include "rotate_macro.h"

typedef uint8_t v32b __attribute__((__vector_size__(32)));
typedef uint16_t v16w __attribute__((__vector_size__(32)));
typedef uint32_t v8dw __attribute__((__vector_size__(32)));
typedef uint64_t v4qw __attribute__((__vector_size__(32)));




#define ALL_ONES64 0xffffffffffffffffUL



#define ZPN_TWISTER(A,B,C,D)\
A+=ROTL64(B,5+((C)&31))^(~(C));B+=ROTL64(C,7+((D)&31))&(~(D));\
C^=ROTL64(D,15+((A)&31))+(A);D-=ROTL64(A,31+((B)&31))+(~B);

#define REV_ZPN_TWISTER(A,B,C,D)\
D+=ROTL64(A,31+((B)&31))+(~B);C^=ROTL64(D,15+((A)&31))+(A);\
B-=ROTL64(C,7+((D)&31))&(~(D));A-=ROTL64(B,5+((C)&31))^(~(C));


#define ZPN_TWISTER_MULTI(A,B,C,D)\
A+=ROTL64(B,1)+ROTL64(C,4)+ROTL64(D,8);B+=ROTL64(A,53)+ROTL64(C,5)+ROTL64(D,3);\
C+=ROTL64(A,49)+ROTL64(B,37)+ROTL64(D,2);D+=ROTL64(A,2)+ROTL64(B,6)+ROTL64(C,12);

#define REV_ZPN_TWISTER_MULTI(A,B,C,D)\
D-=ROTL64(A,2)+ROTL64(B,6)+ROTL64(C,12);C-=ROTL64(A,49)+ROTL64(B,37)+ROTL64(D,2);\
B-=ROTL64(A,53)+ROTL64(C,5)+ROTL64(D,3);A-=ROTL64(B,1)+ROTL64(C,4)+ROTL64(D,8);


#define ZPN_DIFFUSE_ROUND(A, B, C, D, R, T)\
A+=ROTL64(B,5)-B; C^=D+T; B=ROTL64(B,8); D+=(A);A=ROTL64(A,R);

#define REV_ZPN_DIFFUSE_ROUND(A, B, C, D, R, T)\
A=ROTL64(A,64-(R));D-=(A);B=ROTL64(B,56);C^=(D)+(T);A-=ROTL64(B,5)-(B);

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

static inline void int64toHex(uint64_t x, char* s)
{
    char map[16]={'0','1','2','3',
    '4','5','6','7',
    '8','9','a','b',
    'c','d','e','f'};
    for (int i = 0 ; i< 16 ; ++i)
    {
        s[i]=map[(x>>(60-(i<<2))) & 15];
    }
}
