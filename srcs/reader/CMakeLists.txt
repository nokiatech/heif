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
    heifreadersegment.cpp
    heifstreamfile.cpp
    heifstreamgeneric.cpp
    heifstreaminterface.cpp
    heifstreaminternal.cpp
    # Pure-C reader facade (~9 entry points) over HEIF::Reader.
    # Only the reader build ships these symbols; they intentionally
    # never appear on libheif_writer_shared.so.
    # See srcs/api/reader/heifreader_c.h for the design contract.
    heifreader_c.cpp
    ../common/arraydatatype.cpp
    ../common/customallocator.cpp
    # heiferrorinfo.cpp defines HEIF_DLL_PUBLIC free functions
    # (setErrorInfo / getErrorInfo / setErrorSink / formatErrorInfo / ...).
    # It cannot live in the `common` OBJECT library because that single .o
    # is reused by both the static and shared library targets, and the
    # HEIF_DLL_PUBLIC macro needs different per-target expansions:
    #   * static target: HEIF_BUILDING_LIB     -> empty annotation
    #   * shared target: HEIF_BUILDING_DLL     -> dllexport / visibility(default)
    # Compiling it directly here gives it the per-target macros set on
    # heif_static / heif_shared below. Same TU is also added to the
    # writer CMakeLists for the same reason; static-archive symbol
    # resolution dedups the duplicates harmlessly.
    ../common/heiferrorinfo.cpp
    # Pure-C error-inspection wrappers over the same
    # thread_local slot. Same per-target HEIF_DLL_PUBLIC rationale as
    # heiferrorinfo.cpp above -- must be compiled here, not inside the
    # `common` OBJECT library, so dllexport/visibility(default) follow
    # the consuming static-vs-shared regime.
    ../common/heif_error_c.cpp
    # Pure-C runtime version accessor
    # (heif_get_runtime_version{,_string,_number}). Same
    # per-target HEIF_DLL_PUBLIC rationale as heif_error_c.cpp above --
    # the symbols must follow each consuming target's static-vs-shared
    # macro regime, so the TU is compiled here rather than tucked into
    # the shared `common` OBJECT library. Lets a binding do a load-
    # time SOVERSION handshake from cwrap-only without going back
    # through embind / C++ name mangling.
    ../common/heif_version_c.cpp
    $<$<BOOL:${ANDROID}>:heifstreamlinux.cpp>
    )

set(API_HDRS
    ../api/common/heifid.h
    ../api/common/heifstreaminterface.h
    ../api/common/heifexport.h
    ../api/common/heif_error_c.h
    ../api/common/heif_version_c.h
    ../api/reader/heifreaderdatatypes.h
    ../api/reader/heifreader.h
    ../api/reader/heifreader_c.h
    )

set(READER_HDRS
    heiffiledatatypesinternal.hpp
    heifreaderimpl.hpp
    heifreadersegment.hpp
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

add_library(${HEIF_LIB_NAME} STATIC ${READER_SRCS} ${API_HDRS} ${READER_HDRS} $<TARGET_OBJECTS:common> $<TARGET_OBJECTS:vvcparser>)
set_property(TARGET ${HEIF_LIB_NAME} PROPERTY CXX_STANDARD 17)
target_compile_definitions(${HEIF_LIB_NAME} INTERFACE "HEIF_USE_STATIC_LIB"
                                            PRIVATE "HEIF_BUILDING_LIB"
                                            PRIVATE ${HEIF_LIB_COMMON_DEFINES})
target_include_directories(${HEIF_LIB_NAME} PRIVATE ../common
                                            PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../api/common>
                                            PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../api/reader>
                                            PUBLIC $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/heif>)


if((NOT IOS) AND (NOT BUILD_ONLY_STATIC_LIB))
    add_library(${HEIF_SHARED_LIB_NAME} SHARED ${READER_SRCS} ${API_HDRS} ${READER_HDRS} $<TARGET_OBJECTS:common> $<TARGET_OBJECTS:vvcparser>)
    set_property(TARGET ${HEIF_SHARED_LIB_NAME} PROPERTY CXX_STANDARD 17)
    # Library hygiene: emit `lib<name>.so.<major>.<minor>.<patch>` with a
    # `lib<name>.so.<major>` SONAME symlink. Lets the runtime linker pin
    # consumers to the ABI-major they were built against, which is the
    # whole point of shipping a versioned shared object.
    set_target_properties(${HEIF_SHARED_LIB_NAME} PROPERTIES
        VERSION   ${heif_VERSION}
        SOVERSION ${heif_VERSION_MAJOR}
    )
    # ABI surface containment.  Apply the ELF version script to the shared
    # library so only HEIF::*, HEIFPP::*, and `heif_*` symbols are
    # exported; std:: instantiations and other accidental leakage are
    # forced to local visibility.  Gated on a GNU-style linker because
    # the `--version-script` flag is GNU ld / LLD only (not MSVC link.exe;
    # macOS ld64 has its own `-exported_symbols_list` mechanism that
    # would need a separate file).  Also skipped under emcc:
    # `wasm-ld` accepts the flag but ignores most of it and emits a
    # warning, and wasm consumers control their export set via
    # `EXPORTED_FUNCTIONS` / `EMSCRIPTEN_KEEPALIVE` instead.
    if((CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang") AND NOT APPLE AND NOT WIN32 AND NOT EMSCRIPTEN)
        set(HEIF_VERSION_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/heif.ld")
        target_link_options(${HEIF_SHARED_LIB_NAME} PRIVATE
            "-Wl,--version-script=${HEIF_VERSION_SCRIPT}")
        # Re-link when the version script changes.
        set_target_properties(${HEIF_SHARED_LIB_NAME} PROPERTIES
            LINK_DEPENDS "${HEIF_VERSION_SCRIPT}")
    endif()
    if (CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
      split_debug_info(${HEIF_SHARED_LIB_NAME})
    endif()
    target_compile_definitions(${HEIF_SHARED_LIB_NAME} INTERFACE "HEIF_USE_SHARED_LIB"
                                                       PRIVATE "HEIF_BUILDING_DLL"
                                                       PRIVATE ${HEIF_LIB_COMMON_DEFINES})
    target_include_directories(${HEIF_SHARED_LIB_NAME} PRIVATE ../common
                                                       PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../api/common>
                                                       PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../api/reader>
                                                       PUBLIC $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/heif>)
endif()
