#include "cryptmath.hpp"

int main()
{
	typedef ZASqMatrix<ZAMultiDimTable<ZAComplex<ZAIntegerModP<uint16_t, uint32_t, 65521>>, 2, 3, 2, 2>, 3> Asym_t;
	std::ios::sync_with_stdio(false);
	Asym_t base; //alice and bob decide to use common base.

	base.randomize();
	ZAKey<512> alice_private_key;
	alice_private_key.randomize();
	std::cout << "alice_private_key:   ";
	alice_private_key.print();
	ZAKey<512> bob_private_key;
	bob_private_key.randomize();
	std::cout << "  bob_private_key:   ";
	bob_private_key.print();
	Asym_t alice_public_key, bob_public_key;
	//alice takes base and raises it in power of her private-key
	alice_public_key = createPK<Asym_t, 512>(base, alice_private_key);
	//bob takes base and raises it in power of his private-key
	bob_public_key = createPK<Asym_t, 512>(base, bob_private_key);

	std::cout << "base:   ";
	hexprint(base);
	//   base.print();
	std::cout << "alice_public_key:   ";
	hexprint(alice_public_key);
	//    alice_public_key.print();
	std::cout << "  bob_public_key:   ";
	hexprint(bob_public_key);
	//    bob_public_key.print();

	//alice send her public key to bob, and vice-versa.
	//generating shared secret
	Asym_t alice_shared_key, bob_shared_key;
	//alice raises bob's public key in power of her private-key
	alice_shared_key = createPK<Asym_t, 512>(bob_public_key, alice_private_key);
	//bob raises alice's public key in power of his private-key
	bob_shared_key = createPK<Asym_t, 512>(alice_public_key, bob_private_key);
	//now bob and alice shares a common secret.

	std::cout << "alice_shared_key:   ";
	hexprint(alice_shared_key);
	//    alice_shared_key.print();

	std::cout << "  bob_shared_key:   ";
	hexprint(bob_shared_key);
	//    bob_shared_key.print();

	std::cout << "size of shared secret: " << Asym_t::size() << " bytes" << std::endl;

	// std::cout << "randomness test: " << std::endl;
	// BYTE base_ba[Asym_t::size()];
	// base.write(base_ba);
	// Asym_t b_pow2;
	// b_pow2= base*base*base;

	// Asym_t b2,b2_pow2;
	// base_ba[0]^=1;
	// b2.read(base_ba);
	// b2_pow2 = b2*b2*b2;
	// std::cout << "pow(base ,3):";
	// hexprint(b_pow2);
	// std::cout << "pow(base',3):";
	// hexprint(b2_pow2);
	return 0;
}