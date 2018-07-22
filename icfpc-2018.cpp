#include "icfpc-2018.hpp"
#include <limits>
#include <algorithm>
#include <sstream>

namespace icfpc2018 {

std::pair<bool, Region> Matrix::calc_bounding_region() const
{
	const int max = std::numeric_limits<int>::max();
	const int min = std::numeric_limits<int>::min();

	const Vec vmax(max, max, max);
	const Vec vmin(min, min, min);

	Vec a = vmax;
	Vec b = vmin;

	for(int x = 0; x < R(); ++x)
	{
		for(int y = 0; y < R(); ++y)
		{
			for(int z = 0; z < R(); ++z)
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

	for(int x = 0; x < R(); ++x)
	{
		for(int z = 0; z < R(); ++z)
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
	for(size_t y = 0; y < R(); ++y)
	{
		for(size_t x = 0; x < R(); ++x)
		{
			for(size_t z = 0; z < R(); ++z)
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

	unsigned char r = m.R();
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
	c.a0.first = true;
	c.a0.second = arg;
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
	c.a0.first = true;
	c.a0.second = arg;
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
	c.a0.first = true;
	c.a0.second = arg;
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
	///////////////////////
	case Halt:
		{
			const char a = 0xff;
			s.write(&a, 1);

			break;
		}
	///////////////////////
	case Flip:
		{
			const char a = 0xfd;
			s.write(&a, 1);

			break;
		}
	///////////////////////
	case SMove:
		{
			assert(a0.first);
			assert(a0.second.lld());

			unsigned a = 0;
			unsigned b = 0;

			if(a0.second.x != 0)
			{
				a = 0x14;
				b = (0x1f & (a0.second.x + 15));
			}
			else if(a0.second.y != 0)
			{
				a = 0x24;
				b = (0x1f & (a0.second.y + 15));
			}
			else if(a0.second.z != 0)
			{
				a = 0x34;
				b = (0x1f & (a0.second.z + 15));
			}

			s.write(reinterpret_cast<char*>(&a), 1);
			s.write(reinterpret_cast<char*>(&b), 1);

			break;
		}
	///////////////////////
	case Fill:
		{
			assert(a0.first);
			assert(a0.second.nd());

			unsigned a
				= 9 * (a0.second.x + 1)
				+ 3 * (a0.second.y + 1)
				+ 1 * (a0.second.z + 1);

			a = (a << 3) | 0x3;

			s.write(reinterpret_cast<char*>(&a), 1);

			break;
		}
	///////////////////////
	case Void:
		{
			assert(a0.first);
			assert(a0.second.nd());

			unsigned a
				= 9 * (a0.second.x + 1)
				+ 3 * (a0.second.y + 1)
				+ 1 * (a0.second.z + 1);

			a = (a << 3) | 0x2;

			s.write(reinterpret_cast<char*>(&a), 1);

			break;
		}

	///////////////////////
	default:
		assert(false);
	}
}

void System::serialize_trace(std::ostream& s)
{
	for(const auto& c : trace)
	{
		c.serialize(s);
	}
}

void System::push(Command command)
{
	// Only one command per step supported.
	assert(!curr_command.first);
	curr_command.first = true;
	curr_command.second = command;
}

void System::step()
{
	assert(curr_command.first);

	const Command& command = curr_command.second;

	// Global field energy.
	if(harmonics == Harmonics::Low)
	{
		e += (3 * mat.R() * mat.R() * mat.R());
	}
	else
	{
		e += (30 * mat.R() * mat.R() * mat.R());
	}

	// Bots energy.
	e += 20 * 1;

	// Commands.
	switch(command.type())
	{
	case Command::Halt:
		break;

	case Command::Flip:
		if(harmonics == Harmonics::Low)
		{
			harmonics = Harmonics::High;
		}
		else
		{
			harmonics = Harmonics::Low;
		}
		break;

	case Command::SMove:
		assert(command.arg0().first);
		assert(command.arg0().second.lld());

		pos = pos + command.arg0().second;
		e += 2 * command.arg0().second.mlen();

		if(pos.x < 0 || pos.y < 0 || pos.z < 0)
		{
			std::ostringstream os;
			os << "Wrong position (negative), mat.R = " << mat.R()
				<< ", pos = " << pos;
			throw std::runtime_error(os.str());
		}

		if(pos.x >= mat.R() || pos.y >= mat.R() || pos.z >= mat.R())
		{
			std::ostringstream os;
			os << "Wrong position (positive), mat.R = " << mat.R()
				<< ", pos = " << pos;
			throw std::runtime_error(os.str());
		}

		break;

	case Command::Fill:
		{
			assert(command.arg0().first);
			assert(command.arg0().second.nd());

			const Vec tgt = pos + command.arg0().second;

			if(out_mat.voxel(tgt))
			{
				e += 6;
			}
			else
			{
				out_mat.set_voxel(tgt, true);
				e += 12;
			}

			break;
		}

	case Command::Void:
		{
			assert(command.arg0().first);
			assert(command.arg0().second.nd());

			const Vec tgt = pos + command.arg0().second;

			if(out_mat.voxel(tgt))
			{
				out_mat.set_voxel(tgt, false);
				assert(e >= 12);
				e -= 12;
			}
			else
			{
				e += 3;
			}

			break;
		}


	default:
		assert(false);
	}

	trace.push_back(curr_command.second);
	curr_command.first = false;
	curr_command.second = Command();
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
	const int dx = x - pos.x;

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
	const int dy = y - pos.y;

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
	const int dz = z - pos.z;

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
	if(s.matrix().R() < 2)
	{
		throw std::runtime_error("Matrix too small for this algo");
	}

	const std::pair<bool, Region> region = s.matrix().calc_bounding_region();
	if(!region.first)
	{
		throw std::runtime_error("Empty matrix ?");
	}

	bounding_region = region.second;

	assert(bounding_region.a.x > 0
		&& bounding_region.a.x < s.matrix().R() - 1);
	assert(bounding_region.a.y >= 0
		&& bounding_region.a.y < s.matrix().R() - 1);
	assert(bounding_region.a.z > 0
		&& bounding_region.a.z < s.matrix().R() - 1);

	assert(bounding_region.b.x > 0
		&& bounding_region.b.x < s.matrix().R() - 1);
	assert(bounding_region.b.y >= 0
		&& bounding_region.b.y < s.matrix().R() - 1);
	assert(bounding_region.b.z > 0
		&& bounding_region.b.z < s.matrix().R() - 1);
	
	///////////////////
	// Move to the starting point. 

	const int initial_y = dir == Tracer::Direction::Up
		? 1 : (bounding_region.b.y + 1);

	Vec initial_pos(bounding_region.a.x, initial_y, bounding_region.a.z);
	s.move_to(initial_pos, System::MovementOrder::YZX);
	assert(s.bot_pos() == initial_pos);

	///////////////////

	s.push_and_step(Command::flip());

	///////////////////
	// Iterate the matrix.

	for(int y = 1; y < bounding_region.b.y + 2; ++y)
	{
		scan_xz_plane(s.bot_pos().y - 1);

		if(y < bounding_region.b.y + 1)
		{
			s.push_and_step(Command::smove_y(
				(dir == Tracer::Direction::Up) ? 1 : -1));
		}
	}

	///////////////////

	assert(s.bot_pos().y == bounding_region.b.y + 1
		|| s.bot_pos().y == 1);

	///////////////////

	s.push_and_step(Command::flip());
}

void Tracer::halt()
{
	s.move_to(Vec());
	assert(s.bot_pos() == Vec());

	s.push_and_step(Command::halt());
}

void Tracer::scan_xz_plane(int y)
{
	assert(y >= 0);
	assert(s.bot_pos().y == y + 1 && "bot must be one level above");

	const std::pair<bool, Region> curr_region
		= s.matrix().calc_bounding_region_y(y);

	if(curr_region.first)
	{
		const std::vector<Vec> corners = {
			curr_region.second.a,
			Vec(curr_region.second.a.x, y, curr_region.second.b.z),
			curr_region.second.b,
			Vec(curr_region.second.b.x, y, curr_region.second.a.z)
		};

		const auto closest_corner_it = std::min_element(
			begin(corners), end(corners),
			[&](const auto& a, const auto& b) {
				return (a - s.bot_pos()).mlen() < (b - s.bot_pos()).mlen();
		});

		assert(closest_corner_it != corners.end());

		int x_sweep_dir = 0;
		int z_sweep_dir = 0;
		
		if(closest_corner_it->x == curr_region.second.a.x)
		{
			x_sweep_dir = 1;
		}
		else
		{
			x_sweep_dir = -1;
		}

		if(closest_corner_it->z == curr_region.second.a.z)
		{
			z_sweep_dir = 1;
		}
		else
		{
			z_sweep_dir = -1;
		}

		s.move_to(closest_corner_it->xz(s.bot_pos().y));

		for(int x = curr_region.second.a.x;
				x <= curr_region.second.b.x; ++x)
		{
			for(int z = curr_region.second.a.z;
					z <= curr_region.second.b.z; ++z)
			{
				if(s.matrix().voxel(Vec(x, y, z)))
				{
					s.move_to(Vec(x, s.bot_pos().y, z));
					handle_voxel(Vec(x, y, z));
				}
			}
		}
	}
}

void Assembler::handle_voxel(const Vec& p)
{
	s.push_and_step(Command::fill_below());
}

void Disassembler::handle_voxel(const Vec& p)
{
	s.push_and_step(Command::voiid_below());
}

} //
