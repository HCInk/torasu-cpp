cmake_policy(PUSH)

cmake_minimum_required (VERSION 2.6)

if(NOT TARGET torasu)

	project (torasu-cpp-core)
	
	set(TORASU_CORE_SRC "${CMAKE_CURRENT_LIST_DIR}/src/torasu-cpp-core.cpp")
	
	add_library(torasu SHARED ${TORASU_CORE_SRC})
	
endif()

cmake_policy(POP)