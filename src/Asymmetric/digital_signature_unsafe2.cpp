#include "cryptmath.hpp"
#include <chrono>
/* This algorithm is by definition unsafe.
* An attacker can use existing signature to create another signatures.
* The only way to make it work is by using Time stamp authority, but any timestamp authority cannot use this method.
*/
int main()
{
    typedef ZASqMatrix<ZAMultiDimTable<ZAComplex<ZAIntegerModP<uint16_t, uint32_t, 65521>>, 2, 3, 2, 2>, 3> Asym_t;
    std::ios::sync_with_stdio(false);
    Asym_t base; //alice choose un-invertible base to create a public key.

    base.randomize();
    //alice choose a random private key greater than the size of message checksums
    ZAKey<640> alice_private_key;
    alice_private_key.randomize();
    alice_private_key[ZAKey<640>::n_qw - 1] |= 0x8000000000000000; //set the MSB of the private key, to make avoid overflow on subtraction
    std::cout << "alice_private_key (private):     ";
    alice_private_key.print();

    Asym_t alice_public_key;
    //alice takes base and raises it in power of her private-key
    alice_public_key = createPK<Asym_t, 640>(base, alice_private_key);

    //alice want to sign a message with the following 512bit checksum
    uint64_t message_checksum[10] = {683842996535, 24363274, 5344411790621, 42513666,
                                     32423234, 23415, 111111111, 42513666,
       0x0000,0x0000};//last two is not part of the checksum
       //set the message checksum high-part with time stamp.
    uint64_t time_stamp = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
    message_checksum [8] =  time_stamp;

    ZAKey<640> message_checksum_key;
    message_checksum_key.read((BYTE*)message_checksum);
    std::cout << "message_checksum_key (public):   ";
    message_checksum_key.print();

    //alice subtract 1 from her private key, and raises it in power of the checksum
\
    ZAKey<640> one_key;
    one_key.toIdentity();
    ZAKey<640> alice_minus_one = alice_private_key - one_key;
    std::cout << "alice_minus_one (private):             ";
    alice_minus_one.print();
    ZAKey<640> checksum_minus_one = message_checksum_key - one_key;
    std::cout << "base (public):   ";
    hexprint(base);
    //   base.print();
    std::cout << "alice_public_key (public):   ";
    hexprint(alice_public_key);
    std::cout << "alice_public_key (public) raised in checksum:   ";
    hexprint(createPK<Asym_t, 640>(alice_public_key,message_checksum_key));
    std::cout << "time stamp:   "<< time_stamp<<std::endl;
    
    //    alice_public_key.print();

    //alice publish her public message signature, with base and message checksum
    Asym_t alice_public_signature = createPK<Asym_t, 640>(base, alice_minus_one);
    alice_public_signature = createPK<Asym_t, 640>(alice_public_signature, message_checksum_key);
    alice_public_signature = alice_public_signature * createPK<Asym_t, 640>(base, checksum_minus_one);
    std::cout << "alice_public_signature (public):   ";
    hexprint(alice_public_signature);

    //bob want to verify Alice signature, so he takes alice public key and raises it in power of message checksum.
    //then he multiply base with alice public signature and he should get the same value.
    //Asym_t verify_signature;


    Asym_t should_be_alice_public_key_power_of_message_checksum = base * alice_public_signature;
    std::cout << "multiplying base with signature:   ";
    std::cout << "should_be_alice_public_key_power_of_message_checksum:   ";
    hexprint(should_be_alice_public_key_power_of_message_checksum);

    std::cout << "size of signature: " << Asym_t::size() << " bytes" << std::endl;

    return 0;
}
