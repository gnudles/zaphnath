
#include "zpn_common.h"
#include "zpn_hash.h"
#include <assert.h>


void init_sponge(hash_sponge sponge)
{
	memset(sponge,0x5A,sizeof(hash_sponge));
}
void bytes_to_chunk(uint8_t *bytes, hash_chunk chunk)
{
    for (int i = 0; i < 64; ++i)
    {
        if  (i%8 == 0)
        {
            chunk[i/8] = 0;
        }
        chunk[i/8] |= bytes[i]<<((i%8)*8);
    }
}

static inline void zpn_hash_small_step(uint64_t * a, uint64_t * b, uint64_t * c, uint64_t * d)
{
	uint64_t in[4] = {*a,*b,*c,*d};
    //ZPN_DIFFUSE_ROUND(in[0],in[1],in[2],in[3],45,0xA5714C3F2E960B8DUL);
	*a = ROTL64(in[0],12)+ROTL64(in[1],15)+ROTL64(in[2],18)+ROTL64(in[3],21);
	*b = ROTL64(in[0],4)+ROTL64(in[1],9)+ROTL64(in[2],14)+ROTL64(in[3],19);
	*c = ROTL64(in[0],17)+ROTL64(in[1],25)+ROTL64(in[2],33)+ROTL64(in[3],41);
	*d = ROTL64(in[0],22)+ROTL64(in[1],32)+ROTL64(in[2],42)+ROTL64(in[3],52);
}
void pad_last_chunk(hash_chunk chunk,unsigned int length_of_last_chunk)
{
    assert(length_of_last_chunk < 64);
    chunk[length_of_last_chunk/8] |= ALL_ONES64 << (length_of_last_chunk % 8);
    for (int i=length_of_last_chunk/8 +1 ; i <8; i++)
    {
        chunk[i] = ALL_ONES64;
    }
}
void zpn_feed_hash_sponge(hash_sponge sponge, hash_chunk chunk, int step888)
{
	for (int i=0;i<8;++i)
		sponge[i]^=chunk[i];
#define zpn_feed_hash_sponge_step(A,B,C,D) zpn_hash_small_step(&sponge[A],&sponge[B],&sponge[C],&sponge[D])
    int off1=((step888>>6)&7),off2=((step888>>3)&7),off3=step888&7;
	for (int i=0;i<8;++i)
		zpn_feed_hash_sponge_step(0+i,8+(off1+i)%8,16+(off2+i)%8,24+(off3+i)%8);
	/*zpn_feed_hash_sponge_step(24,25,26,27);
	zpn_feed_hash_sponge_step(28,29,30,31);
	zpn_feed_hash_sponge_step(24,25,30,31);
	zpn_feed_hash_sponge_step(26,27,28,29);*/
    
#undef zpn_feed_hash_sponge_step

}

void zpn_hash_sponge_obscure(hash_sponge sponge)
{
	for (int i=0;i<31;++i)
	{
		sponge[i]+=(sponge[i+1]>>6);
		ROTL64(sponge[i],(sponge[i+1]&63));
	}
	zpn_hash_small_step(&sponge[7],&sponge[15],&sponge[23],&sponge[31]);
	for (int i=0;i<31;++i)
	{
		sponge[i]+=(sponge[i+1]>>6);
		ROTL64(sponge[i],(sponge[i+1]&63));
	}
    int step888=0x57;
    hash_chunk chunk_ones = {ALL_ONES64,ALL_ONES64,ALL_ONES64,ALL_ONES64,
    ALL_ONES64,ALL_ONES64,ALL_ONES64,ALL_ONES64};
    for (int i = 0; i < 4; ++i)
    {
        zpn_feed_hash_sponge(sponge,chunk_ones,step888);
        step888 += 0x4a;
    }
}
