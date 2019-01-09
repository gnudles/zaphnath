#include <stdio.h>
#include "zaphnath.h"


int main()
{
	struct time_schedule ts;
	uint8_t key[16]={4,5,66,87,198,14,231,54,
	99,72,44,33,22,11,66,77};
	uint8_t in[16]={2,5,66,87,198,14,231,54,
	99,72,44,33,22,11,66,77};
	uint8_t out[16];
	zpn_expand_key(key,16,7,&ts);
	zpn_encrypt(0x4,0x50,&ts,in,out);
	int i= 0;
	for (i=0;i<16;++i)
		printf("%02x ",in[i]);
	printf("\n");
	for (i=0;i<16;++i)
		printf("%02x ",out[i]);
	printf("\n");
	zpn_decrypt(0x4,0x50,&ts,in,out);
	for (i=0;i<16;++i)
		printf("%02x ",in[i]);
	printf("\n");
	printf("\n");
	in[0]=0;
	zpn_encrypt(0x4,0x50,&ts,in,out);
	for (i=0;i<16;++i)
		printf("%02x ",in[i]);
	printf("\n");
	for (i=0;i<16;++i)
		printf("%02x ",out[i]);
	printf("\n");
	zpn_decrypt(0x4,0x50,&ts,in,out);
	for (i=0;i<16;++i)
		printf("%02x ",in[i]);
	printf("\nbye bye\n");
	return 0;
}

