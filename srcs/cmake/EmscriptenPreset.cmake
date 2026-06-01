# srcs/cmake/EmscriptenPreset.cmake
#
# Convenience CMake include for wasm/JS consumers.
#
# This file is NOT a CMake toolchain file (emcc supplies its own via
# `emcmake`). It is a small set of safe defaults that a wasm consumer
# can `include()` from their own CMakeLists.txt to avoid having to
# discover the right combination of HEIF_* options by trial and error.
#
# Typical usage from a sibling repo:
#
#   # Make sure HEIF source is reachable; e.g. as a git submodule under ../heif/srcs
#   include(${CMAKE_CURRENT_SOURCE_DIR}/../heif/srcs/cmake/EmscriptenPreset.cmake)
#   add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../heif/srcs heif_build EXCLUDE_FROM_ALL)
#
#   add_executable(libreader my_wrapper.cpp)
#   target_link_libraries(libreader PRIVATE heif_static)
#
#   # Your emcc link options.  Note: NO `EXPORTED_FUNCTIONS=[...]` list is
#   # needed -- every public `heif_*` C-facade entry self-exports through
#   # EMSCRIPTEN_KEEPALIVE (see srcs/api/common/heifexport.h).
#   target_link_options(libreader PRIVATE
#       -sMODULARIZE=1
#       -sEXPORT_NAME=HEIFReaderModule
#       -sALLOW_MEMORY_GROWTH=1
#       -sEXPORTED_RUNTIME_METHODS=['ccall','cwrap','HEAPU8']
#   )
#
# The defaults below are deliberately the SMALLEST set that makes a wasm
# build of this library work.  They are *opinionated* (no shared library,
# no test/example targets) but they match what every wasm build will need.
# Override any of them BEFORE add_subdirectory() if you have a reason.

if(NOT EMSCRIPTEN)
    message(WARNING
        "EmscriptenPreset.cmake included from a non-Emscripten build "
        "(CMAKE_C_COMPILER_ID='${CMAKE_C_COMPILER_ID}'). "
        "This file is only meaningful under `emcmake cmake ...`. "
        "No defaults will be applied.")
    return()
endif()

# 1.  emcc cannot produce shared libraries -- only `.wasm` modules and
#     side modules.  Force static-only so the library doesn't try to
#     build heif_shared / heif_writer_shared targets that wasm-ld
#     would reject.
set(BUILD_ONLY_STATIC_LIB ON CACHE BOOL
    "Wasm: emcc has no .so concept; build static archive only" FORCE)

# 2.  Skip example and tool targets.  These pull in tools like
#     `readerapp` that have no meaning in a browser
#     and would only inflate the wasm bundle if accidentally linked.
set(NO_EXAMPLES ON CACHE BOOL "Wasm: skip in-tree example targets" FORCE)
set(NO_TOOLS    ON CACHE BOOL "Wasm: skip CLI utility targets"     FORCE)

# 3.  Modern wasm exception model.  Smaller and ~free at runtime
#     compared to the legacy `-sDISABLE_EXCEPTION_CATCHING=0` JS path.
#     Requires emcc >= 3.1.45.  Consumers stuck on older emcc can
#     override this AFTER include() with `add_compile_options(-fexceptions)`.
add_compile_options(-fwasm-exceptions)
add_link_options(-fwasm-exceptions)

# 4.  Optimise for size by default.  Wasm payload size is the single
#     biggest determinant of cold-load latency in a browser.  Override
#     to -O2 or -O3 if you need raw throughput more than bytes-on-the-wire.
add_compile_options(-Os)
add_link_options(-Os)

# 5.  Hidden visibility is already on for every GNU/Clang build via
#     srcs/CMakeLists.txt; emcc treats hidden as effectively local for
#     wasm-ld, so combined with the EMSCRIPTEN_KEEPALIVE tagging in
#     heifexport.h the export set is exactly the public C facade and
#     nothing else.  No extra flag needed here -- kept as a comment so
#     consumers know not to override -fvisibility=default.

message(STATUS "HEIF: Emscripten preset applied "
               "(static-only, tests/examples/tools off, wasm-EH on, -Os).")
