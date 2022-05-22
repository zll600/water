cmake_minimum_required(VERSION 3.22)

project(water)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic  -std=c++11  -Wall -Werror")

include(cmake/utils.cmake)
add_definitions(-Wno-builtin-macro-redefined)

add_subdirectory(water)
add_subdirectory(tests)
