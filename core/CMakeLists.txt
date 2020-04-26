cmake_policy(PUSH)

cmake_minimum_required (VERSION 2.6)

if(NOT TARGET torasu)

	project (torasu-cpp-core)
	
    set(CMAKE_CXX_STANDARD 20)
	find_package(nlohmann_json 3.2.0 REQUIRED)

	set(TORASU_CORE_SRC 
		${CMAKE_CURRENT_LIST_DIR}/src/torasu-cpp-core.cpp
		${CMAKE_CURRENT_LIST_DIR}/src/DataPackable.cpp)
	add_library(torasu SHARED ${TORASU_CORE_SRC})
	target_link_libraries(torasu PRIVATE nlohmann_json::nlohmann_json)
	if(APPLE)
	else()
	target_link_libraries(torasu asan)
	endif()
	include_directories(${CMAKE_CURRENT_LIST_DIR}/include)
	
endif()

cmake_policy(POP)
