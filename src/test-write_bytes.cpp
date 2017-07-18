// Author: Dmitry Kukovinets (d1021976@gmail.com), 18.07.2017, 19:18

#include <iostream>
#include <iomanip>
#include <array>
#include <algorithm>

#include "pi_data.hpp"


int
main()
{
	constexpr std::size_t off = 1000, off2 = 20;
	std::array<unsigned char, 100> tmp1, tmp2;
	
	pi::write_bytes(tmp1.data(), tmp1.size(), off);
	pi::write_bytes(tmp2.data(), tmp2.size(), off + off2);
	
	std::array<unsigned char, 100 + off2> expected;
	{
		std::size_t i = off;
		for (auto &x: expected)
			x = pi::get_byte(i++);
	}
	
	
	if (
			!std::equal(tmp1.begin() + off2, tmp1.end(), tmp2.begin())
		||
			!std::equal(tmp1.begin(), tmp1.end(), expected.begin())
		||
			!std::equal(tmp2.begin(), tmp2.end(), expected.begin() + off2)
	) {
		std::cout << "          ";
		for (unsigned int x: tmp1)
			std::cout << std::hex << std::setw(2) << std::setfill('0') << x;
		std::cout << std::endl;
		
		std::cout << "          ";
		for (std::size_t i = 0; i < off2; ++i)
			std::cout << "..";
		for (unsigned int x: tmp2)
			std::cout << std::hex << std::setw(2) << std::setfill('0') << x;
		std::cout << std::endl;
		
		std::cout << "Expected: ";
		for (unsigned int x: expected)
			std::cout << std::hex << std::setw(2) << std::setfill('0') << x;
		std::cout << std::endl;
		
		return 1;
	}
	
	return 0;
}
