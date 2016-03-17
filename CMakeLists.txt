cmake_minimum_required(VERSION 2.8)
project(staticdb)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

if(${CMAKE_SYSTEM} MATCHES "Linux")
	set(STATICDB_LINUX ON)
endif()

message("C compiler:   " ${CMAKE_C_COMPILER})
message("C++ compiler: " ${CMAKE_CXX_COMPILER})

if(UNIX)
	if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
		add_definitions("-std=c++1y")
	else()
		execute_process(COMMAND ${CMAKE_CXX_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION)
		if(GCC_VERSION VERSION_GREATER 4.7)
			add_definitions(-std=c++1y)
		else()
			add_definitions(-std=c++0x)
		endif()
	endif()

	add_definitions("-Wall -Wextra -Wconversion -Wvla")

	#warn when using undefined macros
	add_definitions("-Wundef")

	add_definitions("-Werror")
endif()

if(MSVC)
	#make Visual C++ 2013 build in parallel
	add_definitions("/MP")

	#we want more warnings
	add_definitions("/W4")

	add_definitions("-DBOOST_ALL_NO_LIB")
	
	#Boost Asio uses deprecated Winsock APIs, but we are not interested in warnings about that
	add_definitions("-D_WINSOCK_DEPRECATED_NO_WARNINGS")

	#SILICIUM_TRAIT triggers warnings about missing arguments for the macro BOOST_PP_EXPAND_I (maybe a VC++ 2013 bug?)
	add_definitions("/wd4003")

	#Boost future has unreachable code for no reason
	add_definitions("/wd4702")
	
	#Boost in_place triggers /W4 warning about an assignment operator that could not be generated
	add_definitions("/wd4512")

	#stupid warning about std::copy
	add_definitions("/wd4996")

	#workaround for Boost 1.55 Context error LNK2026
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SAFESEH:NO")

	if(MSVC14)
		#Boost 1.58 Asio has a macro redefinition on Visual Studio 2015
		add_definitions("/wd4005")

		#unrestricted union bug
		add_definitions("/wd4624")

		#Boost 1.58 Coroutine warning about throw in a noexcept function
		add_definitions("/wd4297")
	endif()

	add_definitions("-DBOOST_ASIO_NO_DEPRECATED")

	set_property(GLOBAL PROPERTY USE_FOLDERS ON)
	
	if(MSVC14 AND (CMAKE_SIZEOF_VOID_P EQUAL 8))
		set(CMAKE_CXX_FLAGS_DEBUG "/D_DEBUG /MDd /Zi /Ob0 /Od")
	endif()
endif()

if(EXISTS ${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
	include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
	conan_basic_setup()
else()
	message(FATAL "you have to install the dependencies with Conan (conan.io)")
endif()

if(WIN32)
	#Boost.Asio wants this for no reason
	add_definitions("-D_WIN32_WINDOWS")

	add_definitions("-DBOOST_ASIO_HAS_IOCP")
endif()

add_definitions("-DSILICIUM_NO_DEPRECATED")

include_directories(".")
add_subdirectory("tests")

find_program(STATICDB_CLANG_FORMAT NAMES clang-format clang-format-3.7 clang-format-3.8 PATHS "C:/Program Files/LLVM/bin")
add_custom_target(clang-format COMMAND ${STATICDB_CLANG_FORMAT} -i ${formatted} WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
