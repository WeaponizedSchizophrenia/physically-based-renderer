cmake_minimum_required(VERSION 3.31)
project(PhysicallyBasedRenderer CXX)

include(cmake/CPM.cmake)
include(cmake/Dependencies.cmake)

enable_testing()

add_subdirectory(third_party)
add_subdirectory(src)
add_subdirectory(tests)
