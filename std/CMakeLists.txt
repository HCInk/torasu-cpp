cmake_policy(PUSH)

cmake_minimum_required (VERSION 3.0)

if(NOT TARGET torasustd)

	# Meta / Settings

	project(torasu-cpp-std VERSION 0.0.4)
	set(CMAKE_CXX_STANDARD 20)

	# TORASU-CORE (local)
	include(${CMAKE_CURRENT_LIST_DIR}/../core/CMakeLists.txt)

	# Includes provided by this
	include_directories(BEFORE ${CMAKE_CURRENT_LIST_DIR}/include)

	# Compile

	set(TORASU_STD_SRC
		${CMAKE_CURRENT_LIST_DIR}/src/torasu-cpp-std.cpp
		${CMAKE_CURRENT_LIST_DIR}/src/torasu-std-index.cpp
		# Interfaces
		${CMAKE_CURRENT_LIST_DIR}/src/LIcore_logger.cpp
		${CMAKE_CURRENT_LIST_DIR}/src/EIcore_runner.cpp
		# Data
		${CMAKE_CURRENT_LIST_DIR}/src/Dnum.cpp
		${CMAKE_CURRENT_LIST_DIR}/src/Dstring.cpp
		${CMAKE_CURRENT_LIST_DIR}/src/Dstring_pair.cpp
		${CMAKE_CURRENT_LIST_DIR}/src/Dstring_map.cpp
		${CMAKE_CURRENT_LIST_DIR}/src/Dbimg.cpp
		${CMAKE_CURRENT_LIST_DIR}/src/Dbimg_sequence.cpp
		${CMAKE_CURRENT_LIST_DIR}/src/Daudio_buffer.cpp
		${CMAKE_CURRENT_LIST_DIR}/src/Dfile.cpp
		${CMAKE_CURRENT_LIST_DIR}/src/Dmatrix.cpp
		# Renderables
		${CMAKE_CURRENT_LIST_DIR}/src/Rnum.cpp
		${CMAKE_CURRENT_LIST_DIR}/src/Rmatrix.cpp
		${CMAKE_CURRENT_LIST_DIR}/src/Rstring.cpp
		${CMAKE_CURRENT_LIST_DIR}/src/Radd.cpp
		${CMAKE_CURRENT_LIST_DIR}/src/Rsubtract.cpp
		${CMAKE_CURRENT_LIST_DIR}/src/Rmultiply.cpp
		${CMAKE_CURRENT_LIST_DIR}/src/Rdivide.cpp
		${CMAKE_CURRENT_LIST_DIR}/src/Rlocal_file.cpp
		${CMAKE_CURRENT_LIST_DIR}/src/Rnet_file.cpp
		${CMAKE_CURRENT_LIST_DIR}/src/Rproperty.cpp
		${CMAKE_CURRENT_LIST_DIR}/src/Rjson_prop.cpp
		${CMAKE_CURRENT_LIST_DIR}/src/Rstring_replace.cpp
		${CMAKE_CURRENT_LIST_DIR}/src/Rmix_pipelines.cpp
		${CMAKE_CURRENT_LIST_DIR}/src/Rrctx_value.cpp
		${CMAKE_CURRENT_LIST_DIR}/src/Rmod_rctx.cpp
		${CMAKE_CURRENT_LIST_DIR}/src/Rfloor_mod.cpp
		${CMAKE_CURRENT_LIST_DIR}/src/Rsin.cpp
		${CMAKE_CURRENT_LIST_DIR}/src/Rfallback.cpp
		${CMAKE_CURRENT_LIST_DIR}/src/Rlog_message.cpp
		${CMAKE_CURRENT_LIST_DIR}/src/Rerror.cpp
		${CMAKE_CURRENT_LIST_DIR}/src/Rstring_map.cpp
		${CMAKE_CURRENT_LIST_DIR}/src/Rstring_concat.cpp
		${CMAKE_CURRENT_LIST_DIR}/src/Rstring_file.cpp
		${CMAKE_CURRENT_LIST_DIR}/src/Rnumber_string.cpp
		${CMAKE_CURRENT_LIST_DIR}/src/Rlist.cpp)

	if (BUILD_TORASU_STATIC)
		add_library(torasustd STATIC ${TORASU_STD_SRC})
	else()
		add_library(torasustd SHARED ${TORASU_STD_SRC})
	endif()

	# Linking libraries

	if (EMSCRIPTEN)
		set_target_properties(torasustd PROPERTIES LINK_FLAGS "-s FETCH=1")
	else()
		set_property(TARGET torasustd PROPERTY INSTALL_RPATH "/usr/local/lib")
		set_property(TARGET torasustd PROPERTY BUILD_RPATH "." "/usr/local/lib")
		target_link_libraries(torasustd curl)
	endif()
	target_link_libraries(torasustd torasu)
	# Installation

	install(TARGETS torasustd DESTINATION lib)
	install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/include/torasu DESTINATION include)

endif()

cmake_policy(POP)
