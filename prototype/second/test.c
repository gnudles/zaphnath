#include <stdio.h>
#include "zaphnath.h"

int main()
{
	struct zpn_key ts;
	uint8_t key[16]={4,65,66,87,198,14,231,54,
	99,72,44,33,22,11,66,77};
	uint8_t in[32]={0,0,0,0};
	uint8_t out[32];
	zpn_expand_key(key,16,1,&ts);
	int j= 0;
	int i= 0;
	for (j=0;j<46;++j)
	{
	((uint64_t*)in)[0]*=2;
	((uint64_t*)in)[0]++;

	zpn_encrypt(0x4,0x51,&ts,in,out);
//	in[0]^=out[0];
	for (i=0;i<32;++i)
		printf("%02x ",in[i]);
	printf("\n");
	for (i=0;i<32;++i)
		printf("%02x ",out[i]);
	printf("\n");

	}
	printf("\n");
	in[0]^=0x8;
	zpn_encrypt(0x4,0x50,&ts,in,out);
	for (i=0;i<32;++i)
		printf("%02x ",in[i]);
	printf("\n");
	for (i=0;i<32;++i)
		printf("%02x ",out[i]);
	printf("\n");
	zpn_decrypt(0x4,0x50,&ts,in,out);
	for (i=0;i<32;++i)
		printf("%02x ",in[i]);
	printf("\nbye bye\n");
	return 0;
}

