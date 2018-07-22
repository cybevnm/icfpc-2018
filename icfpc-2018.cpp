#include "icfpc-2018.hpp"

namespace icfpc2018 {

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

	std::cerr << (int)(unsigned char)r << std::endl;

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
	c.arg0.first = true;
	c.arg0.second = arg;
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
	c.arg0.first = true;
	c.arg0.second = arg;
	return c;
}

Command Command::fill_below()
{
	return fill(Vec(0, -1, 0));
}

void Command::serialize(std::ostream& s) const
{
	switch(type)
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
			assert(arg0.first);
			assert(arg0.second.lld());

			unsigned a = 0;
			unsigned b = 0;

			if(arg0.second.x != 0)
			{
				a = 0x14;
				b = (0x1f & (arg0.second.x + 15));
			}
			else if(arg0.second.y != 0)
			{
				a = 0x24;
				b = (0x1f & (arg0.second.y + 15));
			}
			else if(arg0.second.z != 0)
			{
				a = 0x34;
				b = (0x1f & (arg0.second.z + 15));
			}

			s.write(reinterpret_cast<char*>(&a), 1);
			s.write(reinterpret_cast<char*>(&b), 1);

			break;
		}
	///////////////////////
	case Fill:
		{
			assert(arg0.first);
			assert(arg0.second.nd());

			unsigned a
				= 9 * (arg0.second.x + 1)
				+ 3 * (arg0.second.y + 1)
				+ 1 * (arg0.second.z + 1);

			a = (a << 3) | 0x3;

			s.write(reinterpret_cast<char*>(&a), 1);

			break;
		}
	///////////////////////
	}
}

void System::build_trace()
{
	Vec pos;

	if(matrix.R() < 2)
	{
		throw std::runtime_error("Matrix too small for this algo");
	}

	int x_sweep_dir = 1;
	int z_sweep_dir = 1;

	push_command(Command::smove_y(1)); //
	pos.y += 1;                        //

	push_command(Command::flip()); //

	for(int y = 0; y < matrix.R() - 2; ++y)
	{
		for(int x = 0; x < matrix.R() - 1; ++x)
		{
			for(int z = 0; z < matrix.R() - 1; ++z)
			{
				if(matrix.voxel(pos - Vec(0, 1, 0)))
				{
					push_command(Command::fill_below());            //
					out_matrix.set_voxel(pos - Vec(0, 1, 0), true); //
				}

				push_command(Command::smove_z(z_sweep_dir)); //
				pos.z += z_sweep_dir;                        //
			}

			if(z_sweep_dir == 1)
			{
				assert(pos.z == matrix.R() - 1);
			}
			else
			{
				assert(pos.z == 0);
			}

			push_command(Command::smove_x(x_sweep_dir)); //
			pos.x += x_sweep_dir;                        //

			z_sweep_dir *= -1;
		}

		if(x_sweep_dir == 1)
		{
			assert(pos.x == matrix.R() - 1);
		}
		else
		{
			assert(pos.x == 0);
		}

		push_command(Command::smove_y(1));  //
		pos.y += 1;                         //

		x_sweep_dir *= -1;
	}

	///////////////////

	std::cerr << "pos.y=" << pos.y << std::endl;

	assert(pos.y == matrix.R() - 1);
	assert(pos.x == 0 || pos.x == matrix.R() - 1);
	assert(pos.z == 0 || pos.z == matrix.R() - 1);

	///////////////////

	if(pos.x != 0)
	{
		const int steps_num = pos.x;
		for(int x = 0; x < steps_num; ++x)
		{
			push_command(Command::smove_x(-1)); //
			pos.x -= 1;                         //
		}
	}

	assert(pos.x == 0);

	///////////////////

	if(pos.z != 0)
	{
		const int steps_num = pos.z;
		for(int z = 0; z < steps_num; ++z)
		{
			push_command(Command::smove_z(-1)); //
			pos.z -= 1;                         //
		}

	}

	assert(pos.z == 0);

	///////////////////

	if(pos.y != 0)
	{
		const int steps_num = pos.y;
		for(int y = 0; y < steps_num; ++y)
		{
			push_command(Command::smove_y(-1)); //
			pos.y -= 1;                         //
		}
	}

	assert(pos.y == 0);

	///////////////////

	assert(pos == Vec(0,0,0));

	push_command(Command::flip()); //

	push_command(Command::halt()); //
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

void System::push_command(Command command)
{
	trace.push_back(command);
}

} //
