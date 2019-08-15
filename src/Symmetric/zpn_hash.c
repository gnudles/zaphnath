
#include "zpn_common.h"
#include "zpn_hash.h"
#include <assert.h>
#include <stdio.h>


void zpn_hash_init_sponge(hash_sponge sponge)
{
	memset(sponge,0xcf,sizeof(hash_sponge));
}
void zpn_hash_bytes_to_chunk(uint8_t *bytes, hash_chunk chunk)
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
void zpn_hash_nbytes_to_chunk(uint8_t *bytes, unsigned int length, hash_chunk chunk)
{
    assert (length<=64);
    int i;
    for ( i = 0; i < 8; ++i)
    {
        chunk[i] = 0;
    }
    for ( i = 0; i < length; ++i)
    {
        chunk[i/8] |= ((uint64_t)bytes[i])<<((i%8)*8);
    }
}

static inline void zpn_hash_small_step(uint64_t * a, uint64_t * b, uint64_t * c, uint64_t * d)
{
	uint64_t in[4] = {*a,*b,*c,*d};
	*a = ROTL64(in[0],9)+ROTL64(in[1],1)+ROTL64(in[2],3)+ROTL64(in[3],27);
	*b = ROTL64(in[0],20)+ROTL64(in[1],7)+ROTL64(in[2],10)+ROTL64(in[3],3);
	*c = ROTL64(in[0],24)+ROTL64(in[1],35)+ROTL64(in[2],48)+ROTL64(in[3],63);
	*d = ROTL64(in[0],37)+ROTL64(in[1],42)+ROTL64(in[2],47)+ROTL64(in[3],52);
}
static inline void zpn_hash_nonlinear_step(hash_sponge sponge)
{
    uint64_t sponge_31=sponge[31];
    ++sponge[31];//zero state protection
    sponge[31] ^= ROTL64(sponge[29],sponge[31]&63);
    sponge[29] += ~(ROTL64(sponge[29],51)&sponge_31);
    
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
void zpn_hash_pad_last_chunk(hash_chunk chunk,unsigned int length_of_last_chunk)
{
    assert(length_of_last_chunk < 64);
    chunk[length_of_last_chunk/8] |= ALL_ONES64 << ((length_of_last_chunk % 8)*8);
    for (int i = length_of_last_chunk/8 +1 ; i <8; i++)
    {
        chunk[i] = ALL_ONES64;
    }
    chunk[7] ^= ((uint64_t)length_of_last_chunk) << 56;
}

void zpn_hash_vertical_tilt(hash_sponge sponge, int step888)
{
    #ifdef DEBUG
    printf ("in zpn_feed_hash_sponge. sponge is\n");
    print_sponge(sponge);
    printf ("\nchunk is\n");
    print_chunk(chunk);
    printf ("\n");
    #endif
	
    zpn_hash_nonlinear_step(sponge);
#define zpn_feed_hash_sponge_step(A,B,C,D) zpn_hash_small_step(&sponge[A],&sponge[B],&sponge[C],&sponge[D])
    int off1=((step888>>6)&7),off2=((step888>>3)&7),off3=step888&7;
	for (int i=0;i<8;++i)
		zpn_feed_hash_sponge_step(0+i,8+(off1+i)%8,16+(off2+i)%8,24+(off3+i)%8);
    
#undef zpn_feed_hash_sponge_step

}
void zpn_hash_horizontal_tilt(hash_sponge sponge)
{
    zpn_hash_nonlinear_step(sponge);
	
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

void zpn_hash_sponge_obscure(hash_sponge sponge, int step888)
{
	
    /*
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
*/

    for (int i = 0; i < ZPN_OBSCURE_STEPS ; ++i)
    {
	    zpn_hash_horizontal_tilt(sponge);
        zpn_hash_vertical_tilt(sponge,step888);
        step888 += ZPN_DEFAULT_STEP_INC;
    }
}
