#include <stdint.h>
//#include <stdio.h>
uint64_t find_mod_invert(uint64_t x, uint64_t bits)
{
	if (bits > 64 || ((x&1)==0))
		return 0;
	uint64_t mask = 1;
	mask<<=bits;
	mask--;
	x&=mask;
	uint64_t result=1, state=x, ctz=0;
	while(state!=1)
	{
		ctz=__builtin_ctzll(state^1);
		result|=1<<ctz;
		state+=x<<ctz;
		state&=mask;
	}
	return result;
}
/*
int main()
{
	uint64_t i;
	uint64_t r=55332889;
	for (i=0;i<6000;i++)
	{
		r^=(r<<13)+66899043217;
		r*=58995422567ULL+i;
		r|=1;
		printf("%llu, %llu, \n",r, find_mod_invert(r,64));
	}
	for (i=1;i<512;i+=2)
	{
		printf("%llu: %llu, \n",i>>1, find_mod_invert(i,9)>>1);
	}
	return 0;
}
*/
