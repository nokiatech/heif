# This file is part of Nokia HEIF library
#
# Copyright (c) 2015-2026 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
#
# Contact: heif@nokia.com
#
# This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its subsidiaries. All rights are reserved.
#
# Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior written consent of Nokia.

cmake_minimum_required(VERSION 3.5...3.28 FATAL_ERROR)

project(writer LANGUAGES CXX)

if(IOS)
    if(${IOS_PLATFORM} STREQUAL "OS")
        set(HEIF_WRITER_LIB_NAME heif_writer_static_iphoneos)
        set(HEIF_SHARED_WRITER_LIB_NAME heif_writer_shared_iphoneos)
    elseif(${IOS_PLATFORM} STREQUAL "SIMULATOR64")
        set(HEIF_WRITER_LIB_NAME heif_writer_static_iphonesimulator)
        set(HEIF_SHARED_WRITER_LIB_NAME heif_writer_shared_iphonesimulator)
    endif()
else()
    set(HEIF_WRITER_LIB_NAME heif_writer_static)
    set(HEIF_SHARED_WRITER_LIB_NAME heif_writer_shared)
endif()

set(WRITER_SRCS
    idgenerators.cpp
    refsgroup.cpp
    samplegroup.cpp
    timeutility.cpp
    writerimpl.cpp
    writermetaimpl.cpp
    writermoovimpl.cpp
    ../common/arraydatatype.cpp
    ../common/customallocator.cpp
    # See srcs/reader/CMakeLists.txt for the rationale: heiferrorinfo.cpp
    # defines HEIF_DLL_PUBLIC functions whose annotation must follow the
    # consuming target's static-vs-shared regime, which the `common`
    # OBJECT library cannot provide. Compiled here so the writer gets
    # the correct dllexport / visibility(default) when built shared.
    ../common/heiferrorinfo.cpp
    # Pure-C error wrappers -- same per-target HEIF_DLL_PUBLIC
    # rationale as heiferrorinfo.cpp above. Static-archive symbol
    # resolution dedups the duplicate definitions across reader+writer
    # shared libs harmlessly.
    ../common/heif_error_c.cpp
    # Pure-C runtime version accessor -- same per-target HEIF_DLL_PUBLIC
    # rationale as heif_error_c.cpp above. Compiled into the writer
    # target as well so a writer-only embedder can also do a SOVERSION
    # handshake without pulling in libheif_shared.
    ../common/heif_version_c.cpp
    )

if (WIN32)
	#Use native windows file api
	set(WRITER_SRCS ${WRITER_SRCS} fileoutputstream_win32.cpp)
else()
	#Use std::ofstream
	set(WRITER_SRCS ${WRITER_SRCS} fileoutputstream_std.cpp)
endif()

set(API_HDRS
    ../api/common/heifallocator.h
    ../api/common/heifexport.h
    ../api/common/heif_error_c.h
    ../api/common/heif_version_c.h
    ../api/common/heifid.h
    ../api/writer/heifwriter.h
    ../api/writer/heifwriterdatatypes.h
    )

set(WRITER_HDRS
    fileoutputstream.hpp
    idgenerators.hpp
    refsgroup.hpp
    samplegroup.hpp
    timeutility.hpp
    writerconstants.hpp
    writerdatatypesinternal.hpp
    writerimpl.hpp
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

set(HEIF_WRITER_LIB_COMMON_DEFINES "_FILE_OFFSET_BITS=64" "_LARGEFILE64_SOURCE" "HEIF_WRITER_LIB" $<$<BOOL:${ANDROID}>:HEIF_USE_LINUX_FILESTREAM>)

add_library(${HEIF_WRITER_LIB_NAME} STATIC ${WRITER_SRCS} ${API_HDRS} ${WRITER_HDRS} $<TARGET_OBJECTS:common> $<TARGET_OBJECTS:vvcparser>)


set_property(TARGET ${HEIF_WRITER_LIB_NAME} PROPERTY CXX_STANDARD 17)
target_compile_definitions(${HEIF_WRITER_LIB_NAME} INTERFACE "HEIF_USE_STATIC_LIB"
                                                   PRIVATE "HEIF_BUILDING_LIB"
                                                   PRIVATE ${HEIF_WRITER_LIB_COMMON_DEFINES})
target_include_directories(${HEIF_WRITER_LIB_NAME} PRIVATE ../common
                                                   PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../api/common>
                                                   PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../api/writer>
                                                   PUBLIC $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/heif>)


# vvc-parser is consumed only by writer-private writerimpl.hpp; keep PRIVATE.
target_include_directories(${HEIF_WRITER_LIB_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../vvc-parser)


if((NOT IOS) AND (NOT BUILD_ONLY_STATIC_LIB))
    add_library(${HEIF_SHARED_WRITER_LIB_NAME} SHARED ${WRITER_SRCS} ${API_HDRS} ${WRITER_HDRS} $<TARGET_OBJECTS:common> $<TARGET_OBJECTS:vvcparser>)

    set_property(TARGET ${HEIF_SHARED_WRITER_LIB_NAME} PROPERTY CXX_STANDARD 17)
    # Library hygiene: emit `lib<name>.so.<major>.<minor>.<patch>` with a
    # `lib<name>.so.<major>` SONAME symlink so consumers pin to ABI-major.
    set_target_properties(${HEIF_SHARED_WRITER_LIB_NAME} PROPERTIES
        VERSION   ${heif_VERSION}
        SOVERSION ${heif_VERSION_MAJOR}
    )

    # ABI surface containment.  Same version script as heif_shared;
    # heif_shared; see srcs/reader/CMakeLists.txt for the full rationale
    # (including the `NOT EMSCRIPTEN` gate that suppresses the harmless
    # `wasm-ld` warning under emcc).
    if((CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang") AND NOT APPLE AND NOT WIN32 AND NOT EMSCRIPTEN)
        set(HEIF_VERSION_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/heif.ld")
        target_link_options(${HEIF_SHARED_WRITER_LIB_NAME} PRIVATE
            "-Wl,--version-script=${HEIF_VERSION_SCRIPT}")
        set_target_properties(${HEIF_SHARED_WRITER_LIB_NAME} PROPERTIES
            LINK_DEPENDS "${HEIF_VERSION_SCRIPT}")
    endif()

    if (CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
      split_debug_info(${HEIF_SHARED_WRITER_LIB_NAME})
    endif()
    target_compile_definitions(${HEIF_SHARED_WRITER_LIB_NAME} INTERFACE "HEIF_USE_SHARED_LIB"
                                                              PRIVATE "HEIF_BUILDING_DLL"
                                                              PRIVATE ${HEIF_WRITER_LIB_COMMON_DEFINES})
    target_include_directories(${HEIF_SHARED_WRITER_LIB_NAME} PRIVATE ../common
                                                              PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../api/common>
                                                              PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../api/writer>
                                                              PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../vvc-parser
                                                              PUBLIC $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/heif>)
endif()
