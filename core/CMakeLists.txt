cmake_policy(PUSH)

cmake_minimum_required (VERSION 2.6)

if(NOT TARGET torasu)

	project (torasu-cpp-core)
	
	add_library(torasu SHARED ${CMAKE_CURRENT_LIST_DIR}/torasu-cpp-core.cpp)
	
endif()

cmake_policy(POP)