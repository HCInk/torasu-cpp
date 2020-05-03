cmake_policy(PUSH)

cmake_minimum_required (VERSION 2.6)

if(NOT TARGET torasu-cpp-examples)

	# Meta / Settings
	
	project (torasu-cpp-examples)
	set(CMAKE_CXX_STANDARD 20)

	# Packages
	
	if (APPLE)
		find_package(nlohmann_json 3.2.0 REQUIRED)
	endif()

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
		target_link_libraries(torasu-cpp-examples nlohmann_json::nlohmann_json)
	else()
		target_link_libraries(torasu-cpp-examples asan)
	endif()

	target_link_libraries(torasu-cpp-examples torasu torasustd)

	# Tests

	find_package(Catch2)
	if (Catch2_FOUND)
		message(STATUS "Found Catch2-cmake-package: Enabling tests.")

		set(TORASU_TEST_SRC 
			${CMAKE_CURRENT_LIST_DIR}/test/torasu-cpp-test.cpp)
		add_executable(torasu-cpp-test ${TORASU_TEST_SRC})


		if(APPLE)	
			target_link_libraries(torasu-cpp-test nlohmann_json::nlohmann_json)
		else()
			target_link_libraries(torasu-cpp-test asan)
		endif()

		target_link_libraries(torasu-cpp-test torasu torasustd)

		target_link_libraries(torasu-cpp-test Catch2::Catch2)

		include(CTest)
		include(Catch)
		catch_discover_tests(torasu-cpp-test)

	else()
		message(WARNING "Found Catch2-cmake-package not found: Disabling tests.")
	endif()

endif()

cmake_policy(POP)
