#include "cryptmath.hpp"

int main()
{
    typedef ZASqMatrix<ZAMultiDimTable<ZAComplex<ZAIntegerModP<uint16_t, uint32_t, 65521>>, 2, 3, 5>, 3> Asym_t;
    std::ios::sync_with_stdio(false);
    Asym_t base; //alice choose un-invertible base to create a public key.

    base.randomize();
    //alice choose a random private key greater than the size of message checksums (576bit in this case)
    ZAKey<576> alice_private_key;
    alice_private_key.randomize();
    alice_private_key[ZAKey<576>::n_qw - 1] |= 0x8000000000000000; //set the MSB of the private key, to make avoid overflow on subtraction
    std::cout << "alice_private_key (private):     ";
    alice_private_key.print();

    Asym_t alice_public_key;
    //alice takes base and raises it in power of her private-key
    alice_public_key = createPK<Asym_t, 576>(base, alice_private_key);

    //alice want to sign a message with the following 512bit checksum
    // uint64_t message_checksum[9] = {683842996535, 24363274, 5344411790621, 42513666,
    //                                 32423234, 23415, 111111111, 42513666,
    //   0x0000};//last one is not part of the checksum
    ZAKey<576> message_checksum_key;
    //message_checksum_key.read((BYTE*)message_checksum);
    message_checksum_key.randomize();
    message_checksum_key[ZAKey<576>::n_qw - 1] = 0; //set last to zero because the checksum is only 512 bits.
    std::cout << "message_checksum_key (public):   ";
    message_checksum_key.print();

    //alice subtract the checksum from her private key:
    ZAKey<576> signature_key;
    signature_key = alice_private_key - message_checksum_key;
    std::cout << "signature_key (private):         ";
    signature_key.print();

    std::cout << "base (public):   ";
    hexprint(base);
    //   base.print();
    std::cout << "alice_public_key (public):   ";
    hexprint(alice_public_key);
    //    alice_public_key.print();

    //alice publish her public message signature, with base and message checksum
    Asym_t alice_public_signature = createPK<Asym_t, 576>(base, signature_key);
    std::cout << "alice_public_signature (public):   ";
    hexprint(alice_public_signature);

    //bob want to verify Alice signature, so he takes base and raises it in power of message checksum.
    //then he multiply that with alice public signature and he should get Alice's public key.
    Asym_t verify_signature;

    verify_signature = createPK<Asym_t, 576>(base, message_checksum_key);

    std::cout << "verify_signature:   ";
    hexprint(verify_signature);
    //    alice_shared_key.print();

    Asym_t should_be_alice_public_key = verify_signature * alice_public_signature;
    std::cout << "should_be_alice_public_key:   ";
    hexprint(should_be_alice_public_key);

    std::cout << "size of signature: " << Asym_t::size() << " bytes" << std::endl;

    return 0;
}