# Copyright (c) 2015, Nokia Technologies Ltd.
# All rights reserved.
#
# Licensed under the Nokia High-Efficiency Image File Format (HEIF) License (the "License").
#
# You may not use the High-Efficiency Image File Format except in compliance with the License.
# The License accompanies the software and can be found in the file "LICENSE.TXT".
#
# You may also obtain the License at:
# https://nokiatech.github.io/heif/license.txt

# The project name
project(HEIF)

# Required version of CMake
cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

# Get build version from git
exec_program(
    "git"
    ${CMAKE_CURRENT_SOURCE_DIR}
    ARGS "describe"
    OUTPUT_VARIABLE GIT_DESCRIBE)

# Get build timestamp
string(TIMESTAMP BUILD_TIMESTAMP UTC)

# Create the version number header
configure_file("${PROJECT_SOURCE_DIR}/Srcs/buildinfo/buildinfo.hpp.in" "${PROJECT_BINARY_DIR}/buildinfo.hpp")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# Path to where the executable resides
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/Bins)

message(STATUS "System name       : ${CMAKE_SYSTEM_NAME}")
message(STATUS "Project Name      : ${PROJECT_NAME}")
message(STATUS "Project directory : ${PROJECT_SOURCE_DIR}")
message(STATUS "Executables in    : ${EXECUTABLE_OUTPUT_PATH}")
message(STATUS "File-list         : ${FILELIST}")

# Set executable filenames
set(WRITER_EXE writerapp)
set(EXAMPLE_EXE examples)

set(CMAKE_VERBOSE_MAKEFILE on)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    # GCC 4.9.1 gives unnecessary warnings about missing field initializers when compiling
    # with -std=c++11 and -Wextra, so use -Wno-missing-field-initializers for now.
    set(warnings "-Wall -Wextra -Werror -Wno-missing-field-initializers -Wno-error=unused-parameter -Wno-error=unused-variable")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    set(warnings "/W4 /EHsc")
endif()

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
    message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${warnings}")
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} ${warnings}")

# Disable Visual Studio Microsoft language extensions
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Za")
    set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} /Za")
endif()

# Add sub-directories that contain sources
add_subdirectory(${PROJECT_SOURCE_DIR}/Srcs)

# Add Doxygen API documentation target
add_subdirectory(Docs)
