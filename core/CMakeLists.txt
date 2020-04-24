cmake_policy(PUSH)

cmake_minimum_required (VERSION 2.6)

if(NOT TARGET torasu)

	project (torasu-cpp-core)
	
    set(CMAKE_CXX_STANDARD 20)

	set(TORASU_CORE_SRC 
		${CMAKE_CURRENT_LIST_DIR}/src/torasu-cpp-core.cpp
		${CMAKE_CURRENT_LIST_DIR}/src/DataPackable.cpp)
	
	add_library(torasu SHARED ${TORASU_CORE_SRC})
	
	target_link_libraries(torasu asan json-c)

	include_directories(${CMAKE_CURRENT_LIST_DIR}/include)
	
endif()

cmake_policy(POP)
