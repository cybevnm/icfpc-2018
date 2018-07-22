#include <iostream>
#include <fstream>

#include "icfpc-2018.hpp"

using namespace icfpc2018;

int main(int argc, char* argv[])
{
	try
	{
		if(argc < 3)
		{
			throw std::runtime_error("Wrong argv");
		}

		std::cerr << "Building trace for " << argv[1]
		  << " into " << argv[2] << std::endl;

		const Matrix m = read_model_file(argv[1]);

		std::cerr << "R: " << m.R() << std::endl;

		System s(m);

		s.build_trace();

		std::ofstream f(argv[2], std::ios::binary);
		if(!f)
		{
			throw std::runtime_error("Can't open " + std::string(argv[2]));
		}

		std::cerr << "Energy: " << s.energy() << std::endl;

		s.serialize_trace(f);

		write_model_file(s.result_matrix(), "/tmp/result_model.mdl");
	}
	catch(const std::runtime_error& e)
	{
		std::cerr << e.what() << std::endl;
		std::cout << "Usage: trace input_model ouput_trace" << std::endl;
		return 1;
	}

	return 0;

}
