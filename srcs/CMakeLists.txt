# This file is part of Nokia HEIF library
#
# Copyright (c) 2015-2026 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
#
# Contact: heif@nokia.com
#
# This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its subsidiaries. All rights are reserved.
#
# Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior written consent of Nokia.

# Sub-project so PROJECT_SOURCE_DIR points here for all subdirectory CMakeLists.
project(heif VERSION 4.0.0 LANGUAGES CXX C)

# Default to Release for single-config generators (Makefiles, Ninja).
# Multi-config generators (Visual Studio, Xcode) ignore this and let
# the user pick the config at build time instead.
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    Debug Release RelWithDebInfo MinSizeRel)
endif()



# -- Python binding sdist / wheel build switch ------------------------------
#
# When ON, this build is being driven by scikit-build-core from the
# repo-root pyproject.toml to produce a Python sdist or wheel. Trims
# the build aggressively:
#
#   * Skips examples/, api-java/cpp. These produce
#     artifacts the binding does not need.
#   * Skips the system-install ruleset at the bottom of this file
#     (heif_static, heif_writer_static, heifpp, headers, EXPORT,
#     heifConfig.cmake, heif.pc) -- none of those go into a Python
#     wheel. Replaced by a single install(TARGETS heif_shared) into
#     the `heif` package layout used by the Python binding's tier-2
#     resolver in bindings/python/heif/_ffi.py::_search_wheel_bundle().
#
# Off by default; the system-install workflow (`cmake --install build`)
# is unchanged.
option(HEIF_PYTHON_BINDING_ONLY
       "Trim the build for Python binding sdist/wheel packaging" OFF)
if(HEIF_PYTHON_BINDING_ONLY)
  message(STATUS "HEIF_PYTHON_BINDING_ONLY=ON -- building heif_shared only for Python wheel/sdist")
endif()

set(CMAKE_CXX_STANDARD 17)
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
try_compile(COMPILE_SUCCEEDED ${CMAKE_BINARY_DIR}/_compile_check SOURCES ${PROJECT_SOURCE_DIR}/../build/allocator_test.cpp CMAKE_FLAGS CXX_STANDARD 17)
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

  # Enforce strict C++ conformance (catches implicit conversions that GCC/Clang reject)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /permissive-")

  # Enable multiprocessor compilation (some claim this might be harmful?)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")

elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  # Use MATCHES for Clang to also include AppleClang
  # Note: -g is NOT added here -- CMake's built-in flags already include it
  # for Debug (-g) and RelWithDebInfo (-O2 -g -DNDEBUG). Adding it here
  # would force debug symbols into Release and MinSizeRel builds.
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

  # ABI surface containment.  Two flags work together:
  #
  #   -fvisibility=hidden          : symbols are hidden by default;
  #                                  only those tagged HEIF_DLL_PUBLIC
  #                                  (see srcs/api/common/heifexport.h)
  #                                  are exported.
  #   -fvisibility-inlines-hidden  : inline functions and template
  #                                  instantiations are hidden even
  #                                  when their enclosing class is
  #                                  HEIF_DLL_PUBLIC.  Without this,
  #                                  every inline ctor/dtor/operator=
  #                                  of a public class leaks as a
  #                                  weak symbol -- a 3923-symbol pile
  #                                  on a 133-symbol intentional
  #                                  surface, all of which would be
  #                                  covered by the SOVERSION 4 ABI
  #                                  contract by accident.
  #
  # The ELF version script `srcs/cmake/heif.ld` (consumed via
  # -Wl,--version-script= in srcs/lib/CMakeLists.txt) provides
  # belt-and-braces enforcement of the exported symbol set.
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden -fvisibility-inlines-hidden")
else()
  if (NOT IOS)
    message(FATAL_ERROR "Unsupported compiler!")
  endif()
endif()

if(ANDROID)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_FILE_OFFSET_BITS=64")
endif()

if(IOS)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
endif()

# # Path to where the executable resides under where build is called from
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)

# For multi-config generators (Visual Studio, Xcode), prevent per-config subdirectories
# (e.g. bin/Debug/, bin/Release/) so output goes directly to bin/ and lib/ like on Linux.
foreach(CONFIG_TYPE ${CMAKE_CONFIGURATION_TYPES})
  string(TOUPPER ${CONFIG_TYPE} CONFIG_TYPE_UPPER)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONFIG_TYPE_UPPER} ${CMAKE_BINARY_DIR}/bin)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CONFIG_TYPE_UPPER} ${CMAKE_BINARY_DIR}/lib)
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CONFIG_TYPE_UPPER} ${CMAKE_BINARY_DIR}/lib)
endforeach()

# Verbose makefiles for Debug builds (shows full compiler command lines).
# Release/RelWithDebInfo/MinSizeRel get clean progress-only output.
#
# IMPORTANT: only apply this to single-config generators (Make / Ninja).
# Multi-config generators (Visual Studio, Xcode) leave CMAKE_BUILD_TYPE
# empty at configure time and pick the config at build time via --config.
# Setting CMAKE_VERBOSE_MAKEFILE there would either (a) silently turn
# every config verbose because a stale `-DCMAKE_BUILD_TYPE=Debug` is
# still in the cache, or (b) miss Debug entirely. For multi-config
# generators the equivalent control is `cmake --build ... --verbose`
# or msbuild's `/v:` flag -- let the developer opt in explicitly.
get_property(_HEIF_IS_MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(NOT _HEIF_IS_MULTI_CONFIG)
  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_VERBOSE_MAKEFILE ON)
  else()
    set(CMAKE_VERBOSE_MAKEFILE OFF)
  endif()
endif()
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Get build version.
#
# BuildInfo::Version must always agree with HEIF_VERSION_MAJOR/MINOR/PATCH and
# the SONAME of the shipped .so. Historically this was set straight from
# `git describe`, which silently drifts when no tag matches PROJECT_VERSION.
# We now anchor on
# PROJECT_VERSION and append the git short-sha purely as build provenance.
execute_process(
    COMMAND "git" "rev-parse" "--short" "HEAD"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_SHA
    RESULT_VARIABLE GIT_ERROR
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
    )

if(GIT_ERROR OR "${GIT_SHA}" STREQUAL "")
    set(GIT_DESCRIBE "v${PROJECT_VERSION}")
else()
    set(GIT_DESCRIBE "v${PROJECT_VERSION}+${GIT_SHA}")
endif()

# Get build timestamp
string(TIMESTAMP BUILD_TIMESTAMP UTC)

# Create the version number header
configure_file("${PROJECT_SOURCE_DIR}/buildinfo/buildinfo.hpp.in" "${PROJECT_BINARY_DIR}/buildinfo.hpp")
include_directories("${PROJECT_BINARY_DIR}")

message(STATUS "VVC support: Enabled")
set(VVC_PARSER "Custom NAL parser")
add_subdirectory(vvc-parser)

# Needed early so subdirectories can use CMAKE_INSTALL_INCLUDEDIR etc.
include(GNUInstallDirs)

add_subdirectory(common)
add_subdirectory(reader)
add_subdirectory(writer)

# Under HEIF_PYTHON_BINDING_ONLY everything below this point is skipped:
# CLI tools, examples, C++ convenience API, JNI bridge.
# The Python binding only consumes heif_shared.
if(NOT HEIF_PYTHON_BINDING_ONLY)

# CLI tools
if(NOT IOS)
  add_subdirectory(readerapp)
endif()

if (NOT IOS)
  add_subdirectory(examples)
endif()

add_subdirectory(api-cpp)

if ((NOT ANDROID) AND (NOT IOS) AND (NOT BUILD_ONLY_STATIC_LIB))
  find_package(JNI)
  if (JNI_FOUND)
    add_subdirectory(api-java/cpp)
  endif()
endif()

endif() # NOT HEIF_PYTHON_BINDING_ONLY

message(STATUS "")
message(STATUS "========== Build Configuration Summary ==========")
message(STATUS "System name       : ${CMAKE_SYSTEM_NAME}")
message(STATUS "Project Name      : ${PROJECT_NAME}")
message(STATUS "Project directory : ${PROJECT_SOURCE_DIR}")
message(STATUS "Build called from : ${PROJECT_BINARY_DIR}")
message(STATUS "Executables in    : ${EXECUTABLE_OUTPUT_PATH}")
message(STATUS "Libraries in      : ${LIBRARY_OUTPUT_PATH}")
message(STATUS "")
message(STATUS "---------- Build Type ----------")
if(CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Build type        : Multi-config (${CMAKE_CONFIGURATION_TYPES})")
  message(STATUS "  -> Select at build time: cmake --build build --config Release")
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
  message(STATUS "Build type        : Debug")
  message(STATUS "  -> No optimization, full debug symbols (-g)")
  message(STATUS "  -> Assertions ENABLED (NDEBUG not defined)")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
  message(STATUS "Build type        : Release")
  message(STATUS "  -> Full optimization (-O3), no debug symbols")
  message(STATUS "  -> Assertions DISABLED (NDEBUG defined)")
elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
  message(STATUS "Build type        : RelWithDebInfo")
  message(STATUS "  -> Optimization (-O2) with debug symbols (-g)")
  message(STATUS "  -> Assertions DISABLED (NDEBUG defined)")
elseif(CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
  message(STATUS "Build type        : MinSizeRel")
  message(STATUS "  -> Size-optimized (-Os), no debug symbols")
  message(STATUS "  -> Assertions DISABLED (NDEBUG defined)")
else()
  message(STATUS "Build type        : NOT SET")
  message(STATUS "  -> WARNING: Use -DCMAKE_BUILD_TYPE=<Debug|Release|RelWithDebInfo|MinSizeRel>")
endif()
message(STATUS "CXX Compiler      : ${CMAKE_CXX_COMPILER_ID} (${CMAKE_CXX_COMPILER})")
message(STATUS "CXX Flags         : ${CMAKE_CXX_FLAGS}")
if(NOT CMAKE_CONFIGURATION_TYPES)
  string(TOUPPER "${CMAKE_BUILD_TYPE}" _BUILD_TYPE_UPPER)
  message(STATUS "CXX Flags (${CMAKE_BUILD_TYPE}): ${CMAKE_CXX_FLAGS_${_BUILD_TYPE_UPPER}}")
endif()
message(STATUS "")
message(STATUS "---------- Options ----------")
# Normalise unset / empty value to OFF so the summary never prints a
# blank field that looks like a configuration error.
if(NOT BUILD_ONLY_STATIC_LIB)
  set(BUILD_ONLY_STATIC_LIB OFF)
endif()
message(STATUS "VVC Support       : ${VVC_PARSER}")
message(STATUS "Static lib only   : ${BUILD_ONLY_STATIC_LIB}")
message(STATUS "")
message(STATUS "=================================================")
if(CMAKE_CONFIGURATION_TYPES)
  message(STATUS "")
  message(STATUS "Build:   cmake --build . --config Release")
  message(STATUS "Install: cmake --install . --config Release --prefix <path>")
else()
  message(STATUS "")
  message(STATUS "Build:   cmake --build ${PROJECT_BINARY_DIR}")
  message(STATUS "Install: cmake --install ${PROJECT_BINARY_DIR} --prefix <path>")
endif()
message(STATUS "")

# -- install() rules ---------------------------------------------
# Usage:
#   cmake --install build --prefix /usr/local
#
# Installs libraries to    <prefix>/lib/
#         C API headers to <prefix>/include/heif/
#         C++ API headers  <prefix>/include/heif/heifpp/
#         CMake config to  <prefix>/lib/cmake/heif/
#
# Under HEIF_PYTHON_BINDING_ONLY the layout is different: scikit-build-core
# extracts files from CMAKE_INSTALL_PREFIX into the wheel root, so we want
# heif_shared to land at <wheel-root>/heif/ (next to _ffi.py, where tier 2
# of the resolver looks for it). Headers, pkg-config, EXPORT and the C++
# convenience targets are not installed in this mode -- a Python wheel does
# not need them.

if(HEIF_PYTHON_BINDING_ONLY)

  # Wheel-layout install: heif_shared lands in the `heif` package directory
  # next to _ffi.py. The COMPONENT name lets scikit-build-core filter to
  # exactly this rule via [tool.scikit-build] install.components.
  if(TARGET heif_shared)
    install(TARGETS heif_shared
      LIBRARY DESTINATION heif COMPONENT python_binding
      RUNTIME DESTINATION heif COMPONENT python_binding   # Windows DLLs land here too
    )
  else()
    message(FATAL_ERROR
      "HEIF_PYTHON_BINDING_ONLY=ON but heif_shared target was not created. "
      "This usually means BUILD_ONLY_STATIC_LIB is also ON or the build is "
      "targeting iOS -- the Python binding requires the shared library.")
  endif()

else() # NOT HEIF_PYTHON_BINDING_ONLY -- normal system install path

# --- Libraries ---
# Components: runtime  = shared libs only (distro -lib / runtime packages)
#             dev      = static libs + headers + CMake/pkg-config config
#             jni      = Java JNI shared library
# Usage: cmake --install build --component runtime
#        cmake --install build --component dev
#        cmake --install build --component jni
# Omitting --component installs everything (backward-compatible default).
install(TARGETS heif_static heif_writer_static heifpp
  EXPORT heifTargets
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT dev
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT dev
)

if(TARGET heif_shared)
  install(TARGETS heif_shared
    EXPORT heifTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT dev
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT runtime
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT runtime
  )
endif()
if(TARGET heif_writer_shared)
  install(TARGETS heif_writer_shared
    EXPORT heifTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT dev
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT runtime
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT runtime
  )
endif()

# --- C API headers (reader + writer + common) ---
install(DIRECTORY api/common/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/heif
  COMPONENT dev FILES_MATCHING PATTERN "*.h")
install(DIRECTORY api/reader/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/heif
  COMPONENT dev FILES_MATCHING PATTERN "*.h")
install(DIRECTORY api/writer/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/heif
  COMPONENT dev FILES_MATCHING PATTERN "*.h")

# --- C++ convenience API headers ---
install(DIRECTORY api-cpp/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/heif/heifpp
  COMPONENT dev FILES_MATCHING PATTERN "*.h")

# --- CMake package config (enables find_package(heif)) ---
install(EXPORT heifTargets
  FILE heifTargets.cmake
  NAMESPACE heif::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/heif
  COMPONENT dev
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/heifConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/heifConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/heifConfig.cmake"
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/heif
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/heifConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/heifConfigVersion.cmake"
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/heif
  COMPONENT dev
)

# --- pkg-config (heif.pc) ---
# Lets non-CMake build systems (Meson, Bazel, autotools, raw Make) discover
# the installed library via `pkg-config --cflags --libs heif`.
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/heif.pc.in"
  "${CMAKE_CURRENT_BINARY_DIR}/heif.pc"
  @ONLY
)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/heif.pc"
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
  COMPONENT dev
)

endif() # NOT HEIF_PYTHON_BINDING_ONLY
