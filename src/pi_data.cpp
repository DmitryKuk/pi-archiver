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


#include <array>
#include <utility>
#include <mutex>

#include <boost/multiprecision/gmp.hpp>

#include "pi_data.hpp"


namespace {


template<class Float, std::size_t PowersOf2>
class pi_calculator
{
public:
	using float_type = Float;
	
	
	inline pi_calculator(unsigned int correct_digits = 9, float_type eps = 1e-17):
		correct_digits_{correct_digits},
		eps_{std::move(eps)}
	{
		if (!pi_calculator::is_initialized_) {	// Thanks to GCC
			std::lock_guard<std::mutex> l{pi_calculator::initialization_mutex_};
			if (!pi_calculator::is_initialized_) {
				pi_calculator::powers_of_2_ = pi_calculator::get_powers_of_2();
				pi_calculator::is_initialized_ = true;
			}
		}
	}
	
	
	unsigned char get_byte(std::size_t n);
	void write_bytes(void *buf, std::size_t size, std::size_t start_n);
	void test_bytes(std::size_t start_n);
private:
	static constexpr std::size_t powers_of_2_count_ = PowersOf2;
	static_assert(powers_of_2_count_ > 0, "Incorrect powers_of_2_count_.");
	
	static std::array<float_type, powers_of_2_count_> powers_of_2_;
	static std::mutex initialization_mutex_;
	static bool is_initialized_;
	
	
	static std::array<float_type, powers_of_2_count_> get_powers_of_2();
	static float_type expm(float_type p, const float_type &ak);
	
	
	const unsigned int correct_digits_;
	const float_type eps_;
	
	float_type series(unsigned int m, std::size_t n);
};	// class pi_calculator



// public
template<class Float, std::size_t PowersOf2>
unsigned char
pi_calculator<Float, PowersOf2>::get_byte(std::size_t n)
{
	using std::floor;
	using boost::multiprecision::floor;
	using std::trunc;
	using boost::multiprecision::trunc;
	using std::abs;
	using boost::multiprecision::abs;
	
	
	n *= 2;	// Number of byte, not hex digit
	
	pi_calculator::float_type y = 4 * series(1, n) - 2 * series(4, n) - series(5, n) - series(6, n);
	y = abs(y - trunc(y) + 1);
	y = 256 * (y - floor(y));
	
	return static_cast<unsigned char>(y);
}


template<class Float, std::size_t PowersOf2>
void
pi_calculator<Float, PowersOf2>::write_bytes(void *buf, std::size_t size, std::size_t start_n)
{
	using std::floor;
	using boost::multiprecision::floor;
	using std::trunc;
	using boost::multiprecision::trunc;
	using std::abs;
	using boost::multiprecision::abs;
	
	
	start_n *= 2;	// Number of byte, not hex digit
	
	pi_calculator::float_type y;
	auto byte_buf = static_cast<unsigned char *>(buf);
	
	const auto write_seq =
		[&](std::size_t seq_size)
		{
			y = 4 * series(1, start_n) - 2 * series(4, start_n) - series(5, start_n) - series(6, start_n);
			y = abs(y - trunc(y) + 1);
			
			for (std::size_t i = 0; i < seq_size; ++i, ++byte_buf) {
				y = 256 * (y - floor(y));
				*byte_buf = static_cast<unsigned char>(y);
			}
		};
	
	
	const auto stop_n = start_n + size * 2;
	
	{
		const auto correct_digits = this->correct_digits_ & (~static_cast<decltype(this->correct_digits_)>(1));
		for (; start_n + correct_digits < stop_n; start_n += correct_digits)
			write_seq(correct_digits / 2);
	}
	
	{
		const auto trailing = stop_n - start_n;
		if (trailing > 0)
			write_seq(trailing);
	}
}



template<class Float, std::size_t PowersOf2>
// static
constexpr std::size_t
	pi_calculator<Float, PowersOf2>::powers_of_2_count_;

template<class Float, std::size_t PowersOf2>
// static
std::array<typename pi_calculator<Float, PowersOf2>::float_type, pi_calculator<Float, PowersOf2>::powers_of_2_count_>
	pi_calculator<Float, PowersOf2>::powers_of_2_;

template<class Float, std::size_t PowersOf2>
// static
std::mutex
	pi_calculator<Float, PowersOf2>::initialization_mutex_;

template<class Float, std::size_t PowersOf2>
// static
bool
	pi_calculator<Float, PowersOf2>::is_initialized_ = false;



// private
template<class Float, std::size_t PowersOf2>
// static
std::array<typename pi_calculator<Float, PowersOf2>::float_type, pi_calculator<Float, PowersOf2>::powers_of_2_count_>
pi_calculator<Float, PowersOf2>::get_powers_of_2()
{
	std::array<pi_calculator::float_type, pi_calculator::powers_of_2_count_> res;
	
	pi_calculator::float_type p = 1;
	for (auto &r: res) {
		r = p;
		p *= 2;
	}
	
	return res;
}


// expm = 16^p mod ak
// NOTE: Uses the left-to-right binary exponentiation scheme.
template<class Float, std::size_t PowersOf2>
// static
typename pi_calculator<Float, PowersOf2>::float_type
pi_calculator<Float, PowersOf2>::expm(
	typename pi_calculator<Float, PowersOf2>::float_type p,
	const typename pi_calculator<Float, PowersOf2>::float_type &ak
)
{
	using std::trunc;
	using boost::multiprecision::trunc;
	
	if (ak == 1)
		return 0;
	
	// Greatest power of two less than or equal to p
	pi_calculator::float_type pt = pi_calculator::powers_of_2_[0];
	std::size_t i;
	for (i = 1; i < pi_calculator::powers_of_2_.size(); ++i)
		if (pi_calculator::powers_of_2_[i] > p) {
			pt = pi_calculator::powers_of_2_[i - 1];
			break;
		}
	
	// Binary exponentiation algorithm modulo ak
	pi_calculator::float_type r = 1;
	for (std::size_t j = 0; j < i; ++j) {
		if (p >= pt) {
			p -= pt;
			r *= 16;
			r -= trunc(r / ak) * ak;
		}
		pt *= 0.5;
		if (pt >= 1) {
			r *= r;
			r -= trunc(r / ak) * ak;
		}
	}
	
	return r;
}


// Evaluates the series sum_k 16^(n-k)/(8*k+m) using the modular exponentiation technique.
template<class Float, std::size_t PowersOf2>
typename pi_calculator<Float, PowersOf2>::float_type
pi_calculator<Float, PowersOf2>::series(unsigned int m, std::size_t n)
{
	using std::trunc;
	using boost::multiprecision::trunc;
	
	typename pi_calculator<Float, PowersOf2>::float_type s = 0;
	
	// Sum the series up to n
	{
		pi_calculator::float_type ak = m;
		for (std::size_t k = 0; k < n; ++k) {
			ak = 8 * k + m;
			s += expm(n - k, ak) / ak;
			s -= trunc(s);
		}
	}
	
	// Compute a few terms where k >= n
	{
		pi_calculator::float_type t, power_of_16 = 1;
		for (std::size_t k = n; k <= n + 100; ++k, power_of_16 /= 16) {
			t = power_of_16 / (8 * k + m);
			if (t < this->eps_)
				break;
			
			s += t;
			s -= trunc(s);
		}
	}
	
	return s;
}


};  // namespace



namespace pi {


unsigned char
get_byte(std::size_t n)
{
	constexpr std::size_t threshold = 7'500'000;
	
	if (n <= threshold) {
		pi_calculator<long double, 25> c;
		return c.get_byte(n);
	} else {
		pi_calculator<boost::multiprecision::mpf_float_100, 35> c{68, 1e-90};
		return c.get_byte(n);
	}
}


void
write_bytes(void *buf, std::size_t size, std::size_t start_n)
{
	constexpr std::size_t threshold = 7'500'000;
	
	if (start_n <= threshold) {
		const auto low_size = std::min(size, threshold - start_n);
		size -= low_size;
		
		pi_calculator<long double, 25> c;
		c.write_bytes(buf, low_size, start_n);
		buf = static_cast<unsigned char *>(buf) + low_size;
	}
	
	const auto stop_n = start_n + size;
	if (threshold < stop_n) {
		pi_calculator<boost::multiprecision::mpf_float_100, 35> c{68, 1e-90};
		c.write_bytes(buf, size, start_n);
	}
}


};	// namespace pi
