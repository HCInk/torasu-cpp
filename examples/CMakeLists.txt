cmake_minimum_required (VERSION 2.6)

project (torasu-cpp-examples)

include(../core/CMakeLists.txt)

include(../std/CMakeLists.txt)

add_executable(torasu-cpp-examples torasu-cpp-examples.cpp)

target_link_libraries(torasu-cpp-examples torasu torasustd)
