#include "cryptmath.hpp"

int main()
{
	typedef ZASqMatrix<ZAMultiDimTable<ZAComplex<ZAIntegerModP<uint8_t, uint16_t, 251>>, 2>, 2> Asym_t;
	std::ios::sync_with_stdio(false);
	Asym_t base; //alice and bob decide to use common base.

	base.randomize();
    //base.findInverse();



	std::cout << "randomness test: " << std::endl;
	BYTE base_ba[Asym_t::size()];
	base.write(base_ba);
	Asym_t b_pow2;
	b_pow2= base*base*base;

	Asym_t b2,b2_pow2;
	base_ba[0]^=1;
	b2.read(base_ba);
	b2_pow2 = b2*b2*b2;
	std::cout << "pow(base ,3):";
	hexprint(b_pow2);
	std::cout << "pow(base',3):";
	hexprint(b2_pow2);
	return 0;
}