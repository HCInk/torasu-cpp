cmake_policy(PUSH)

cmake_minimum_required (VERSION 2.6)

if(NOT TARGET torasustd)

	project (torasu-cpp-std)
	
	include(../core/CMakeLists.txt)
	
	add_library(torasustd SHARED ${CMAKE_CURRENT_LIST_DIR}/torasu-cpp-std.cpp)

endif()

cmake_policy(POP)