/// ICFPC2018 solution code chunks.
/// Copyright (C) 2018 cybevnm

#include "icfpc-2018.hpp"
#include <limits>
#include <algorithm>
#include <sstream>

namespace icfpc2018 {

std::pair<bool, Region> Matrix::calc_bounding_region() const
{
	const int max = std::numeric_limits<int>::max();
	const int min = std::numeric_limits<int>::min();

	Vec a(max, max, max);
	Vec b(min, min, min);

	for(int x = 0; x < r(); ++x)
	{
		for(int y = 0; y < r(); ++y)
		{
			for(int z = 0; z < r(); ++z)
			{
				if(voxel(Vec(x, y, z)))
				{
					a.x = std::min(x, a.x);
					a.y = std::min(y, a.y);
					a.z = std::min(z, a.z);

					b.x = std::max(x, b.x);
					b.y = std::max(y, b.y);
					b.z = std::max(z, b.z);
				}
			}
		}
	}
	
	if(b.x < a.x || b.y < a.y || b.z < a.z)
	{
		return std::make_pair(false, Region());
	}
	else
	{
		return std::make_pair(true, Region(a, b));
	}
}

std::pair<bool, Region> Matrix::calc_bounding_region_y(int y) const
{
	const int max = std::numeric_limits<int>::max();
	const int min = std::numeric_limits<int>::min();

	Vec a(max, y, max);
	Vec b(min, y, min);

	for(int x = 0; x < r(); ++x)
	{
		for(int z = 0; z < r(); ++z)
		{
			if(voxel(Vec(x, y, z)))
			{
				a.x = std::min(x, a.x);
				a.z = std::min(z, a.z);

				b.x = std::max(x, b.x);
				b.z = std::max(z, b.z);
			}
		}
	}

	if(b.x < a.x || b.z < a.z)
	{
		return std::make_pair(false, Region());
	}
	else
	{
		return std::make_pair(true, Region(a, b));
	}
}

void Matrix::print(std::ostream& s) const
{
	for(auto y = 0; y < r(); ++y)
	{
		for(auto x = 0; x < r(); ++x)
		{
			for(auto z = 0; z < r(); ++z)
			{
				s << (voxel(Vec(x,y,z)) ? 'x' : '.');
			}
			s << std::endl;
		}
		s << "---------------------" << std::endl;
	}
}

Matrix read_model_file(const std::string& path)
{
	std::ifstream f(path);
	if(!f)
	{
		throw std::runtime_error("Can't open " + path);
	}

	unsigned char r = 0;
	f.read(reinterpret_cast<char*>(&r), 1);

	if(!f || f.gcount() != 1 || r == 0)
	{
		throw std::runtime_error("Can't read R from " + path);
	}

	Matrix result(r);

	const unsigned rrr = r * r * r;
	std::vector<char> data((rrr / 8) + ((rrr % 8) ? 1 : 0));
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
				const size_t i = x * r * r + y * r + z;
				const uint8_t byte = data[i / 8];
				const bool full = byte & (1 << i % 8);
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

	unsigned char r = m.r();
	f.write(reinterpret_cast<char*>(&r), 1);
	if(!f)
	{
		throw std::runtime_error("Can't write R to " + path);
	}

	const unsigned rrr = r * r * r;
	std::vector<char> data((rrr / 8) + ((rrr % 8) ? 1 : 0));

	for(int x = 0; x < r; ++x)
	{
		for(int y = 0; y < r; ++y)
		{
			for(int z = 0; z < r; ++z)
			{
				const size_t i = x * r * r + y * r + z;
				char& byte = data.at(i / 8);
				if(m.voxel(Vec(x, y, z)))
				{
					byte = byte | (1 << i % 8);
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

Command Command::halt()
{
	return Command(Halt);
}

Command Command::flip()
{
	return Command(Flip);
}

Command Command::smove(const Vec& arg)
{
	if(!arg.lld())
	{
		throw std::runtime_error("smove: arg is not lld");
	}
	Command c(SMove);
	c.m_arg0.first = true;
	c.m_arg0.second = arg;
	return c;
}

Command Command::smove_x(int x)
{
	return smove(Vec(x, 0, 0));
}

Command Command::smove_y(int y)
{
	return smove(Vec(0, y, 0));
}

Command Command::smove_z(int z)
{
	return smove(Vec(0, 0, z));
}

Command Command::fill(const Vec& arg)
{
	if(!arg.nd())
	{
		throw std::runtime_error("fill: arg is not nd");
	}

	Command c(Fill);
	c.m_arg0.first = true;
	c.m_arg0.second = arg;
	return c;
}

Command Command::fill_below()
{
	return fill(Vec(0, -1, 0));
}

Command Command::voiid(const Vec& arg)
{
	if(!arg.nd())
	{
		throw std::runtime_error("void: arg is not nd");
	}

	Command c(Void);
	c.m_arg0.first = true;
	c.m_arg0.second = arg;
	return c;
}

Command Command::voiid_below()
{
	return voiid(Vec(0, -1, 0));
}

void Command::serialize(std::ostream& s) const
{
	switch(type())
	{
	case Halt:
		{
			const char a = 0xff;
			s.write(&a, 1);
			break;
		}

	case Flip:
		{
			const char a = 0xfd;
			s.write(&a, 1);
			break;
		}

	case SMove:
		{
			assert(m_arg0.first);
			assert(m_arg0.second.lld());

			unsigned a = 0;
			unsigned b = 0;

			if(m_arg0.second.x != 0)
			{
				a = 0x14;
				b = (0x1f & (m_arg0.second.x + 15));
			}
			else if(m_arg0.second.y != 0)
			{
				a = 0x24;
				b = (0x1f & (m_arg0.second.y + 15));
			}
			else if(m_arg0.second.z != 0)
			{
				a = 0x34;
				b = (0x1f & (m_arg0.second.z + 15));
			}

			s.write(reinterpret_cast<char*>(&a), 1);
			s.write(reinterpret_cast<char*>(&b), 1);

			break;
		}

	case Fill:
		{
			assert(m_arg0.first);
			assert(m_arg0.second.nd());

			unsigned a
				= 9 * (m_arg0.second.x + 1)
				+ 3 * (m_arg0.second.y + 1)
				+ 1 * (m_arg0.second.z + 1);

			a = (a << 3) | 0x3;

			s.write(reinterpret_cast<char*>(&a), 1);

			break;
		}

	case Void:
		{
			assert(m_arg0.first);
			assert(m_arg0.second.nd());

			unsigned a
				= 9 * (m_arg0.second.x + 1)
				+ 3 * (m_arg0.second.y + 1)
				+ 1 * (m_arg0.second.z + 1);

			a = (a << 3) | 0x2;

			s.write(reinterpret_cast<char*>(&a), 1);

			break;
		}

	default:
		assert(false);
	}
}

System::System(const Matrix& matrix)
: m_matrix(matrix)
, m_out_matrix(matrix.r())
{
	m_trace.reserve(5 * 1000 * 1000);
}

System::System(const System& src, const Matrix& matrix)
: System(matrix)
{
	assert(!src.m_out_matrix.calc_bounding_region().first);
	m_harmonics = src.m_harmonics;
	m_energy = src.m_energy;
	m_pos = src.m_pos;
	assert(!src.m_curr_command.first);
	m_trace = src.m_trace;
}

void System::serialize_trace(std::ostream& s)
{
	for(const auto& c : m_trace)
	{
		c.serialize(s);
	}
}

void System::push(Command command)
{
	// Only one command per step is supported.
	assert(!m_curr_command.first);
	m_curr_command.first = true;
	m_curr_command.second = command;
}

void System::step()
{
	assert(m_curr_command.first);

	const Command& command = m_curr_command.second;

	// Global field energy.
	if(m_harmonics == Harmonics::Low)
	{
		m_energy += (3 * m_matrix.r() * m_matrix.r() * m_matrix.r());
	}
	else
	{
		m_energy += (30 * m_matrix.r() * m_matrix.r() * m_matrix.r());
	}

	// Bots energy.
	m_energy += 20 * 1;

	// Commands.
	switch(command.type())
	{
	case Command::Halt:
		break;

	case Command::Flip:
		if(m_harmonics == Harmonics::Low)
		{
			m_harmonics = Harmonics::High;
		}
		else
		{
			m_harmonics = Harmonics::Low;
		}
		break;

	case Command::SMove:
		assert(command.arg0().first);
		assert(command.arg0().second.lld());

		m_pos = m_pos + command.arg0().second;
		m_energy += 2 * command.arg0().second.mlen();

		if(m_pos.x < 0
			|| m_pos.y < 0
			|| m_pos.z < 0)
		{
			std::ostringstream os;
			os << "Wrong position mat.R = " << m_matrix.r()
				<< ", pos = " << m_pos;
			throw std::runtime_error(os.str());
		}

		if(m_pos.x >= m_matrix.r()
			|| m_pos.y >= m_matrix.r()
			|| m_pos.z >= m_matrix.r())
		{
			std::ostringstream os;
			os << "Wrong position mat.R = " << m_matrix.r()
				<< ", pos = " << m_pos;
			throw std::runtime_error(os.str());
		}

		break;

	case Command::Fill:
		{
			assert(command.arg0().first);
			assert(command.arg0().second.nd());

			const Vec tgt = m_pos + command.arg0().second;

			if(m_out_matrix.voxel(tgt))
			{
				m_energy += 6;
			}
			else
			{
				m_out_matrix.set_voxel(tgt, true);
				m_energy += 12;
			}

			break;
		}

	case Command::Void:
		{
			assert(command.arg0().first);
			assert(command.arg0().second.nd());

			const Vec tgt = m_pos + command.arg0().second;

			if(m_out_matrix.voxel(tgt))
			{
				m_out_matrix.set_voxel(tgt, false);
				assert(m_energy >= 12);
				m_energy -= 12;
			}
			else
			{
				m_energy += 3;
			}

			break;
		}

	default:
		assert(false);
	}

	m_trace.push_back(m_curr_command.second);
	m_curr_command.first = false;
	m_curr_command.second = Command();
}

void System::push_and_step(Command command)
{
	push(command);
	step();
}

namespace {
const int max_step_len = 15;
} //

void System::move_to_x(int x)
{
	const int dx = x - m_pos.x;

	if(dx != 0)
	{
		const int sign = (dx > 0) ? 1 : -1;
		for(int i = 0; i < abs(dx / max_step_len); ++i)
		{
			push_and_step(Command::smove_x(max_step_len * sign));
		}

		const int rem = dx % max_step_len;
		if(rem != 0)
		{
			push_and_step(Command::smove_x(rem));
		}
	}
}

void System::move_to_y(int y)
{
	const int dy = y - m_pos.y;

	if(dy != 0)
	{
		const int sign = (dy > 0) ? 1 : -1;
		for(int i = 0; i < abs(dy / max_step_len); ++i)
		{
			push_and_step(Command::smove_y(max_step_len * sign));
		}

		const int rem = dy % max_step_len;
		if(rem != 0)
		{
			push_and_step(Command::smove_y(rem));
		}
	}
}

void System::move_to_z(int z)
{
	const int dz = z - m_pos.z;

	if(dz != 0)
	{
		const int sign = (dz > 0) ? 1 : -1;
		for(int i = 0; i < abs(dz / max_step_len); ++i)
		{
			push_and_step(Command::smove_z(max_step_len * sign));
		}

		const int rem = dz % max_step_len;
		if(rem != 0)
		{
			push_and_step(Command::smove_z(rem));
		}
	}
}

void System::move_to(const Vec& tgt, MovementOrder order)
{
	// No volatile points in the volume assumed.

	if(order == MovementOrder::XZY)
	{
		move_to_x(tgt.x);
		move_to_z(tgt.z);
		move_to_y(tgt.y);
	}
	else if(order == MovementOrder::YZX)
	{
		move_to_y(tgt.y);
		move_to_z(tgt.z);
		move_to_x(tgt.x);
	}
}

void Tracer::run()
{
	if(m_system.matrix().r() < 2)
	{
		throw std::runtime_error("Matrix too small for this algo");
	}

	const auto region = m_system.matrix().calc_bounding_region();
	if(!region.first)
	{
		throw std::runtime_error("Empty matrix ?");
	}

	m_bounding_region = region.second;

	assert(m_bounding_region.a.x > 0
		&& m_bounding_region.a.x < m_system.matrix().r() - 1);
	assert(m_bounding_region.a.y >= 0
		&& m_bounding_region.a.y < m_system.matrix().r() - 1);
	assert(m_bounding_region.a.z > 0
		&& m_bounding_region.a.z < m_system.matrix().r() - 1);

	assert(m_bounding_region.b.x > 0
		&& m_bounding_region.b.x < m_system.matrix().r() - 1);
	assert(m_bounding_region.b.y >= 0
		&& m_bounding_region.b.y < m_system.matrix().r() - 1);
	assert(m_bounding_region.b.z > 0
		&& m_bounding_region.b.z < m_system.matrix().r() - 1);
	
	// Move to the starting point. 

	const int initial_y = m_dir == Tracer::Direction::Up
		? 1 : (m_bounding_region.b.y + 1);

	Vec initial_pos(m_bounding_region.a.x, initial_y, m_bounding_region.a.z);
	m_system.move_to(initial_pos, System::MovementOrder::YZX);
	assert(m_system.bot_pos() == initial_pos);

	///////////////////

	m_system.push_and_step(Command::flip());

	// Iterate the matrix.

	for(int y = 1; y < m_bounding_region.b.y + 2; ++y)
	{
		scan_xz_plane(m_system.bot_pos().y - 1);

		if(y < m_bounding_region.b.y + 1)
		{
			m_system.push_and_step(Command::smove_y(
				(m_dir == Tracer::Direction::Up) ? 1 : -1));
		}
	}

	///////////////////

	assert(m_system.bot_pos().y == m_bounding_region.b.y + 1
		|| m_system.bot_pos().y == 1);

	///////////////////

	m_system.push_and_step(Command::flip());
}

void Tracer::halt()
{
	m_system.move_to(Vec());
	assert(m_system.bot_pos() == Vec());

	m_system.push_and_step(Command::halt());
}

void Tracer::scan_xz_plane(int y)
{
	assert(y >= 0);
	assert(m_system.bot_pos().y == y + 1
		&& "bot must be one level above");

	const auto curr_region
		= m_system.matrix().calc_bounding_region_y(y);

	if(curr_region.first)
	{
		const std::vector<Vec> vertices = {
			curr_region.second.a,
			Vec(curr_region.second.a.x, y, curr_region.second.b.z),
			curr_region.second.b,
			Vec(curr_region.second.b.x, y, curr_region.second.a.z)
		};

		const auto closest_vertex_it = std::min_element(
			begin(vertices), end(vertices),
			[&](const auto& a, const auto& b) {
				return (a - m_system.bot_pos()).mlen()
			    < (b - m_system.bot_pos()).mlen();
		});
		assert(closest_vertex_it != vertices.end());

		const auto furthest_vertex_it = vertices.begin()
			+ ((closest_vertex_it - vertices.begin()) + 2) % vertices.size();
		assert(furthest_vertex_it != vertices.end());

		m_system.move_to(
			closest_vertex_it->xz(m_system.bot_pos().y));
		
		const int x_sweep_dir
			= (closest_vertex_it->x < furthest_vertex_it->x) ? 1 : -1;
		int z_sweep_dir
			= (closest_vertex_it->z < furthest_vertex_it->z) ? 1 : -1;
		int src_z = closest_vertex_it->z;
		int tgt_z = furthest_vertex_it->z;

		for(int x = closest_vertex_it->x;
				x != furthest_vertex_it->x + x_sweep_dir;
				x += x_sweep_dir)
		{
			for(int z = src_z;
					z != tgt_z + z_sweep_dir;
					z += z_sweep_dir)
			{
				if(m_system.matrix().voxel(Vec(x, y, z)))
				{
					m_system.move_to(Vec(x, m_system.bot_pos().y, z));
					handle_voxel(Vec(x, y, z));
				}
			}
			std::swap(src_z, tgt_z);
			z_sweep_dir *= -1;
		}
	}
}

void Assembler::handle_voxel(const Vec& p)
{
	m_system.push_and_step(Command::fill_below());
}

void Disassembler::handle_voxel(const Vec& p)
{
	m_system.push_and_step(Command::voiid_below());
}

} //
