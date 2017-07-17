// Author: Dmitry Kukovinets (d1021976@gmail.com), 17.07.2017, 14:31

#ifndef PI_ARCHIVER_PI_DATA_HPP
#define PI_ARCHIVER_PI_DATA_HPP


namespace pi {


unsigned char get_byte(std::size_t n);
void write_bytes(void *buf, std::size_t size, std::size_t start_n);
void test_bytes(std::size_t start_n);


};	// namespace pi


#endif	// PI_ARCHIVER_PI_DATA_HPP
