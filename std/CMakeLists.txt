cmake_policy(PUSH)

cmake_minimum_required (VERSION 2.6)


if(NOT TARGET torasustd)

	project (torasu-cpp-std)

	include(${CMAKE_CURRENT_LIST_DIR}/../core/CMakeLists.txt)
	
	set(CMAKE_CXX_STANDARD 20)
	
	set(TORASU_STD_SRC 
		${CMAKE_CURRENT_LIST_DIR}/src/torasu-cpp-std.cpp)
	
	add_library(torasustd SHARED ${TORASU_STD_SRC})
	
	target_link_libraries(torasustd asan torasu)
	
	include_directories(${CMAKE_CURRENT_LIST_DIR}/include)

endif()

cmake_policy(POP)
