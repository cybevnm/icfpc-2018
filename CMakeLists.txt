cmake_minimum_required(VERSION 3.0)

project(icfpc-2018)

find_package(Boost
	COMPONENTS system filesystem unit_test_framework REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++14")

add_executable(trace icfpc-2018.cpp trace.cpp)

add_executable(tests icfpc-2018.cpp tests.cpp)

target_link_libraries(tests
	${Boost_SYSTEM_LIBRARY}
	${Boost_FILESYSTEM_LIBRARY}
  ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})

