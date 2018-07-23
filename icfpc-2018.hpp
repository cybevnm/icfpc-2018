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
	explicit Vec(int x = 0, int y = 0, int z = 0)
	: x(x), y(y), z(z)
	{
	}

	int mlen() const
	{
		return std::abs(x) + std::abs(y) + std::abs(z);
	}

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

inline std::ostream& operator<<(std::ostream& s, const Vec& v)
{
	s << "(" << v.x << ", " << v.y << ", " << v.z << ")";
	return s;
}

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
	explicit Matrix(unsigned R)
	: m_r(R)
	{
		assert(R > 0 && R < 251);
		m_bits.resize(R*R*R);
	}

	unsigned r() const
	{
		return m_r;
	}

	bool voxel(const Vec& c) const
	{
		assert(c.valid_coordinate());
		return m_bits[c.x * m_r * m_r + c.y * m_r + c.z];
	}

	void set_voxel(const Vec& c, bool full)
	{
		assert(c.valid_coordinate());
		m_bits[c.x * m_r * m_r + c.y * m_r + c.z] = full;
	}

	std::pair<bool, Region> calc_bounding_region() const;

	std::pair<bool, Region> calc_bounding_region_y(int y) const;

	void print(std::ostream& s) const;
	
private:
	std::vector<uint8_t> m_bits;
	unsigned m_r;
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

		Void,

		// FussionP,
		// FussionS,

		// GFill,
		// GVoid,
	};

	Command()
	: m_type(Undefined)
	{
	}

	explicit Command(Type type)
	: m_type(type)
	{
	}

	Type type() const
	{
		return m_type;
	}

	const std::pair<bool, Vec>&	arg0() const
	{
		return m_arg0;
	}

	static Command halt();

	static Command flip();

	static Command smove(const Vec& arg);

	static Command smove_x(int x);

	static Command smove_y(int y);

	static Command smove_z(int z);

	static Command fill(const Vec& arg);

	static Command fill_below();

	static Command voiid(const Vec& arg);

	static Command voiid_below();

	void serialize(std::ostream& s) const;

private:
	Type m_type;
	std::pair<bool, Vec> m_arg0;
};

class Bot
{
public:
	Bot(unsigned id, unsigned parent_id)
	: m_id(id)
	, m_parent_id(parent_id)
	{
	}

	unsigned id() const
	{
		return m_id;
	}

	unsigned parent_id() const
	{
		return m_parent_id;
	}

	Vec& pos()
	{
		return m_pos;
	}

private:
	unsigned m_id;
	unsigned m_parent_id;
	Vec m_pos;
	std::vector<unsigned> m_seeds;
};

enum class Harmonics { Low, High };

class System
{
public:
	explicit System(const Matrix& matrix);

	/// Allows to continue the src execution.
	System(const System& src, const Matrix& matrix);

	void serialize_trace(std::ostream& s);

	uint64_t energy() const
	{
		return m_energy;
	}

	const Vec& bot_pos() const
	{
		return m_pos;
	}

	const Matrix& matrix() const
	{
		return m_matrix;
	}

	Matrix& out_matrix()
	{
		return m_out_matrix;
	}

public:
	void push(Command command);

	void step();

	void push_and_step(Command command);

private:
	void move_to_x(int x);
	void move_to_y(int y);
	void move_to_z(int z);

public:
	enum class MovementOrder { XZY, YZX };

	void move_to(const Vec& tgt,
		MovementOrder order = MovementOrder::XZY);

private:
	Matrix m_matrix;
	Matrix m_out_matrix;

	Harmonics m_harmonics = Harmonics::Low;
	uint64_t m_energy = 0;

	Vec m_pos;

	std::pair<bool, Command> m_curr_command;
	std::vector<Command> m_trace;
};

class Tracer
{
public:
	enum class Direction { Up, Down };

	Tracer(System& system, Direction dir)
	: m_system(system), m_dir(dir)
	{
	}

	virtual ~Tracer() { }

	void run();

	void halt();

protected:
	System& m_system;

	Direction m_dir;

private:
	virtual void handle_voxel(const Vec& p) = 0;

private:
	void scan_xz_plane(int y);

private:
	Region m_bounding_region;

};

class Assembler : public Tracer
{
public:
	explicit Assembler(System& system)
	: Tracer(system, Direction::Up)
	{
	}

private:
	void handle_voxel(const Vec& p) override;
};

class Disassembler : public Tracer
{
public:
	explicit Disassembler(System& system)
	: Tracer(system, Direction::Down)
	{
		system.out_matrix() = system.matrix();
	}

private:
	void handle_voxel(const Vec& p) override;
};

} //
