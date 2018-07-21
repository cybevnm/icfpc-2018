/// ICFPC2018 solution code chunks.
/// Copyright (C) 2018 cybevnm

#pragma once

#include <vector>
#include <cassert>
#include <cmath>
#include <string>
#include <stdexcept>

namespace icfpc2018 {

struct Vec
{
	Vec(int x = 0, int y = 0, int z = 0)
	: x(x), y(y), z(z)
	{
	}

	int mlen() const
	{
		return std::abs(x) + std::abs(y) + std::abs(z);
	}

	// bool adjacent() const
	// {
	// }

	int clen() const
	{
		return std::max(std::abs(x), std::max(std::abs(y), std::abs(z)));
	}

	bool ld() const
	{
		return
			(x != 0 && y == 0 && z == 0) ||
			(x == 0 && y != 0 && z == 0) ||
			(x == 0 && y == 0 && z != 0);
	}

	bool sld() const
	{
		return ld() && mlen() < 6;
	}

	bool lld() const
	{
		return ld() && mlen() < 16;
	}

	bool nd() const
	{
		return clen() == 1 && mlen() > 0 && mlen() < 3;
	}

	bool valid_coordinate() const
	{
		return x >= 0 && y >= 0 && z >= 0;
	}

	int x = 0;
	int y = 0;
	int z = 0;
};

struct Region
{
	// bool member()
	// {
	// 
	// }

	Vec a, b;
};

class Matrix
{
public:
	Matrix(unsigned R)
	: r(R)
	{
		assert(R > 0 && R < 251);
		bits.resize(R*R*R);
	}

	unsigned R() const
	{
		return r;
	}

	bool voxel(const Vec& c) const
	{
		assert(c.valid_coordinate());
		return bits[c.x * r * r + c.y * r + c.z];
	}

	void set_voxel(const Vec& c, bool full)
	{
		assert(c.valid_coordinate());
		bits[c.x * r * r + c.y * r + c.z] = full;
	}

	// bool grounded(const Vec& c) const
	// {
	// 	if(c.y == 0)
	// 	{
	// 		return true;
	// 	}
	// 	else
	// 	{
	// 		
	// 	}
	// }
	
private:
	std::vector<int> bits;
	unsigned r;
};

/// @throw std::runtime_error
Matrix read_model_file(const std::string& path);

/// @throw std::runtime_error
void write_model_file(const Matrix& m, const std::string& path);

class Command
{
public:
	enum Type {
		/// No args.
		Halt,
		// Wait,
		// Flip,
		/// lld
		SMove,
		// LMove,
		// Fission,
		/// nd
		Fill,
		// FussionP,
		// FussionS,
	};

	Command(Type type)
	: type(type)
	{

	}

	Type type;
	std::pair<bool, Vec> arg0;
};

enum class Harmonics { Low, High };

class System
{
public:
	System(const Matrix& matrix)
	: matrix(matrix)
	{
	}

	

	unsigned energy = 0;
	Harmonics harmonics = Harmonics::Low;
	Matrix matrix;
	// bots
	// trace
};

}
