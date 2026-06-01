cmake_minimum_required(VERSION 3.12)

project("heif-wasm")

# This project builds the HEIF C++ library into WebAssembly via Emscripten.
option(EmscriptenBuild "Turn on modifications when building with Emscripten." OFF)

# HEIF_ROOT: absolute path to the HEIF library source tree.
# Default assumes this binding lives at <repo>/bindings/javascript-wasm/.
# Override with -DHEIF_ROOT=/path/to/heif when building out-of-tree.
if(NOT HEIF_ROOT)
    get_filename_component(HEIF_ROOT "${CMAKE_SOURCE_DIR}/../.." ABSOLUTE)
endif()
message(STATUS "HEIF library root: ${HEIF_ROOT}")

# Build the reader API without its own root CMakeLists.txt to enable
# Emscripten-specific build flags.
# Get build version from git
execute_process(
    COMMAND "git" "describe"
    WORKING_DIRECTORY ${HEIF_ROOT}
    OUTPUT_VARIABLE GIT_DESCRIBE
    RESULT_VARIABLE GIT_ERROR
    OUTPUT_STRIP_TRAILING_WHITESPACE
    )

if(GIT_ERROR)
    message( "Could not get version information from git")
    set(GIT_DESCRIBE "v4.0.0")
endif()

# Parse MAJOR.MINOR.PATCH out of GIT_DESCRIBE (e.g. "v3.6.2-108-g8c8ef7ee" -> 3,6,2)
# so the @PROJECT_VERSION_*@ substitutions in buildinfo.hpp.in produce valid
# preprocessor tokens. Upstream's srcs/CMakeLists.txt gets these for free from
# its own `project(heif VERSION 4.0.0 ...)` call, but out-of-tree consumers
# (us, via add_subdirectory) have to populate them ourselves or HEIF_VERSION_*
# expand to empty and break heif_version_c.cpp.
if(GIT_DESCRIBE MATCHES "^v?([0-9]+)\\.([0-9]+)\\.([0-9]+)")
    set(PROJECT_VERSION_MAJOR ${CMAKE_MATCH_1})
    set(PROJECT_VERSION_MINOR ${CMAKE_MATCH_2})
    set(PROJECT_VERSION_PATCH ${CMAKE_MATCH_3})
else()
    set(PROJECT_VERSION_MAJOR 0)
    set(PROJECT_VERSION_MINOR 0)
    set(PROJECT_VERSION_PATCH 0)
endif()
message(STATUS "HEIF library version parsed from git: "
        "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")

# Get build timestamp
string(TIMESTAMP BUILD_TIMESTAMP UTC)
# Create the version number header.
# Generate into the build tree (mirroring the upstream srcs/CMakeLists.txt
# configure_file call) rather than writing into the source tree. The
# build-tree location is on the include path so that translation units
# pulled in cross-directory -- notably heif_version_c.cpp compiled as
# ../common/heif_version_c.cpp -- can resolve `#include "buildinfo.hpp"`
# via the include path instead of relying on same-directory lookup.
configure_file("${HEIF_ROOT}/srcs/buildinfo/buildinfo.hpp.in" "${CMAKE_BINARY_DIR}/buildinfo.hpp")
include_directories("${CMAKE_BINARY_DIR}")
include_directories("${HEIF_ROOT}/srcs/api/common")
include_directories("${HEIF_ROOT}/srcs/common")
include_directories("${HEIF_ROOT}/srcs/api/reader")


# Emscripten does not support shared libraries
set(BUILD_ONLY_STATIC_LIB ON)

# Enable C++ exception support for Emscripten builds.
# Wasm-native exceptions (-fwasm-exceptions) are faster and produce smaller
# binaries than the legacy JS-based approach (-fexceptions +
# -sDISABLE_EXCEPTION_CATCHING=0).  Requires Emscripten >=3.1.28 (we use 4.0+)
# and a browser with the Wasm Exception Handling proposal (Chrome 95+,
# Firefox 100+, Safari 15.2+).  The library's try/catch blocks compile to
# native wasm EH instructions; no JS-side catch table overhead.
if(EmscriptenBuild)
    add_compile_options(-fwasm-exceptions)
    # -Wno-ignored-attributes is not needed: the HEIF_DLL_PUBLIC /
    # HEIF_DLL_FN_EXPORT macro split in heifexport.h ensures that under
    # __EMSCRIPTEN__, HEIF_DLL_PUBLIC expands to nothing (applied to
    # types) and HEIF_DLL_FN_EXPORT carries EMSCRIPTEN_KEEPALIVE (applied
    # to functions only). A clean rebuild produces zero
    # -Wignored-attributes warnings.
endif()

add_subdirectory("${HEIF_ROOT}/srcs/vvc-parser" "${CMAKE_BINARY_DIR}/vvc-parser" EXCLUDE_FROM_ALL)
add_subdirectory("${HEIF_ROOT}/srcs/common" "${CMAKE_BINARY_DIR}/heif-common" EXCLUDE_FROM_ALL)
add_subdirectory("${HEIF_ROOT}/srcs/reader" "${CMAKE_BINARY_DIR}/heif-reader" EXCLUDE_FROM_ALL)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})

# heif.js: pure-link binding against the C facade
#
# We do NOT compile any C/C++ source of our own. The HEIF library's static
# archive already exposes every public C entry point with EMSCRIPTEN_KEEPALIVE
# (via HEIF_DLL_FN_EXPORT in heifexport.h). Linking with --whole-archive
# forces every archive member to be included; KEEPALIVE then prevents
# emcc's DCE from stripping the entry points. Result: a single .js module
# whose Module._heif_* surface mirrors heifreader_c.h 1:1, automatically,
# with no per-function bookkeeping in this file.
if(EmscriptenBuild)
    set(HEIF_JS_RAW ${CMAKE_BINARY_DIR}/heif-raw.js)
    add_custom_command(
        OUTPUT ${HEIF_JS_RAW}
        COMMAND ${CMAKE_C_COMPILER}
            -Wl,--whole-archive
                $<TARGET_FILE:heif_static>
            -Wl,--no-whole-archive
            --no-entry
            -o ${HEIF_JS_RAW}
            -O2
            -sMODULARIZE=1
            -sEXPORT_NAME=HEIFModule
            -sALLOW_MEMORY_GROWTH=1
            -sINITIAL_HEAP=134217728
            -sSTACK_SIZE=4194304
            -sALLOW_TABLE_GROWTH=1
            -sWASM_BIGINT=1
            -sEXPORTED_FUNCTIONS=[_malloc,_free]
            -sEXPORTED_RUNTIME_METHODS=[cwrap,ccall,HEAPU8,HEAPU16,HEAPU32,HEAP8,HEAP16,HEAP32,setValue,getValue,addFunction,removeFunction,UTF8ToString,stringToUTF8,lengthBytesUTF8]
            -fwasm-exceptions
            -sASSERTIONS=1
        DEPENDS heif_static
        COMMENT "Linking heif-raw.js (--whole-archive, --no-entry, no glue source)"
        VERBATIM
    )
    add_custom_target(heif-raw ALL DEPENDS ${HEIF_JS_RAW})

    # Stage the deployable bundle in <binding>/dist/ as a single
    # drop-in directory. Runs on every build; the four binary
    # files are .gitignored in dist/, only dist/README.md and
    # dist/.gitignore are tracked. Downstream consumers can either
    # copy dist/* into their project, or use `cmake --install`
    # (or `build-with-emcmake.sh --prefix ...`) to deploy elsewhere.
    set(HEIF_JS_DIST_DIR ${CMAKE_SOURCE_DIR}/dist)
    add_custom_target(heif-dist ALL
        COMMAND ${CMAKE_COMMAND} -E make_directory ${HEIF_JS_DIST_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy
            ${HEIF_JS_RAW}
            ${CMAKE_BINARY_DIR}/heif-raw.wasm
            ${CMAKE_SOURCE_DIR}/heif.js
            ${CMAKE_SOURCE_DIR}/heif.d.ts
            ${CMAKE_SOURCE_DIR}/package.json
            ${HEIF_JS_DIST_DIR}/
        COMMAND ${CMAKE_COMMAND} -E echo
            "-- Staged deployable bundle in ${HEIF_JS_DIST_DIR}/"
        COMMAND ${CMAKE_COMMAND} -E echo
            "--   heif-raw.js  heif-raw.wasm  heif.js  heif.d.ts  package.json"
        DEPENDS heif-raw
        COMMENT "Staging deployable bundle in dist/"
    )

    # Install rule: copy the four binaries + package.json + the
    # bundle README into <prefix>/. README.md is included so the
    # installed directory is also self-documenting.
    install(FILES
        ${HEIF_JS_RAW}
        ${CMAKE_BINARY_DIR}/heif-raw.wasm
        ${CMAKE_SOURCE_DIR}/heif.js
        ${CMAKE_SOURCE_DIR}/heif.d.ts
        ${CMAKE_SOURCE_DIR}/package.json
        ${HEIF_JS_DIST_DIR}/README.md
        DESTINATION .
    )
endif()
