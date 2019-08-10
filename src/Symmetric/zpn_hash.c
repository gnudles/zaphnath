
#include "zpn_common.h"
#include "zpn_hash.h"
#include <assert.h>
#include <stdio.h>


void init_sponge(hash_sponge sponge)
{
	memset(sponge,0xcf,sizeof(hash_sponge));
}
void bytes_to_chunk(uint8_t *bytes, hash_chunk chunk)
{
    for (int i = 0; i < 8; ++i)
    {
        chunk[i] = 0;
    }
    for (int i = 0; i < 64; ++i)
    {
        chunk[i/8] |= ((uint64_t)bytes[i])<<((i%8)*8);
    }
}

static inline void zpn_hash_small_step(uint64_t * a, uint64_t * b, uint64_t * c, uint64_t * d)
{
	uint64_t in[4] = {*a,*b,*c,*d};
	*a = ROTL64(in[0],12)+ROTL64(in[1],15)+ROTL64(in[2],18)+ROTL64(in[3],21);
	*b = ROTL64(in[0],4)+ROTL64(in[1],9)+ROTL64(in[2],14)+ROTL64(in[3],19);
	*c = ROTL64(in[0],17)+ROTL64(in[1],25)+ROTL64(in[2],33)+ROTL64(in[3],41);
	*d = ROTL64(in[0],22)+ROTL64(in[1],32)+ROTL64(in[2],42)+ROTL64(in[3],52);
}
void print_chunk(hash_chunk chunk)
{
    char temp[17];
    temp[16] = 0;
    for (int i= 0; i<8; ++i)
    {
        int64toHex(chunk[i],temp);
        printf("%s",temp);
    }
}
void print_sponge(hash_sponge sponge)
{
    char temp[17];
    temp[16] = 0;
    for (int i= 0; i<32; ++i)
    {
        int64toHex(sponge[i],temp);
        printf("%s",temp);
    }
}
void pad_last_chunk(hash_chunk chunk,unsigned int length_of_last_chunk)
{
    assert(length_of_last_chunk < 64);
    chunk[length_of_last_chunk/8] |= ALL_ONES64 << ((length_of_last_chunk % 8)*8);
    for (int i = length_of_last_chunk/8 +1 ; i <8; i++)
    {
        chunk[i] = ALL_ONES64;
    }
    chunk[7] ^= ((uint64_t)length_of_last_chunk) << 56;
}
void zpn_feed_hash_sponge(hash_sponge sponge, hash_chunk chunk, int step888)
{
    #ifdef DEBUG
    printf ("in zpn_feed_hash_sponge. sponge is\n");
    print_sponge(sponge);
    printf ("\nchunk is\n");
    print_chunk(chunk);
    printf ("\n");
    #endif
	for (int i=0;i<8;++i)
		sponge[i]^=chunk[i];
	++sponge[31];//zero state protection
#define zpn_feed_hash_sponge_step(A,B,C,D) zpn_hash_small_step(&sponge[A],&sponge[B],&sponge[C],&sponge[D])
    int off1=((step888>>6)&7),off2=((step888>>3)&7),off3=step888&7;
	for (int i=0;i<8;++i)
		zpn_feed_hash_sponge_step(0+i,8+(off1+i)%8,16+(off2+i)%8,24+(off3+i)%8);
    
#undef zpn_feed_hash_sponge_step

}
void zpn_hzn_tilt_sponge(hash_sponge sponge)
{
#define zpn_feed_hash_sponge_step(A,B,C,D) zpn_hash_small_step(&sponge[A],&sponge[B],&sponge[C],&sponge[D])
	zpn_feed_hash_sponge_step(0,1,2,3);
	zpn_feed_hash_sponge_step(4,5,6,7);
	zpn_feed_hash_sponge_step(8,9,12,13);
	zpn_feed_hash_sponge_step(10,11,14,15);
	zpn_feed_hash_sponge_step(16,18,20,22);
	zpn_feed_hash_sponge_step(17,19,21,23);
	zpn_feed_hash_sponge_step(24,25,30,31);
	zpn_feed_hash_sponge_step(26,27,28,29);
#undef zpn_feed_hash_sponge_step
}

void zpn_hash_sponge_obscure(hash_sponge sponge)
{

    int step888 = 0x47;
    hash_chunk chunk_ones = {ALL_ONES64,ALL_ONES64,ALL_ONES64,ALL_ONES64,
    ALL_ONES64,ALL_ONES64,ALL_ONES64,ALL_ONES64};
    for (int i = 0; i < 4; ++i)
    {
	zpn_hzn_tilt_sponge(sponge);
        zpn_feed_hash_sponge(sponge,chunk_ones,step888);
        step888 += 0x5d;
    }
	uint64_t sponge_0 = sponge[0];
    for (int i=0;i<31;++i)
    {
	sponge[i]+=(sponge[i+1]>>6);
	ROTL64(sponge[i],(sponge[i+1]&63));
        sponge[i+1] &= sponge[i];
    }
	sponge[31]+=(sponge_0>>6);
	ROTL64(sponge[31],(sponge_0&63));
	sponge[0]+=(sponge_0)& sponge[31];
}
