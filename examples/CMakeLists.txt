cmake_policy(PUSH)

cmake_minimum_required (VERSION 3.0)

if(NOT TARGET torasu-cpp-examples)

	# Meta / Settings
	
	project(torasu-cpp-examples VERSION 0.0.4)
	set(CMAKE_CXX_STANDARD 20)

	# TORASU-CORE (local)
	include(${CMAKE_CURRENT_LIST_DIR}/../core/CMakeLists.txt)
	# TORASU-STD (local)
	include(${CMAKE_CURRENT_LIST_DIR}/../std/CMakeLists.txt)

	# Compile
	
	set(TORASU_EXAMPLES_SRC 
		${CMAKE_CURRENT_LIST_DIR}/src/torasu-cpp-examples.cpp
		${CMAKE_CURRENT_LIST_DIR}/src/task-distribution-test.cpp
		${CMAKE_CURRENT_LIST_DIR}/boilerplate/Dboilerplate.cpp
		${CMAKE_CURRENT_LIST_DIR}/boilerplate/Rboilerplate.cpp)
	add_executable(torasu-cpp-examples ${TORASU_EXAMPLES_SRC})
	
	set_target_properties(torasu-cpp-examples PROPERTIES LINK_FLAGS "-Wl,-rpath,./")

	# Includes for current target

	target_include_directories(torasu-cpp-examples PUBLIC lib/json/single_include/)

	# Libraries for current target

	if(APPLE)

	else()
		if(TORASU_DEBUG_ASAN)
			target_link_libraries(torasu-cpp-examples asan)
		endif()
	endif()

	target_link_libraries(torasu-cpp-examples torasu torasustd)

	# Tests

	add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../lib/Catch2)
	
	set(TORASU_TEST_SRC 
 	${CMAKE_CURRENT_LIST_DIR}/test/torasu-cpp-test.cpp)
	add_executable(torasu-cpp-test ${TORASU_TEST_SRC})

	set_target_properties(torasu-cpp-test PROPERTIES LINK_FLAGS "-Wl,-rpath,./")

	if(APPLE)

	else()
		if(TORASU_DEBUG_ASAN)
			target_link_libraries(torasu-cpp-test asan)
		endif()
	endif()
	

	target_link_libraries(torasu-cpp-test torasu torasustd)

	target_link_libraries(torasu-cpp-test Catch2::Catch2)

	include(CTest)
	include(lib/Catch2/contrib/Catch.cmake)
	catch_discover_tests(torasu-cpp-test)

endif()

cmake_policy(POP)
