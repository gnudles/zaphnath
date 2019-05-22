
all: dh_keyXchange dsa
dh_keyXchange: cryptmath.hpp diffie_hellman.cpp
	clang++ -g --std=c++17 diffie_hellman.cpp -o dh_keyXchange

dsa: cryptmath.hpp digital_signature.cpp
	clang++ -g --std=c++17 digital_signature.cpp -o dsa
