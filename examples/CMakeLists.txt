cmake_policy(PUSH)

cmake_minimum_required (VERSION 2.6)

if(NOT TARGET torasu-cpp-examples)

	# Meta / Settings
	
	project (torasu-cpp-examples)
	set(CMAKE_CXX_STANDARD 20)

	# Packages
	
	find_package(nlohmann_json 3.2.0 REQUIRED)

	# TORASU-CORE (local)
	include(${CMAKE_CURRENT_LIST_DIR}/../core/CMakeLists.txt)
	# TORASU-STD (local)
	include(${CMAKE_CURRENT_LIST_DIR}/../std/CMakeLists.txt)

	# Compile
	
	set(TORASU_EXAMPLES_SRC 
		${CMAKE_CURRENT_LIST_DIR}/src/torasu-cpp-examples.cpp)
	add_executable(torasu-cpp-examples ${TORASU_EXAMPLES_SRC})

	# Link libraries 

	if(APPLE)
	else()
		target_link_libraries(torasu-cpp-examples asan)
	endif()

	target_link_libraries(torasu-cpp-examples torasu torasustd nlohmann_json::nlohmann_json)

endif()

cmake_policy(POP)