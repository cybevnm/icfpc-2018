#include "icfpc-2018.hpp"
#include <limits>

namespace icfpc2018 {

Region Matrix::calc_bounding_region() const
{
	const int max = std::numeric_limits<int>::max();
	const int min = std::numeric_limits<int>::min();

	Vec a(max, max, max);
	Vec b(min, min, min);

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

	Vec c = b - a;
	if(c.x >= 0 && c.y >= 0 && c.z >= 0)
	{
		return Region(a, b);
	}
	else
	{
		throw std::runtime_error("Empty matrix ?");
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

	std::vector<char> data((r * r * r) / 8);
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

	std::vector<char> data((r * r * r) / 8);

	for(int x = 0; x < r; ++x)
	{
		for(int y = 0; y < r; ++y)
		{
			for(int z = 0; z < r; ++z)
			{
				const size_t i = x * r * r + y * r + z;
				char& byte = data[i / 8];
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
		throw std::runtime_error("smove: fill is not nd");
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
	default:
		assert(false);
	}
}

void System::build_trace()
{
	if(matrix.R() < 2)
	{
		throw std::runtime_error("Matrix too small for this algo");
	}

	const Region bounding_region = matrix.calc_bounding_region();

	assert(bounding_region.a.x > 0 && bounding_region.a.x < matrix.R() - 1);
	assert(bounding_region.a.y >= 0 && bounding_region.a.y < matrix.R() - 1);
	assert(bounding_region.a.z > 0 && bounding_region.a.z < matrix.R() - 1);

	assert(bounding_region.b.x > 0 && bounding_region.b.x < matrix.R() - 1);
	assert(bounding_region.b.y >= 0 && bounding_region.b.y < matrix.R() - 1);
	assert(bounding_region.b.z > 0 && bounding_region.b.z < matrix.R() - 1);

	int x_sweep_dir = 1;
	int z_sweep_dir = 1;

	push_and_step(Command::smove_y(1));

	push_and_step(Command::flip());

	// Move to the starting point.
	for(int x = 0; x < bounding_region.a.x; ++x)
	{
		push_and_step(Command::smove_x(1));
	}

	for(int z = 0; z < bounding_region.a.z; ++z)
	{
		push_and_step(Command::smove_z(1));
	}

	assert(pos.x == bounding_region.a.x);
	assert(pos.z == bounding_region.a.z);

	// Build the model.
	for(int y = 1; y < bounding_region.b.y + 2; ++y)
	{
		for(int x = bounding_region.a.x;
				x <= bounding_region.b.x; ++x)
		{
			for(int z = bounding_region.a.z;
					z <= bounding_region.b.z; ++z)
			{
				if(matrix.voxel(pos - Vec(0, 1, 0)))
				{
					push_and_step(Command::fill_below());
				}

				if(z != bounding_region.b.z)
				{
					push_and_step(Command::smove_z(z_sweep_dir));
				}
			}

			assert(pos.z == bounding_region.a.z
				|| pos.z == bounding_region.b.z);

			if(x != bounding_region.b.x)
			{
				push_and_step(Command::smove_x(x_sweep_dir));
			}

			z_sweep_dir *= -1;
		}

		assert(pos.x == bounding_region.a.x
			|| pos.x == bounding_region.b.x);

		if(y < bounding_region.b.y + 1)
		{
			push_and_step(Command::smove_y(1));
		}

		x_sweep_dir *= -1;
	}

	///////////////////

	assert(pos.y == bounding_region.b.y + 1);
	assert(pos.x == bounding_region.a.x
		|| pos.x == bounding_region.b.x);
	assert(pos.z == bounding_region.a.z
		|| pos.z == bounding_region.b.z);

	///////////////////

	push_and_step(Command::flip());

	///////////////////

	if(pos.x != 0)
	{
		const int steps_num = pos.x;
		for(int x = 0; x < steps_num; ++x)
		{
			push_and_step(Command::smove_x(-1));
		}
	}

	assert(pos.x == 0);

	///////////////////

	if(pos.z != 0)
	{
		const int steps_num = pos.z;
		for(int z = 0; z < steps_num; ++z)
		{
			push_and_step(Command::smove_z(-1));
		}

	}

	assert(pos.z == 0);

	///////////////////

	if(pos.y != 0)
	{
		const int steps_num = pos.y;
		for(int y = 0; y < steps_num; ++y)
		{
			push_and_step(Command::smove_y(-1));
		}
	}

	assert(pos.y == 0);

	///////////////////

	assert(pos == Vec(0,0,0));

	push_and_step(Command::halt());
}

void System::serialize_trace(std::ostream& s)
{
	for(const auto& c : trace)
	{
		c.serialize(s);
	}
}

const Matrix& System::result_matrix() const
{
	return out_matrix;
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
		e += (3 * matrix.R() * matrix.R() * matrix.R());
	}
	else
	{
		e += (30 * matrix.R() * matrix.R() * matrix.R());
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
			throw std::runtime_error("Wrong position (negative)");
		}

		if(pos.x >= matrix.R() || pos.y >= matrix.R() || pos.z >= matrix.R())
		{
			throw std::runtime_error("Wrong position (positive)");
		}

		break;

	case Command::Fill:
		{
			assert(command.arg0().first);
			assert(command.arg0().second.nd());

			const Vec tgt = pos + command.arg0().second;

			if(out_matrix.voxel(tgt))
			{
				e += 6;
			}
			else
			{
				out_matrix.set_voxel(tgt, true);
				e += 12;
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

} //
