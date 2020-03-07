cmake_policy(PUSH)

cmake_minimum_required (VERSION 2.6)

if(NOT TARGET torasustd)

	project (torasu-cpp-std)
	
	include(../core/CMakeLists.txt)
	
	set(TORASU_STD_SRC 
		${CMAKE_CURRENT_LIST_DIR}/src/torasu-cpp-std.cpp)
	
	add_library(torasustd SHARED ${TORASU_STD_SRC})

endif()

cmake_policy(POP)