// Author: Dmitry Kukovinets (d1021976@gmail.com), 17.07.2017, 14:31

#include <iostream>

#include "pi_data.hpp"


int
main()
{
	long off = 100000;
	
	pi::test_bytes(off + 0);
	
	std::cout << ".......";
	pi::test_bytes(off + 7);
	
	return 0;
}

// double, 1e-17
//          0: 243f6a8885a3
//        100: 29b7c97c50dd
//      1 000: 49f1c09b07
//     10 000: 8ac8fcfb801
//    100 000: 35ea16c406
// 10 000 000: 7af5863ef

// long double, 1e-40
//        100: 29b7c97c50dd3f84
//      1 000: 49f1c09b075372
//     10 000: 8ac8fcfb8016c
//    100 000: 35ea16c406363a
// 10 000 000: 7af5863efed8d49
// 10 000 001: af5863efed8de

// long double, 1e-17
//    100 000: 35ea16c406363a
// 10 000 001: af5863efed8de
