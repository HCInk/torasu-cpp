cmake_policy(PUSH)

cmake_minimum_required (VERSION 2.6)

if(NOT TARGET torasustd)
	
	# Meta / Settings

	project (torasu-cpp-std)
	set(CMAKE_CXX_STANDARD 20)
	
	# Packages

	if (APPLE)
		find_package(nlohmann_json 3.2.0 REQUIRED)
	endif()

	# TORASU-CORE (local)
	include(${CMAKE_CURRENT_LIST_DIR}/../core/CMakeLists.txt)

	# Compile

	set(TORASU_STD_SRC 
		${CMAKE_CURRENT_LIST_DIR}/src/torasu-cpp-std.cpp)
	add_library(torasustd SHARED ${TORASU_STD_SRC})

	# Link libraries 

	if(APPLE)
		target_link_libraries(torasustd nlohmann_json::nlohmann_json)
	else()
		target_link_libraries(torasustd asan)
	endif()

	target_link_libraries(torasustd torasu)
	
	# Includes for others

	include_directories(${CMAKE_CURRENT_LIST_DIR}/include)

endif()

cmake_policy(POP)
