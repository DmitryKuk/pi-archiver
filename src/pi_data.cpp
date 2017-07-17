// Author: Dmitry Kukovinets (d1021976@gmail.com), 17.07.2017, 14:31

// Based on original David H. Bailey implementation.

// This program implements the BBP algorithm to generate a few hexadecimal
// digits beginning immediately after a given position n, or in other words
// beginning at position n + 1. On most systems using IEEE 64-bit floating-
// point arithmetic, this code works correctly so long as n is less than
// approximately 1.18 * 10 ^ 7. If 80-bit arithmetic can be employed, this limit
// is significantly higher. Whatever arithmetic is used, results for a given
// position n can be checked by repeating with n - 1 or n + 1, and verifying
// that the hex digits perfectly overlap with an offset of one, except possibly
// for a few trailing digits. The resulting fractions are typically accurate
// to at least 11 decimal digits, and to at least 9 hex digits.
// David H. Bailey     2006-09-08


#include <iostream>
#include <array>
#include <cmath>

#include "pi_data.hpp"


namespace {


constexpr std::size_t powers_of_2_count = 25;


std::array<long double, powers_of_2_count>
get_powers_of_2() noexcept
{
	std::array<long double, powers_of_2_count> res;
	
	long double p = 1;
	for (auto &r: res) {
		r = p;
		p *= 2;
	}
	
	return res;
}


// expm = 16^p mod ak
// NOTE: Uses the left-to-right binary exponentiation scheme.
long double
expm(long double p, long double ak) noexcept
{
	if (ak == 1)
		return 0;
	
	// Greatest power of two less than or equal to p
	long double pt;
	std::size_t i;
	{
		static const auto tp = get_powers_of_2();
		static_assert(powers_of_2_count > 0, "Incorrect powers_of_2_count.");
		pt = tp[0];
		for (i = 1; i < tp.size(); ++i)
			if (tp[i] > p) {
				pt = tp[i - 1];
				break;
			}
	}
	
	
	// Binary exponentiation algorithm modulo ak
	long double p1 = p;
	long double r = 1;
	for (std::size_t j = 1; j <= i; ++j) {
		if (p1 >= pt) {
			p1 -= pt;
			r *= 16;
			r -= std::trunc(r / ak) * ak;
		}
		pt *= 0.5;
		if (pt >= 1) {
			r *= r;
			r -= std::trunc(r / ak) * ak;
		}
	}
	
	return r;
}



// Evaluates the series sum_k 16^(n-k)/(8*k+m) using the modular exponentiation technique.
long double
series(long m, long n)
{
	constexpr long double eps = 1e-40;
	long double s = 0;
	
	// Sum the series up to n
	for (long k = 0; k < n; ++k) {
		long double ak = 8 * k + m;
		s += expm(n - k, ak) / ak;
		s -= std::trunc(s);
	}
	
	// Compute a few terms where k >= n
	for (long k = n; k <= n + 100; ++k) {
		long double ak = 8 * k + m;
		
		long double t = std::pow(16, n - k) / ak;
		if (t < eps)
			break;
		
		s += t;
		s -= std::trunc(s);
	}
	
	return s;
}


};  // namespace



namespace pi {


unsigned char
get_byte(long n) noexcept
{
	long double y = 4 * series(1, n) - 2 * series(4, n) - series(5, n) - series(6, n);
	y = std::abs(y - std::trunc(y) + 1);
	y = 16 * (y - std::floor(y));
	
	const auto first = static_cast<unsigned char>(y);
	y = 16 * (y - std::floor(y));
	const auto second = static_cast<unsigned char>(y);
	
	return (first << 4) | second;
}


// void
// write_bytes(void *buf, std::size_t size, long start_n)
// {
// }


void
test_bytes(long start_n)
{
	long double y = 4 * series(1, start_n) - 2 * series(4, start_n) - series(5, start_n) - series(6, start_n);
	y = std::abs(y - std::trunc(y) + 1);
	
	
	for (int i = 0; i < 15; ++i) {
		y = 16 * (y - std::floor(y));
		auto first = static_cast<unsigned char>(y);
		y = 16 * (y - std::floor(y));
		auto second = static_cast<unsigned char>(y);
		
		std::cout << std::hex << static_cast<unsigned int>(first) << static_cast<unsigned int>(second);
	}
	std::cout << std::endl;
}


};	// namespace pi
