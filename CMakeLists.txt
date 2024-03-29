
cmake_minimum_required (VERSION 3.0)
project(torasu-cpp VERSION 0.0.4)
set(CMAKE_CXX_STANDARD 20)

if (NOT MINGW AND NOT APPLE)
	set(TORASU_DEBUG_ASAN true)
endif()

option(BUILD_TORASU_STATIC "Build TORASU-Libraries with static linking" OFF)

if (EMSCRIPTEN)
	message( STATUS "Linking static with emscripten" )
	set(BUILD_TORASU_STATIC ON)
	set(TORASU_DEBUG_ASAN OFF)
	add_compile_options("SHELL:-pthread -s USE_PTHREADS -fexceptions")
	add_link_options("SHELL:-pthread -s USE_PTHREADS -fexceptions")
endif()

include(core/CMakeLists.txt)
include(std/CMakeLists.txt)
include(examples/CMakeLists.txt)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall")

if( MINGW )

    message( STATUS "Installing system-libraries: MinGW DLLs." )
	# get_filename_component( Mingw_Path ${CMAKE_CXX_COMPILER} PATH )
	set(Mingw_Path "/usr/x86_64-w64-mingw32/bin/")
	configure_file(${Mingw_Path}/libstdc++-6.dll libstdc++-6.dll COPYONLY)
	configure_file(${Mingw_Path}/libwinpthread-1.dll libwinpthread-1.dll COPYONLY)
	configure_file(${Mingw_Path}/libgcc_s_seh-1.dll libgcc_s_seh-1.dll COPYONLY)

	# For tests
	configure_file(${Mingw_Path}/libssp-0.dll libssp-0.dll COPYONLY)
	
endif()