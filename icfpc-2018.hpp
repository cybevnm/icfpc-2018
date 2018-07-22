/// ICFPC2018 solution code chunks.
/// Copyright (C) 2018 cybevnm

#pragma once

#include <vector>
#include <cassert>
#include <cmath>
#include <string>
#include <stdexcept>
#include <tuple>
#include <fstream>
#include <iostream>
#include <cstdint>

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

	Vec xz(int y = 0) const
	{
		return Vec(this->x, y, this->z);
	}

	int x = 0;
	int y = 0;
	int z = 0;
};

inline Vec operator+(const Vec& a, const Vec& b)
{
	return Vec(a.x + b.x, a.y + b.y, a.z + b.z);
}

inline Vec operator-(const Vec& a, const Vec& b)
{
	return Vec(a.x - b.x, a.y - b.y, a.z - b.z);
}

inline bool operator==(const Vec& a, const Vec& b)
{
	return std::tie(a.x, a.y, a.z) == std::tie(b.x, b.y, b.z);
}

inline bool operator!=(const Vec& a, const Vec& b)
{
	return !(a == b);
}

/// Inclusive.
struct Region
{
	Region()
	{
	}

	Region(const Vec& a, const Vec& b)
	{
		this->a.x = std::min(a.x, b.x);
		this->a.y = std::min(a.y, b.y);
		this->a.z = std::min(a.z, b.z);

		this->b.x = std::max(a.x, b.x);
		this->b.y = std::max(a.y, b.y);
		this->b.z = std::max(a.z, b.z);
	}

	Vec size() const
	{
		return b - a + Vec(1, 1, 1);
	}

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

	std::pair<bool, Region> calc_bounding_region() const;

	std::pair<bool, Region> calc_bounding_region_y(int y) const;

	void print(std::ostream& s) const;

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
		Undefined,
		Halt,
		// Wait,
		Flip,
		SMove,
		// LMove,
		// Fission,
		Fill,

		// Void,

		// FussionP,
		// FussionS,

		// GFill,
		// GVoid,
	};

	Command()
	: t(Undefined)
	{
	}

	Command(Type type)
	: t(type)
	{
	}

	Type type() const
	{
		return t;
	}

	const std::pair<bool, Vec>&	arg0() const
	{
		return a0;
	}

	static Command halt();

	static Command flip();

	static Command smove(const Vec& arg);

	static Command smove_x(int x);

	static Command smove_y(int y);

	static Command smove_z(int z);

	static Command fill(const Vec& arg);

	static Command fill_below();

	void serialize(std::ostream& s) const;

private:
	Type t;
	std::pair<bool, Vec> a0;
};

enum class Harmonics { Low, High };

class System
{
public:
	System(const Matrix& matrix)
	: mat(matrix)
	, out_mat(matrix.R())
	{
		trace.reserve(5 * 1000 * 1000);
	}

	void serialize_trace(std::ostream& s);

	uint64_t energy() const
	{
		return e;
	}

	const Vec& bot_pos() const
	{
		return pos;
	}

	const Matrix& matrix()
	{
		return mat;
	}

	const Matrix& out_matrix()
	{
		return out_mat;
	}

public:
	void push(Command command);

	void step();

	void push_and_step(Command command);

public:
	void move_to(const Vec& tgt);

private:
	Matrix mat;
	Matrix out_mat;

	Harmonics harmonics = Harmonics::Low;
	uint64_t e = 0;

	Vec pos;

	std::pair<bool, Command> curr_command;
	std::vector<Command> trace;

};

class Builder
{
public:
	Builder(System& system)
	: s(system)
	{
	}

	void build_trace();

private:
	void scan_xz_plane(int y);

private:
	System& s;

private:
	Region bounding_region;
};

}
