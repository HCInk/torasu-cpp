cmake_minimum_required (VERSION 2.6)

project (torasu-cpp-examples)

include(../core/CMakeLists.txt)

include(../std/CMakeLists.txt)

set(CMAKE_CXX_STANDARD 20)
	
set(TORASU_EXAMPLES_SRC 
	${CMAKE_CURRENT_LIST_DIR}/src/torasu-cpp-examples.cpp)

add_executable(torasu-cpp-examples ${TORASU_EXAMPLES_SRC})

target_link_libraries(torasu-cpp-examples torasu torasustd)
