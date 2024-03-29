cmake_policy(PUSH)

cmake_minimum_required (VERSION 3.0)

if(NOT TARGET torasu)

	# Meta / Settings

	project(torasu-cpp-core VERSION 0.0.4)
	set(CMAKE_CXX_STANDARD 20)

	# Includes provided by this

	include_directories(BEFORE ${CMAKE_CURRENT_LIST_DIR}/include)
	include_directories(BEFORE ${CMAKE_CURRENT_LIST_DIR}/../lib/include)

	# Compile

	set(TORASU_CORE_SRC
		${CMAKE_CURRENT_LIST_DIR}/src/torasu-cpp-core.cpp
		${CMAKE_CURRENT_LIST_DIR}/src/torasu-core-index.cpp
		${CMAKE_CURRENT_LIST_DIR}/src/DataPackable.cpp
		${CMAKE_CURRENT_LIST_DIR}/src/render_tools.cpp
		${CMAKE_CURRENT_LIST_DIR}/src/SimpleRenderable.cpp
		${CMAKE_CURRENT_LIST_DIR}/src/Dlog_entry.cpp)
if (BUILD_TORASU_STATIC)
	add_library(torasu STATIC ${TORASU_CORE_SRC})
else()
	add_library(torasu SHARED ${TORASU_CORE_SRC})
endif()
	# Installation

	install(TARGETS torasu DESTINATION lib)
	install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/include/torasu DESTINATION include)
	install(FILES ${CMAKE_CURRENT_LIST_DIR}/../lib/include/torasu/json.hpp DESTINATION include/torasu)
	
	# Deployed libraries
	install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/../lib/json/single_include/nlohmann DESTINATION include/torasu/lib)

endif()

cmake_policy(POP)
