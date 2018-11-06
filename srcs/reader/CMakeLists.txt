# This file is part of Nokia HEIF library
#
# Copyright (c) 2015-2018 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
#
# Contact: heif@nokia.com
#
# This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its subsidiaries. All rights are reserved.
#
# Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior written consent of Nokia.

cmake_minimum_required(VERSION 3.2 FATAL_ERROR)

if(${CMAKE_EXTRA_GENERATOR} MATCHES "Eclipse CDT4")
    set(CMAKE_ECLIPSE_VERSION "4.4" CACHE STRING "Eclipse version" FORCE)
    set(CMAKE_CXX_COMPILER_ARG1 "-std=c++11" CACHE STRING "C++ version for eclipse" FORCE)
endif()

project(reader LANGUAGES CXX)


if(IOS)
    if(${IOS_PLATFORM} STREQUAL "OS")
        set(HEIF_LIB_NAME heif_static_iphoneos)
        set(HEIF_SHARED_LIB_NAME heif_shared_iphoneos)
    elseif(${IOS_PLATFORM} STREQUAL "SIMULATOR64")
        set(HEIF_LIB_NAME heif_static_iphonesimulator)
        set(HEIF_SHARED_LIB_NAME heif_shared_iphonesimulator)
    endif()
else()
    set(HEIF_LIB_NAME heif_static)
    set(HEIF_SHARED_LIB_NAME heif_shared)
endif()

set(READER_SRCS
    heifreaderimpl.cpp
    heifreaderaccessors.cpp
    heifstreamfile.cpp
    heifstreamgeneric.cpp
    heifstreaminterface.cpp
    heifstreaminternal.cpp
    ../common/arraydatatype.cpp
    ../common/customallocator.cpp
    $<$<BOOL:${ANDROID}>:heifstreamlinux.cpp>
    )

set(API_HDRS
    ../api/common/heifid.h
    ../api/common/heifstreaminterface.h
    ../api/common/heifexport.h
    ../api/reader/heifreaderdatatypes.h
    ../api/reader/heifreader.h
    )

set(READER_HDRS
    heiffiledatatypesinternal.hpp
    heifreaderimpl.hpp
    heifstreamfile.hpp
    heifstreamgeneric.hpp
    heifstreaminternal.hpp
    )

macro(split_debug_info target)
  # objcopy doesn't work on MacOSX, so exclude that
  if(NOT CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
      # Use MATCHES for Clang to also include AppleClang.. though it's not really needed as it's excluded.
      add_custom_command(TARGET ${target}
    POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} --only-keep-debug $<TARGET_FILE:${target}> $<TARGET_FILE:${target}>.debug
    COMMAND ${CMAKE_OBJCOPY} --strip-debug $<TARGET_FILE:${target}>
    COMMAND ${CMAKE_OBJCOPY} --add-gnu-debuglink=$<TARGET_FILE:${target}>.debug $<TARGET_FILE:${target}>
    )
    endif()
  endif()
endmacro()

set(HEIF_LIB_COMMON_DEFINES "_FILE_OFFSET_BITS=64" "_LARGEFILE64_SOURCE" "HEIF_READER_LIB" $<$<BOOL:${ANDROID}>:HEIF_USE_LINUX_FILESTREAM>)

add_library(${HEIF_LIB_NAME} STATIC ${READER_SRCS} ${API_HDRS} ${READER_HDRS} $<TARGET_OBJECTS:common>)
set_property(TARGET ${HEIF_LIB_NAME} PROPERTY CXX_STANDARD 11)
target_compile_definitions(${HEIF_LIB_NAME} INTERFACE "HEIF_USE_STATIC_LIB"
                                            PRIVATE "HEIF_BUILDING_LIB"
                                            PRIVATE ${HEIF_LIB_COMMON_DEFINES})
target_include_directories(${HEIF_LIB_NAME} PRIVATE ../common
                                            PUBLIC ../api/common
                                            PUBLIC ../api/reader)

if(NOT IOS)
    add_library(${HEIF_SHARED_LIB_NAME} SHARED ${READER_SRCS} ${API_HDRS} ${READER_HDRS} $<TARGET_OBJECTS:common> )
    set_property(TARGET ${HEIF_SHARED_LIB_NAME} PROPERTY CXX_STANDARD 11)
    if (CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
      split_debug_info(${HEIF_SHARED_LIB_NAME})
    endif()
    target_compile_definitions(${HEIF_SHARED_LIB_NAME} INTERFACE "HEIF_USE_SHARED_LIB"
                                                       PRIVATE "HEIF_BUILDING_DLL"
                                                       PRIVATE ${HEIF_LIB_COMMON_DEFINES})
    target_include_directories(${HEIF_SHARED_LIB_NAME} PRIVATE ../common
                                                       PUBLIC ../api/common
                                                       PUBLIC ../api/reader)
endif(NOT IOS)
