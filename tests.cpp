// ICFPC2018 solution code chunks.
// Copyright (C) 2018 cybevnm

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

BOOST_TEST_DONT_PRINT_LOG_VALUE(Vec);

BOOST_AUTO_TEST_CASE(Model_io_test)
{
	BOOST_CHECK_NO_THROW(read_model_file(path("tests/FA001_tgt.mdl")));
	
	{
		const Matrix m = read_model_file(path("tests/FA001_tgt.mdl"));
		BOOST_CHECK_NO_THROW(write_model_file(m, "/tmp/test001.mdl"));
		BOOST_CHECK_EQUAL(read_full(path("tests/FA001_tgt.mdl")),
			read_full("/tmp/test001.mdl"));
	}
	
	{
		const Matrix m = read_model_file(path("tests/FA186_tgt.mdl"));
		BOOST_CHECK_NO_THROW(write_model_file(m, "/tmp/test186.mdl"));
		BOOST_CHECK_EQUAL(read_full(path("tests/FA186_tgt.mdl")),
			read_full("/tmp/test186.mdl"));
	}
}

BOOST_AUTO_TEST_CASE(System_move_to_test)
{
	Matrix m(100);
	System s(m);

	BOOST_CHECK_EQUAL(Vec(), s.bot_pos());
	s.move_to(Vec(5,5,5));
	BOOST_CHECK_EQUAL(Vec(5,5,5), s.bot_pos());
	s.move_to(Vec(20,20,20));
	BOOST_CHECK_EQUAL(Vec(20,20,20), s.bot_pos());
	s.move_to(Vec(0,0,0));
	BOOST_CHECK_EQUAL(Vec(), s.bot_pos());
}
