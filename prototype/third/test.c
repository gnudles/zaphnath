#include <stdio.h>
#include "zaphnath.h"

int main()
{
	struct zpn_key ts;
	uint8_t key[40]={37, 85, 82, 38, 19, 222, 249, 198, 154, 134, 254, 209, 131, 16, 83, 93, 90, 179, 89, 168, 42, 215, 13, 134, 139, 203, 216, 106, 78, 156, 75, 190, 105, 12, 194, 157, 235, 129, 102, 62};
	uint8_t in[32]={0,0,0,0,0,0,0,0};
	uint8_t out[32];
	zpn_expand_key(key,16,6,&ts);
	int j= 0;
	int i= 0;
	for (j=0;j<0;++j)
	{
	((uint64_t*)in)[0]*=2;
	((uint64_t*)in)[0]++;

	zpn_encrypt(0x4,0x50,&ts,in,out);
	in[31]^=0x8;
	printf("\n\n\n\n");
	zpn_encrypt(0x4,0x50,&ts,in,out);
	in[31]^=0x8;
	printf("\n\n\n\n");

	}
	printf("\n\n\n\n\n");
	zpn_encrypt(0x4,0x50,&ts,in,out);
	printf("\n\n\n\n\n");
	//in[0]^=0x8;
	zpn_encrypt(0x4,0x51,&ts,in,out);
	printf("\n\n\n\n\n");
	zpn_decrypt(0x4,0x51,&ts,in,out);
	printf("\nbye bye\n");
	return 0;
}

