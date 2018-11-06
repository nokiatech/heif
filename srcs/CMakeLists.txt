# This file is part of Nokia HEIF library
#
# Copyright (c) 2015-2018 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
#
# Contact: heif@nokia.com
#
# This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its subsidiaries. All rights are reserved.
#
# Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior written consent of Nokia.

cmake_minimum_required(VERSION 3.3.2 FATAL_ERROR)

if(${CMAKE_EXTRA_GENERATOR} MATCHES "Eclipse CDT4")
    set(CMAKE_ECLIPSE_VERSION "4.4" CACHE STRING "Eclipse version" FORCE)
    set(CMAKE_CXX_COMPILER_ARG1 "-std=c++11" CACHE STRING "C++ version for eclipse" FORCE)
endif()

project(heif LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if (USE_LIBC++)
  if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    include_directories(SYSTEM "/usr/include/libcxxabi/")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++ -lc++abi")
  else()
    message("libc++ requires use of Clang.")
  endif()
endif()


#verify which type of allocator we can use.
try_compile(COMPILE_SUCCEEDED ${CMAKE_BINARY_DIR}/compile_tests SOURCES ${PROJECT_SOURCE_DIR}/../build/allocator_test.cpp CMAKE_FLAGS CXX_STANDARD 11)
if(COMPILE_SUCCEEDED)
  message("Allocator test passed")
else()
  message("Allocator test failed, enable fix")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DHEIF_GCC_ALLOCATOR_FIX=1")
endif()


# picked from http://stackoverflow.com/a/3818084
if(MSVC)
  # Force to always compile with W4
  if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
    string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
  endif()

  # Enable multiprocessor compilation (some claim this might be harmful?)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")

elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  # Use MATCHES for Clang to also include AppleClang
  # Update if necessary
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W -Wall -Wextra -Wcast-align")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wcast-qual -Wdisabled-optimization")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Winit-self")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wmissing-declarations -Wmissing-include-dirs")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Woverloaded-virtual")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wredundant-decls -Wshadow -Wsign-conversion")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wsign-promo")

  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wlogical-op")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wnoexcept")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wstrict-null-sentinel")
  endif()

  # -pedantic
  # -Wctor-dtor-privacy
  # -Wformat=2
  # -Wold-style-cast
  # -Wundef
  # -Wstrict-overflow=5
  # -Wswitch-default
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")

  if(DEBUG_STL)
    message("Enabling STL debug mode for the debug build.")
    SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_GLIBCXX_DEBUG")
  endif()
else()
  if (NOT IOS)
    message(FATAL_ERROR "Unsupported compiler!")
  endif()
endif()

if(DISABLE_UNCOVERED_CODE)
  message("Strip some code behind #ifdef for coverage. May cause runtime errors! For coverage analysis only!")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DDISABLE_UNCOVERED_CODE=1")
endif()

if(COVERAGE)
  message("Enabling coverage analysis with gcov")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage -ftest-coverage -fprofile-arcs")
  link_libraries(--coverage -ftest-coverage -fprofile-arcs)
endif(COVERAGE)

if(ANDROID)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__GXX_EXPERIMENTAL_CXX0X__ -D_FILE_OFFSET_BITS=64")
endif()

if(IOS)
  set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -std=c++11")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
endif()

# # Path to where the executable resides under where build is called from
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Get build version from git
execute_process(
    COMMAND "git" "describe"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_DESCRIBE
    RESULT_VARIABLE GIT_ERROR
    OUTPUT_STRIP_TRAILING_WHITESPACE
    )

if(GIT_ERROR)
    message( "Could not get version information from git")
    set(GIT_DESCRIBE "v3.4.0")
endif()

# Get build timestamp
string(TIMESTAMP BUILD_TIMESTAMP UTC)

# Create the version number header
configure_file("${PROJECT_SOURCE_DIR}/buildinfo/buildinfo.hpp.in" "${PROJECT_BINARY_DIR}/buildinfo.hpp")
include_directories("${PROJECT_BINARY_DIR}")

add_subdirectory(common)
add_subdirectory(reader)

add_subdirectory(writer)
if (NOT IOS)
  add_subdirectory(examples)
endif()

add_subdirectory(api-cpp)


if (NOT ANDROID)
  if (NOT IOS)
    find_package(JNI)
    if (JNI_FOUND)
      add_subdirectory(api-java/cpp)
      endif()
  endif()
endif()

message(STATUS "System name       : ${CMAKE_SYSTEM_NAME}")
message(STATUS "Project Name      : ${PROJECT_NAME}")
message(STATUS "Project directory : ${PROJECT_SOURCE_DIR}")
message(STATUS "Build called from : ${PROJECT_BINARY_DIR}")
message(STATUS "Executables in    : ${EXECUTABLE_OUTPUT_PATH}")
message(STATUS "Libraries in      : ${LIBRARY_OUTPUT_DIRECTORY}")

# For Ninja cmake generator
cmake_policy(SET CMP0058 NEW)
