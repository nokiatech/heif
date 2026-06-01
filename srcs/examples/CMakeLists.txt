# This file is part of Nokia HEIF library
#
# Copyright (c) 2015-2026 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
#
# Contact: heif@nokia.com
#
# This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its subsidiaries. All rights are reserved.
#
# Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior written consent of Nokia.

set(EXAMPLE_EXE example)

set(EXAMPLE_SRCS example.cpp)

add_executable(${EXAMPLE_EXE} ${EXAMPLE_SRCS})

set_property(TARGET ${EXAMPLE_EXE} PROPERTY CXX_STANDARD 17)

target_link_libraries(${EXAMPLE_EXE} heif_static heif_writer_static)

if(NOT BUILD_ONLY_STATIC_LIB)
    add_executable(${EXAMPLE_EXE}_shared ${EXAMPLE_SRCS})
    set_property(TARGET ${EXAMPLE_EXE}_shared PROPERTY CXX_STANDARD 17)
    target_link_libraries(${EXAMPLE_EXE}_shared heif_shared heif_writer_shared)
endif()

# heifpp_example -- worked examples for the HEIFPP C++ wrapper
# (`heif::heifpp` / in-tree `heifpp` static lib). Mirrors what the
# Java HEIFExample.java demonstrates, plus a HEIFPP error-model demo.
# Linking only `heifpp` is sufficient: it transitively pulls in
# `heif_static` + `heif_writer_static` (see srcs/api-cpp/CMakeLists.txt
# `target_link_libraries(${HEIFPP_LIB_NAME} ...)`).
set(HEIFPP_EXAMPLE_EXE heifpp_example)
add_executable(${HEIFPP_EXAMPLE_EXE} heifpp_example.cpp)
set_property(TARGET ${HEIFPP_EXAMPLE_EXE} PROPERTY CXX_STANDARD 17)
target_link_libraries(${HEIFPP_EXAMPLE_EXE} heifpp)
