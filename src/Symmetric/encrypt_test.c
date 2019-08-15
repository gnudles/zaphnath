#include <stdio.h>
#include "zaphnath.h"

int main()
{
	struct zpn_key ts;
	uint8_t key[40]={36, 85, 82, 38, 19, 222, 249, 198, 154, 134, 254, 209, 131, 16, 83, 93, 90, 179, 89, 168, 42, 215, 13, 134, 139, 203, 216, 106, 78, 156, 75, 190, 105, 12, 194, 157, 235, 129, 102, 62};
	uint8_t in[32]={'z','a','p','h',
		'n','a','t','h','Y','o','s','e','f'};
		
	data256 din;
	data256 dout;
	
	zpn_bytes_to_data256(in,din);
	zpn_expand_key(key,40,18,&ts);
#if 0
	int j= 0;
	int i= 0;
	for (j=0;j<10000000;++j)
	{
	((uint64_t*)in)[0]*=2;
	((uint64_t*)in)[0]++;

	zpn_encrypt_new(0x4,0x50,&ts,in,out);
	/*in[31]^=0x8;
	printf("\n\n\n\n");
	zpn_encrypt(0x4,0x50,&ts,in,out);
	in[31]^=0x8;
	printf("\n\n\n\n");*/
	in[31]^=out[31];
	}
#endif
	printf("\n\n\n\n\n");
	zpn_encrypt(0x4,0x51,&ts,din,dout);
	printf("\n\n\n\n\n");
	//out[0]^=0x1;
	//zpn_encrypt(0x4,0x51,&ts,in,out);
	//printf("\n\n\n\n\n");
	zpn_decrypt(0x4,0x51,&ts,din,dout);
	printf("\nbye bye\n");
	return 0;
}

