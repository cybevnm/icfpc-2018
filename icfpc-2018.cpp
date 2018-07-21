#include "icfpc-2018.hpp"
#include <fstream>
#include <iostream>

namespace {

template<typename S>
void throw_bad_file(const S& f, const std::string& path)
{
	if(!f)
	{
		throw std::runtime_error("Bad model file " + path);
	}
}

}

namespace icfpc2018 {

Matrix read_model_file(const std::string& path)
{
	std::ifstream f(path);
	if(!f)
	{
		throw std::runtime_error("Can't open " + path);
	}

	unsigned char r = 0;
	f.read(reinterpret_cast<char*>(&r), 1);

	std::cerr << (int)(unsigned char)r << std::endl;

	if(!f || f.gcount() != 1 || r == 0)
	{
		throw std::runtime_error("Can't read R from " + path);
	}

	Matrix result(r);

	std::vector<char> data(r * r * r / 8);
	f.read(data.data(), data.size());
	if(!f || f.gcount() != data.size())
	{
		throw std::runtime_error("Can't read data from " + path); 
	}

	for(int x = 0; x < r; ++x)
	{
		for(int y = 0; y < r; ++y)
		{
			for(int z = 0; z < r; ++z)
			{
				const uint8_t& byte = data[(x * r * r + y * r + z) / 8];
				const size_t i = z % 8;
				const bool full = byte & (1 << i);
				result.set_voxel(Vec(x, y, z), full);
			}
		}
	}

	return result;
}

void write_model_file(const Matrix& m, const std::string& path)
{
	std::ofstream f(path);
	if(!f)
	{
		throw std::runtime_error("Can't open " + path);
	}

	unsigned char r = m.R();
	f.write(reinterpret_cast<char*>(&r), 1);
	if(!f)
	{
		throw std::runtime_error("Can't write R to " + path);
	}

	std::vector<char> data(r * r * r / 8);

	for(int x = 0; x < r; ++x)
	{
		for(int y = 0; y < r; ++y)
		{
			for(int z = 0; z < r; ++z)
			{
				char& byte = data[(x * r * r + y * r + z) / 8];
				const size_t i = z % 8;
				if(m.voxel(Vec(x, y, z)))
				{
					byte = byte | (1 << i);
				}
			}
		}
	}

	f.write(data.data(), data.size());
	if(!f)
	{
		throw std::runtime_error("Can't write data to " + path); 
	}
}

}
