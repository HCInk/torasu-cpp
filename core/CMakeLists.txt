cmake_policy(PUSH)

cmake_minimum_required (VERSION 2.6)

if(NOT TARGET torasu)

	# Meta / Settings

	project (torasu-cpp-core)
	set(CMAKE_CXX_STANDARD 20)

	# Packages

	if (APPLE)
		find_package(nlohmann_json 3.2.0 REQUIRED)
	endif()

	# Compile

	set(TORASU_CORE_SRC
		${CMAKE_CURRENT_LIST_DIR}/src/torasu-cpp-core.cpp
		${CMAKE_CURRENT_LIST_DIR}/src/DataPackable.cpp
		${CMAKE_CURRENT_LIST_DIR}/src/RenderTools.cpp)
	add_library(torasu SHARED ${TORASU_CORE_SRC})

	# Link libraries

	if(APPLE)
		target_link_libraries(torasu nlohmann_json::nlohmann_json)
	else()
		target_link_libraries(torasu asan)
	endif()

	# Includes for others

	include_directories(${CMAKE_CURRENT_LIST_DIR}/include)

endif()

cmake_policy(POP)
