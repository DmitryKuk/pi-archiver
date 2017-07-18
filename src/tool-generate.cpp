// Author: Dmitry Kukovinets (d1021976@gmail.com), 17.07.2017, 14:31

#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <thread>
#include <mutex>
#include <atomic>
#include <stdexcept>

#include <boost/filesystem.hpp>
#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/program_options.hpp>

#include "pi_data.hpp"


void
create_file(const boost::filesystem::path &file_path, std::streamoff file_size)
{
	namespace fs = boost::filesystem;
	
	
	// Remove old file
	if (fs::exists(file_path)) {
		if (fs::is_directory(file_path))
			throw std::runtime_error{"File path is directory"};
		
		if (!boost::interprocess::file_mapping::remove(file_path.c_str()))
			throw std::runtime_error{"Can\'t remove old file"};
	}
	
	
	// Create new file
	std::filebuf filebuf;
	filebuf.open(file_path.c_str(), std::ios_base::out | std::ios_base::trunc | std::ios_base::binary);
	
	
	// Set file size
	if (file_size > 0) {
		filebuf.pubseekoff(file_size - 1, std::ios_base::beg);
		filebuf.sputc(0);
	}
	
	
	// Check result
	{
		const auto make_unsigned =
			[](auto x)
			{
				return static_cast<typename std::make_unsigned<decltype(x)>::type>(x);
			};
		
		if (filebuf.close() == nullptr || make_unsigned(fs::file_size(file_path)) != make_unsigned(file_size))
			throw std::runtime_error{"Can\'t create file of given size"};
	}
}



int
main(int argc, char **argv)
{
	constexpr std::size_t prefferred_block_size = 136;	// LCD(68 / 2, 9 / 2)
	
	
	namespace po = boost::program_options;
	
	const auto print_help =
		[&](std::ostream &stream = std::cout)
		{
			stream
				<< "Usage:    " << argv[0] << " [OPTIONS] [OUTPUT_FILE]" << std::endl
				<< "See help: " << argv[0] << " --help" << std::endl;
		};
	
	
	std::size_t offset, size = 0, block_size = prefferred_block_size;
	std::string output_file;
	unsigned int jobs = 1;
	
	
	try {
		po::options_description options_description("Allowed options");
		options_description.add_options()
			(
				"help,h",
				"Print help message and exit"
			)
			
			(
				"output-file,o", po::value(&output_file),
				"Set output file path"
			)
			
			(
				"size,s", po::value(&size),
				"Set calculating chunk of Pi size (in bytes)"
			)
			
			(
				"offset,O", po::value(&offset)->default_value(0),
				"Set Pi offset (in bytes)"
			)
			
			(
				"block-size,b", po::value(&block_size)->default_value(prefferred_block_size),
				"Set block size (in bytes)"
			)
			
			(
				"jobs,j", po::value(&jobs)->default_value(1),
				"Set jobs count"
			)
		;
		
		po::positional_options_description positional_options;
		positional_options.add("output-file", -1);
		
		
		po::variables_map vm;
		
		po::store(
			po::command_line_parser(argc, argv).options(options_description).positional(positional_options).run(),
			vm
		);
		
		po::notify(vm);
		
		if (vm.count("help")) {
			print_help();
			std::cout << options_description << std::endl;
			return 0;
		}
		
		if (output_file.empty()) {
			std::clog
				<< "Incorrect output file." << std::endl
				<< "Try: \"" << argv[0] << " --help\" for help." << std::endl;
			return 1;
		}
		
		if (size <= 0) {
			std::clog
				<< "Incorrect chunk of Pi size (expected > 0)." << std::endl
				<< "Try: \"" << argv[0] << " --help\" for help." << std::endl;
			return 1;
		}
		
		if (block_size <= 0) {
			std::clog
				<< "Incorrect block size (expected > 0)." << std::endl
				<< "Try: \"" << argv[0] << " --help\" for help." << std::endl;
			return 1;
		}
		
		if (jobs < 1) {
			std::clog
				<< "Incorrect jobs count (expected >= 1)." << std::endl
				<< "Try: \"" << argv[0] << " --help\" for help." << std::endl;
			return 1;
		}
		
		
		create_file(output_file, size);
		boost::interprocess::file_mapping mapping{output_file.c_str(), boost::interprocess::read_write};
		boost::interprocess::mapped_region region{mapping, boost::interprocess::read_write};
		if (size != region.get_size())
			throw std::runtime_error{"Incorrect output file size"};
		
		const auto buf = static_cast<unsigned char *>(region.get_address());
		const auto buf_end = buf + size;
		std::atomic<std::size_t> processing_size{0}, processed_size{0};
		double old_progress = 0;
		std::mutex cout_mutex;
		
		
		const auto calculate_pi =
			[&]
			{
				while (true) {
					const auto curr_offset = processing_size.fetch_add(block_size);
					if (curr_offset >= size)
						break;
					
					const auto curr_size = std::min(block_size, static_cast<std::size_t>(buf_end - buf));
					pi::write_bytes(buf + curr_offset, curr_size, offset + curr_offset);
					
					const auto progress =
						(processed_size.fetch_add(curr_size) + curr_size) * 100 / static_cast<double>(size);
					if (progress - old_progress >= 0.01 && cout_mutex.try_lock()) {
						std::lock_guard<std::mutex> l{cout_mutex, std::adopt_lock};
						
						if (progress - old_progress >= 0.01) {
							old_progress = progress;
							std::cout
								<< "\b\b\b\b\b\b\b\b\b\b"
								<< std::setw(6) << std::setprecision(2) << std::fixed
								<< progress << std::setw(0) << "%...";
							std::cout.flush();
						}
					}
				}
			};
		
		
		std::cout << "Completed:   0.00%...";
		
		std::vector<std::thread> threads;
		threads.reserve(jobs);
		for (unsigned int i = 0; i < jobs; ++i)
			threads.emplace_back(calculate_pi);
		for (auto &thread: threads)
			thread.join();
		
		std::cout << std::endl << "Completed." << std::endl;
	} catch (const std::exception &e) {
		std::cerr << "Error: " << e.what() << '.' << std::endl;
		return 1;
	}
	
	return 0;
}


// Experiments with precision:
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
