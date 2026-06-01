# This file is part of Nokia HEIF library
#
# Copyright (c) 2015-2026 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
#
# Contact: heif@nokia.com
#
# This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its subsidiaries. All rights are reserved.
#
# Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior written consent of Nokia.

# Required version of CMake
cmake_minimum_required(VERSION 3.5...3.28 FATAL_ERROR)

set(READER_EXE readerapp)

set(READERAPP_SRCS
    readerapp.cpp)

add_executable(${READER_EXE} ${READERAPP_SRCS})
set_property(TARGET ${READER_EXE} PROPERTY CXX_STANDARD 17)
target_compile_definitions(${READER_EXE} PUBLIC HEIF_USE_STATIC_LIB)
target_include_directories(${READER_EXE} PRIVATE ${PROJECT_SOURCE_DIR}/common)
target_link_libraries(${READER_EXE} heif_static)
