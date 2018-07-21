#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE tests

#include <iostream>
#include <fstream>
#include <stdexcept>

#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>

#include "icfpc-2018.hpp"

using namespace icfpc2018;

std::string path(const char* rel_path)
{
	assert(rel_path);
	namespace bf = boost::filesystem;
	return (bf::path(__FILE__).parent_path() / bf::path(rel_path)).native();
}

std::string read_full(const std::string& path)
{
	std::ifstream f(path.c_str());
	if(!f)
	{
		throw std::runtime_error("Error");
	}
	std::stringstream ss;
	ss << f.rdbuf();
	return ss.str();
}

BOOST_AUTO_TEST_CASE(Vec_Test)
{
	Vec v1;
}

BOOST_AUTO_TEST_CASE(Model_io_test)
{
	BOOST_CHECK_NO_THROW(
		read_model_file(path("ignore/problemsL/LA001_tgt.mdl")));

	{
		const Matrix m = read_model_file(path("ignore/problemsL/LA001_tgt.mdl"));
		BOOST_CHECK_NO_THROW(write_model_file(m, "/tmp/test001.mdl"));
		BOOST_CHECK_EQUAL(read_full(path("ignore/problemsL/LA001_tgt.mdl")),
			read_full("/tmp/test001.mdl"));
	}

	{
		const Matrix m = read_model_file(path("ignore/problemsL/LA186_tgt.mdl"));
		BOOST_CHECK_NO_THROW(write_model_file(m, "/tmp/test186.mdl"));
		BOOST_CHECK_EQUAL(read_full(path("ignore/problemsL/LA186_tgt.mdl")),
			read_full("/tmp/test186.mdl"));
	}
}
