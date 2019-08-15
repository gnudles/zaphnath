#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>


#include "zpn_hash.h"
#include "zpn_common.h"

#define BUF_SIZE 64
char buf[BUF_SIZE]={0};
uint64_t hashbuf[8];//512 bit
char hexbuf[129];//512 bit
int main (int argc, char ** argv)
{
        hash_sponge sponge;
        zpn_hash_init_sponge(sponge);
        hash_chunk chunk;
        int finalise = 0;
        int step888 = 0;
	zpn_hash_bytes_to_chunk((uint8_t *)buf, chunk);
	zpn_hash_absorb(sponge,chunk);
        zpn_hash_vertical_tilt(sponge,step888);
	step888+=ZPN_DEFAULT_STEP_INC;
        while (finalise != 1)
        {
                
                zpn_hash_sponge_obscure(sponge,step888);
		step888+=ZPN_DEFAULT_STEP_INC*ZPN_OBSCURE_STEPS;
                hashbuf[0]=sponge[0];
                hashbuf[1]=sponge[1];
                hashbuf[2]=sponge[2];
                hashbuf[3]=sponge[3];
                zpn_hash_sponge_obscure(sponge,step888);
		step888+=ZPN_DEFAULT_STEP_INC*ZPN_OBSCURE_STEPS;
                hashbuf[0]^=sponge[0];
                hashbuf[1]^=sponge[1];
                hashbuf[2]^=sponge[2];
                hashbuf[3]^=sponge[3];
                
                zpn_hash_sponge_obscure(sponge,step888);
		step888+=ZPN_DEFAULT_STEP_INC*ZPN_OBSCURE_STEPS;
                
                hashbuf[4]=sponge[0];
                hashbuf[5]=sponge[1];
                hashbuf[6]=sponge[2];
                hashbuf[7]=sponge[3];
                zpn_hash_sponge_obscure(sponge,step888);
		step888+=ZPN_DEFAULT_STEP_INC*ZPN_OBSCURE_STEPS;
                
                hashbuf[4]^=sponge[0];
                hashbuf[5]^=sponge[1];
                hashbuf[6]^=sponge[2];
                hashbuf[7]^=sponge[3];
		int res = write(1,hashbuf,64);
                if (res != 64)
                        break;

        }
	

	return 0;
}
