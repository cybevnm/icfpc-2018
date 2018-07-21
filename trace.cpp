#include <iostream>

#include "icfpc-2018.hpp"

using namespace icfpc2018;

int main(int argc, char* argv[])
{
	try
	{
		if(argc < 2)
		{
			throw std::runtime_error("Wrong argv");
		}

		const Matrix m = read_model_file(argv[1]);

		System s(m);

		

	}
	catch(const std::runtime_error& e)
	{
		std::cerr << e.what() << std::endl;
		return 1;
	}

	return 0;

}
