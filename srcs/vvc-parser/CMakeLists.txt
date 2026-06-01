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

project( common LANGUAGES CXX )

set(VVCPARSER_LIB_NAME vvcparser)

set(VVC_PARSER_HEADERS
    vvcparser.hpp
    NAL/vvcrbspreader.hpp
    NAL/vvcnaluhelper.hpp
    NAL/vvcopiparser.hpp
    NAL/vvcppsparser.hpp
    NAL/vvcspsparser.hpp
    NAL/vvcvpsparser.hpp
)

set(VVC_PARSER_SOURCES
    vvcparser.cpp
    NAL/vvcrbspreader.cpp
    NAL/vvcnaluhelper.cpp
    NAL/vvcopiparser.cpp
    NAL/vvcppsparser.cpp
    NAL/vvcspsparser.cpp
    NAL/vvcvpsparser.cpp
)

add_library(${VVCPARSER_LIB_NAME} OBJECT ${VVC_PARSER_SOURCES} ${VVC_PARSER_HEADERS})
set_property(TARGET ${VVCPARSER_LIB_NAME} PROPERTY CXX_STANDARD 17)
set_property(TARGET ${VVCPARSER_LIB_NAME} PROPERTY POSITION_INDEPENDENT_CODE 1)
target_include_directories(${VVCPARSER_LIB_NAME} PUBLIC ../api/common)

# Mark this object library as part of the library build so HEIF_DLL_PUBLIC
# (see srcs/api/common/heifexport.h) collapses to nothing on MSVC instead of
# expanding to __declspec(dllimport) when these TUs include heiferrorinfo.h
# and reference HEIF::setErrorInfo from the recovery paths in the NAL
# parsers. Without this, MSVC emits LNK4217/LNK4286 cascades in any
# downstream target that statically links vvcparser. PRIVATE because the
# final reader/writer libraries control the consumer-facing macro choice
# via their own INTERFACE compile definitions.
target_compile_definitions(${VVCPARSER_LIB_NAME} PRIVATE HEIF_BUILDING_LIB)
