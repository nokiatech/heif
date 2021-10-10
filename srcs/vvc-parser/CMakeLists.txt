# This file is part of Nokia HEIF library
#
# Copyright (c) 2015-2021 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
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

project( common LANGUAGES CXX )

set(VVCPARSER_LIB_NAME vvcparser)

set(VVC_PARSER_SRCS
    vvcparser.cpp
    )

set(VVC_PARSER_HDRS
    vvcparser.hpp
    )

add_library(${VVCPARSER_LIB_NAME} OBJECT ${VVC_PARSER_SRCS} ${VVC_PARSER_HDRS})

set_property(TARGET ${VVCPARSER_LIB_NAME} PROPERTY CXX_STANDARD 11)
set_property(TARGET ${VVCPARSER_LIB_NAME} PROPERTY POSITION_INDEPENDENT_CODE 1)

target_include_directories(${VVCPARSER_LIB_NAME} PUBLIC ../api/common ${VTM_INCLUDE_DIRS})
target_link_directories(${VVCPARSER_LIB_NAME} PUBLIC ${VTM_LIB_DIR})
target_link_libraries(${VVCPARSER_LIB_NAME} PUBLIC ${VTM_LIBS})
add_dependencies(${VVCPARSER_LIB_NAME} vtm_project)
