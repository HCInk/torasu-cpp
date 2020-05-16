cmake_policy(PUSH)

cmake_minimum_required (VERSION 3.0)

if(NOT TARGET torasustd)
	
	# Meta / Settings

	project(torasu-cpp-std VERSION 0.0.4)
	set(CMAKE_CXX_STANDARD 20)

	# TORASU-CORE (local)
	include(${CMAKE_CURRENT_LIST_DIR}/../core/CMakeLists.txt)

	# Compile

	set(TORASU_STD_SRC 
		${CMAKE_CURRENT_LIST_DIR}/src/torasu-cpp-std.cpp
		${CMAKE_CURRENT_LIST_DIR}/src/Dnum.cpp
		${CMAKE_CURRENT_LIST_DIR}/src/Dstring.cpp
		${CMAKE_CURRENT_LIST_DIR}/src/Dbimg.cpp
		${CMAKE_CURRENT_LIST_DIR}/src/Dfile.cpp
		${CMAKE_CURRENT_LIST_DIR}/src/Rnum.cpp
		${CMAKE_CURRENT_LIST_DIR}/src/Rmultiply.cpp
		${CMAKE_CURRENT_LIST_DIR}/src/Rlocal_file.cpp
		${CMAKE_CURRENT_LIST_DIR}/src/EIcore_runner.cpp)
	add_library(torasustd SHARED ${TORASU_STD_SRC})

	set_target_properties(torasustd PROPERTIES LINK_FLAGS "-Wl,-rpath,./")

	# Includes for current target

	target_include_directories(torasustd PUBLIC lib/json/single_include/)

	# Libraries for current target

	if(APPLE)

	else()
		if(TORASU_DEBUG_ASAN)
			target_link_libraries(torasustd asan)
		endif()
	endif()

	target_link_libraries(torasustd torasu)
	
	# Installation

	install(TARGETS torasustd DESTINATION lib)
	install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/include/torasu DESTINATION include)

	# Includes for others

	include_directories(BEFORE ${CMAKE_CURRENT_LIST_DIR}/include)

endif()

cmake_policy(POP)
