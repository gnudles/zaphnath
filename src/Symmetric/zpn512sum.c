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
#include "i_stream.h"

#define BUF_SIZE 64
char buf[BUF_SIZE];
uint64_t hashbuf[8];//512 bit
char hexbuf[129];//512 bit
int main (int argc, char ** argv)
{
    struct i_stream file_stream;
    int arg_i = 1;
    if (argc == 1)
    {
        arg_i = 0;
    }
    for (; arg_i < argc; ++arg_i)
    {
        FILE * fptr;
        if (arg_i == 0 || strcmp(argv[arg_i],"-") ==0)
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
        init_i_stream_FILE(&file_stream,fptr);
        zpn_hash_calculate (&file_stream,hashbuf);
        for (int i = 0; i<8; ++i)
        {
            int64toHex(hashbuf[i],&hexbuf[i*16]);
        }
        hexbuf[128] = '\0';
        
        printf("%s  ",hexbuf);
        if (fptr == stdin)
            printf("-\n");
        else
        {
            printf("%s\n",argv[arg_i]);
            fclose(fptr);
        }
    }
	return 0;
}
