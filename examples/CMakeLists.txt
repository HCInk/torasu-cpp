cmake_policy(PUSH)

cmake_minimum_required (VERSION 2.6)


if(NOT TARGET torasu-cpp-examples)

	project (torasu-cpp-examples)
	find_package(nlohmann_json 3.2.0 REQUIRED)

	include(${CMAKE_CURRENT_LIST_DIR}/../core/CMakeLists.txt)

	include(${CMAKE_CURRENT_LIST_DIR}/../std/CMakeLists.txt)

	set(CMAKE_CXX_STANDARD 20)
		
	set(TORASU_EXAMPLES_SRC 
		${CMAKE_CURRENT_LIST_DIR}/src/torasu-cpp-examples.cpp)
	add_executable(torasu-cpp-examples ${TORASU_EXAMPLES_SRC})
	
	target_link_libraries(torasu-cpp-examples PRIVATE nlohmann_json::nlohmann_json)

	target_link_libraries(torasu-cpp-examples asan torasu torasustd)

endif()

cmake_policy(POP)