cmake_policy(PUSH)

cmake_minimum_required (VERSION 3.0)

if(NOT TARGET torasu)

	# Meta / Settings

	project(torasu-cpp-core VERSION 0.0.4)
	set(CMAKE_CXX_STANDARD 20)

	# Compile

	set(TORASU_CORE_SRC
		${CMAKE_CURRENT_LIST_DIR}/src/torasu-cpp-core.cpp
		${CMAKE_CURRENT_LIST_DIR}/src/DataPackable.cpp
		${CMAKE_CURRENT_LIST_DIR}/src/RenderTools.cpp)
	add_library(torasu SHARED ${TORASU_CORE_SRC})

	# Includes for current target

	target_include_directories(torasu PUBLIC lib/json/single_include/)

	# Libraries for current target

	if(APPLE)
	
	else()
		if(TORASU_DEBUG_ASAN)
			target_link_libraries(torasu asan)
		endif()
	endif()

	# Includes for others

	include_directories(${CMAKE_CURRENT_LIST_DIR}/include)

endif()

cmake_policy(POP)
