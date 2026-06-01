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

# If built standalone (not as a subdirectory), define a project so
# PROJECT_VERSION is available for Doxyfile.in substitution.
# Version is intentionally NOT hardcoded here; the canonical source of
# truth is `srcs/CMakeLists.txt`. When built standalone the docs simply
# inherit a placeholder version; for a versioned doc build always invoke
# from the top-level project so PROJECT_VERSION is set by `srcs/`.
if(NOT PROJECT_NAME)
    project(heif-docs VERSION 0.0.0)
endif()

# ---------------------------------------------------------------------------
# Doxygen build target
# ---------------------------------------------------------------------------
find_package(Doxygen)

if(NOT DOXYGEN_FOUND)
    message(STATUS "HEIF docs: Doxygen not found, 'doc' target will not be built.")
    return()
endif()

# Diagrams (HAVE_DOT=YES in Doxyfile.in) need graphviz. Without it Doxygen
# silently degrades and the inheritance / collaboration diagrams are skipped.
if(NOT DOXYGEN_DOT_FOUND)
    message(WARNING
        "HEIF docs: graphviz 'dot' not found. "
        "Inheritance and collaboration diagrams will be omitted from the HTML output. "
        "Install graphviz (e.g. 'apt install graphviz' or 'brew install graphviz') for full diagrams.")
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in
               ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)

add_custom_target(doc ALL
    ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Building API HTML documentation with Doxygen" VERBATIM)
