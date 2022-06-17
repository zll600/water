cmake_minimum_required(VERSION 3.22)

project(water)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CXX_FLAGS
    -g
    -rdynamic
    -std=c++11 
    -Wall
    -Werror
)
string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CXX_FLAGS}")

include(cmake/utils.cmake)
add_definitions(-Wno-builtin-macro-redefined)

add_subdirectory(water)
add_subdirectory(tests)
