cmake_policy(PUSH)

cmake_minimum_required (VERSION 3.0)
message(${CMAKE_TOOLCHAIN_FILE})
if(NOT TARGET torasustd)
	# Meta / Settings
	if(EXPORT_TORASUCORE)
		message("LOLOLO")
	endif()
	project(torasu-cpp-std VERSION 0.0.4)
	set(CMAKE_CXX_STANDARD 20)

	# TORASU-CORE (local)
	include(${CMAKE_CURRENT_LIST_DIR}/../core/CMakeLists.txt)

	# Includes provided by this
	include_directories(BEFORE ${CMAKE_CURRENT_LIST_DIR}/include)

	# Compile

	set(TORASU_STD_SRC
		${CMAKE_CURRENT_LIST_DIR}/src/torasu-cpp-std.cpp
		${CMAKE_CURRENT_LIST_DIR}/src/Dnum.cpp
		${CMAKE_CURRENT_LIST_DIR}/src/Dstring.cpp
		${CMAKE_CURRENT_LIST_DIR}/src/Dbimg.cpp
		${CMAKE_CURRENT_LIST_DIR}/src/Dbimg_sequence.cpp
		${CMAKE_CURRENT_LIST_DIR}/src/Dfile.cpp
		${CMAKE_CURRENT_LIST_DIR}/src/Rnum.cpp
		${CMAKE_CURRENT_LIST_DIR}/src/Rmultiply.cpp
		${CMAKE_CURRENT_LIST_DIR}/src/Radd.cpp
		${CMAKE_CURRENT_LIST_DIR}/src/Rsubtract.cpp
		${CMAKE_CURRENT_LIST_DIR}/src/Rlocal_file.cpp
		${CMAKE_CURRENT_LIST_DIR}/src/Rnet_file.cpp
		${CMAKE_CURRENT_LIST_DIR}/src/EIcore_runner.cpp
		${CMAKE_CURRENT_LIST_DIR}/src/Daudio_buffer.cpp)
	add_library(torasustd SHARED ${TORASU_STD_SRC})

	set_target_properties(torasustd PROPERTIES LINK_FLAGS "-Wl,-rpath,./")

	# Includes for current target


	if(WIN32)
  find_package(CURL CONFIG REQUIRED)
     target_link_libraries(torasustd PRIVATE CURL::libcurl)

 find_package(nlohmann_json CONFIG REQUIRED)
     target_link_libraries(torasustd PRIVATE nlohmann_json nlohmann_json::nlohmann_json)
	else()
	target_include_directories(torasustd PUBLIC lib/json/single_include/)
	target_link_libraries(torasustd torasu curl)
	endif()
	# Installation

	install(TARGETS torasustd DESTINATION lib)
	install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/include/torasu DESTINATION include)

endif()

cmake_policy(POP)
