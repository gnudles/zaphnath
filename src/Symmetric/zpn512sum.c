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
char buf[BUF_SIZE];
uint64_t hashbuf[8];//512 bit
char hexbuf[129];//512 bit
int main (int argc, char ** argv)
{
    
    int arg_i = 1;
    if (argc == 1)
    {
        arg_i = 0;
    }
    for (; arg_i < argc; ++arg_i)
    {
        FILE * fptr;
        if (arg_i == 0)
        {
            fptr = stdin;
        }
        else
        {
            fptr = fopen(argv[arg_i],"rb");
            if (fptr == 0)
            {
                fprintf(stderr,"%s: %s: %s\n",argv[0], argv[arg_i], strerror(errno));
                continue;
            }
        }

        hash_sponge sponge;
        zpn_hash_init_sponge(sponge);
        hash_chunk chunk;
        int c,buf_len;
        int finalise = 0;
        int step888 = 0;
        int total_read = 0;
        while (finalise != 1)
        {
            buf_len = 0;
            while (buf_len < BUF_SIZE && finalise != 1)
            {
                c = fread(buf, 1, BUF_SIZE - buf_len, fptr);
                total_read += c;
                if (c > 0 )
                {
                    buf_len += c;
                }
                if (c == 0 )
                {
                    
                    finalise = 1;// inform that this is the last buffer.
                }
            }
            zpn_hash_bytes_to_chunk((uint8_t *)buf, chunk);
            
            if (finalise)
            {
                if (buf_len == 64)
                {
                    zpn_hash_absorb(sponge,chunk);
                    zpn_hash_vertical_tilt(sponge,step888);
                    step888+=ZPN_DEFAULT_STEP_INC;
                    buf_len = 0;
                }
                zpn_hash_pad_last_chunk(chunk,buf_len);
                zpn_hash_absorb(sponge,chunk);
                zpn_hash_vertical_tilt(sponge,step888);
                step888+=ZPN_DEFAULT_STEP_INC;

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
            }
            else
            {
                zpn_hash_absorb(sponge,chunk);
                zpn_hash_vertical_tilt(sponge,step888);
                step888+=ZPN_DEFAULT_STEP_INC;
            }
        }
        for (int i = 0; i<8; ++i)
        {
            int64toHex(hashbuf[i],&hexbuf[i*16]);
        }
        hexbuf[128] = '\0';
        
        printf("%s  ",hexbuf);
        if (arg_i == 0)
            printf("-\n");
        else
        {
            printf("%s\n",argv[arg_i]);
            fclose(fptr);
        }
    }
	return 0;
}
