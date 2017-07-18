// Author: Dmitry Kukovinets (d1021976@gmail.com), 17.07.2017, 14:31

#include <iostream>

#include "pi_data.hpp"


int
main()
{
	std::size_t off = 1000000;
	
	pi::test_bytes(off + 0);
	
	std::cout << "..............................";
	pi::test_bytes(off + 30);
	
	return 0;
}

// double, 1e-17, 100
//          0: 243f6a8885a3
//        100: 29b7c97c50dd
//      1 000: 49f1c09b07
//     10 000: 8ac8fcfb801
//    100 000: 35ea16c406
// 10 000 000: 7af5863ef

// long double, 1e-40, 100
//        100: 29b7c97c50dd3f84
//      1 000: 49f1c09b075372
//     10 000: 8ac8fcfb8016c
//    100 000: 35ea16c406363a
// 10 000 000: 7af5863efed8d49
// 10 000 001: af5863efed8de

// long double, 1e-17, 100
//    100 000: 35ea16c406363a
// 10 000 001: af5863efed8de

// mpf_float_100, 1e-50, 100
//    100 000: 35ea16c406363a30bf0b2e693992b58f7205a7232c4168840b6a4

// mpf_float_100, 1e-70, 100
//    100 000: 35ea16c406363a30bf0b2e693992b58f7205a7232c4168840b6a48ecb67eaa

// mpf_float_100, 1e-80, 100
//    100 000: 35ea16c406363a30bf0b2e693992b58f7205a7232c4168840b6a48ecb67eaa2a5b9d3c

// mpf_float_100, 1e-80, 100
//      10e+9: 


// long double, 1e-17, 100
// 15 000 000: 281f5412646a24d32963cd62ee17c8cb8a86df7f2ed5cccab9339410facebee7abf83d7

// mpf_float_100, 1e-80, 100
// 15 000 001: 81f5412646a24d32963cd62ee17c8cb8a86df7f2ed5cccab9339410facebee7abf83d7

// 15m
// 281f5412646a24d32963cd62ee17c8cb8a86df7f2ed5cccab9339410facebee7abf83d72b3f0dd7bab9d66f083014c9c872fc0e0141578e00000000000000000000000000000
// .81f5412646a24d32963cd62ee17c8cb8a86df7f2ed5cccab9339410facebee7abf83d72ff0011a3d985d5580fc38b70a00000000000000000000000000000000000000000000
// ..............................cb8a86df7f2ed5cccab9339410facebee7abf83d73040115120e1ca0d5b0bd2616826152cfb4f366365d30fc9d7acdd100000000000000000000000000000000000000000000
// ...............................b8a86df7f2ed5cccab9339410facebee7abf83d73040115120e1ca0d5b0bd26168261531ac413e146d601a20eb764eaa00000000000000000000000000000000000000000000
