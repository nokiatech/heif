# This file is part of Nokia HEIF library
#
# Copyright (c) 2015-2018 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
#
# Contact: heif@nokia.com
#
# This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its subsidiaries. All rights are reserved.
#
# Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior written consent of Nokia.

set(EXAMPLE_EXE example)

set(EXAMPLE_SRCS example.cpp)

add_executable(${EXAMPLE_EXE} ${EXAMPLE_SRCS})

set_property(TARGET ${EXAMPLE_EXE} PROPERTY CXX_STANDARD 11)

target_link_libraries(${EXAMPLE_EXE} heif_static heif_writer_static)


add_executable(${EXAMPLE_EXE}_shared ${EXAMPLE_SRCS})

set_property(TARGET ${EXAMPLE_EXE}_shared PROPERTY CXX_STANDARD 11)

target_link_libraries(${EXAMPLE_EXE}_shared heif_shared heif_writer_shared)
