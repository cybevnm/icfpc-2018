#include <iostream>
#include <fstream>

#include "icfpc-2018.hpp"

using namespace icfpc2018;

int main(int argc, char* argv[])
{
	try
	{
		if(argc != 5)
		{
			throw std::runtime_error("Wrong argv");
		}

		std::cerr << "Building trace for reassembling " << argv[1]
		  << " into " << argv[2] << "."
			<< " Trace is in " << argv[3] << "."
			<< " Resulting model is in " << argv[4] << "."
			<< std::endl;

		const Matrix m1 = read_model_file(argv[1]);
		std::cerr << "R1: " << m1.R() << std::endl;

		System ds(m1);
		Disassembler d(ds);
		d.run();

		const Matrix m2 = read_model_file(argv[2]);
		std::cerr << "R2: " << m2.R() << std::endl;

		System as(ds, m2);
		Assembler a(as);
		a.run();
		a.halt();

		std::cerr << "Energy: " << as.energy() << std::endl;

		std::ofstream f(argv[3], std::ios::binary);
		if(!f)
		{
			throw std::runtime_error("Can't open " + std::string(argv[3]));
		}

		as.serialize_trace(f);

		write_model_file(as.out_matrix(), argv[4]);
	}
	catch(const std::runtime_error& e)
	{
		std::cerr << e.what() << std::endl;
		std::cout << "Usage: reassemble src_model tgt_mdl out_trace out_matrix"
			<< std::endl;
		return 1;
	}

	return 0;

}