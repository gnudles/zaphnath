#include <stdio.h>
#include "zaphnath.h"

extern void zpn_mixbit(uint32_t* x);
int main()
{
	struct zpn_key ts;
	uint8_t key[16]={4,5,66,87,198,14,231,54,
	99,72,44,33,22,11,66,77};
	uint8_t in[32]={87,87,87,87,1,1,1,1,
	33,33,33,33,71,71,71,71};
	uint8_t out[32];
	zpn_expand_key(key,16,8,&ts);
	zpn_encrypt(0x4,0x50,&ts,in,out);
	int i= 0;
	for (i=0;i<32;++i)
		printf("%02x ",in[i]);
	printf("\n");
	for (i=0;i<32;++i)
		printf("%02x ",out[i]);
	printf("\n");
	zpn_decrypt(0x4,0x50,&ts,in,out);
	for (i=0;i<32;++i)
		printf("%02x ",in[i]);
	printf("\n");
	printf("\n");
	in[0]=0;
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

