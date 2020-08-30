
cmake_minimum_required (VERSION 3.0)

project(torasu-cpp VERSION 0.0.4)
set(CMAKE_CXX_STANDARD 20)

if (NOT MINGW AND NOT APPLE)
	set(TORASU_DEBUG_ASAN true)
endif()

include(core/CMakeLists.txt)
include(std/CMakeLists.txt)
include(examples/CMakeLists.txt)

#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall")
#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall")

