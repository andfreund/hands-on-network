cmake_minimum_required(VERSION 3.14)

project(hands-on-network VERSION 0.0.1 LANGUAGES C CXX)

set(CMAKE_C_STANDARD   17)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_C_FLAGS           "-Wall -Wextra -Wpedantic")
set(CMAKE_CXX_FLAGS         "-Wall -Wextra -Wpedantic")
set(CMAKE_C_FLAGS_DEBUG     "-g3 -O0")
set(CMAKE_CXX_FLAGS_DEBUG   "-g3 -O0")
set(CMAKE_C_FLAGS_RELEASE   "-O3")
set(CMAKE_CXX_FLAGS_RELEAE  "-O3")

set(CMAKE_CXX_FLAGS_DEVELOP "-O1" CACHE STRING
 "Flags used by the C++ compiler during development builds."
  FORCE)
set(CMAKE_C_FLAGS_DEVELOP "-O1" CACHE STRING
  "Flags used by the C compiler during development builds."
  FORCE)
set(CMAKE_EXE_LINKER_FLAGS_DEVELOP
  "" CACHE STRING
  "Flags used for linking binaries during development builds."
  FORCE)
set(CMAKE_SHARED_LINKER_FLAGS_DEVELOP
  "" CACHE STRING
  "Flags used by the shared libraries linker during development builds."
  FORCE)

mark_as_advanced(
  CMAKE_CXX_FLAGS_DEVELOP
  CMAKE_C_FLAGS_DEVELOP
  CMAKE_EXE_LINKER_FLAGS_DEVELOP
  CMAKE_SHARED_LINKER_FLAGS_DEVELOP
)

option(BUILD_UNITTESTS "Enable unit test builds with GoogleTest" ON)

if(BUILD_UNITTESTS AND (CMAKE_BUILD_TYPE STREQUAL "Release"))
	message(STATUS "Unit tests are not built in Release configuration")
	set(BUILD_UNITTESTS OFF)
endif()

if(BUILD_UNITTESTS)
	include(FetchContent)
	FetchContent_Declare(
  		googletest
  		GIT_REPOSITORY https://github.com/google/googletest.git
  		GIT_TAG v1.13.0
	)
	FetchContent_MakeAvailable(googletest)
	enable_testing()
endif()

add_subdirectory(hands-on-network)
