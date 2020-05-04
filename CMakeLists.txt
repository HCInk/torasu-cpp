
cmake_minimum_required (VERSION 2.6)

project (torasu-cpp)

include(core/CMakeLists.txt)
include(std/CMakeLists.txt)
include(examples/CMakeLists.txt)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall")