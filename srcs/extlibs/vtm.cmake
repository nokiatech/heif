# This file is part of Nokia HEIF library
#
# Copyright (c) 2015-2021 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
#
# Contact: heif@nokia.com
#
# This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its subsidiaries. All rights are reserved.
#
# Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior written consent of Nokia.


include(ExternalProject)
ExternalProject_Add(
    vtm_project
    SOURCE_DIR        "${PROJECT_SOURCE_DIR}/extlibs/VVCSoftware_VTM"
    CONFIGURE_COMMAND cmake ${PROJECT_SOURCE_DIR}/extlibs/VVCSoftware_VTM -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    BUILD_COMMAND     cmake --build . -t DecoderLib CommonLib
    INSTALL_COMMAND   echo Skipping install step for VTM.
    # Apply required patches to VTM. Use --forward switch and '|| true' to prevent error if the patch was already applied.
    # This is not ideal, but prevents build stalling while waiting for keyboard input.
    PATCH_COMMAND     patch --forward -p1 < ../0001-VTM-changes.patch || true
    TEST_COMMAND      ""
)

if(NOT CMAKE_BUILD_TYPE)
    set(VTM_BUILD_TYPE_DIR "release")
else()
    STRING(TOLOWER "${CMAKE_BUILD_TYPE}" VTM_BUILD_TYPE_DIR)
endif()

SET(VTM_LIB_DIR ${PROJECT_SOURCE_DIR}/extlibs/VVCSoftware_VTM/lib/${VTM_BUILD_TYPE_DIR}/)
SET(VTM_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/extlibs/VVCSoftware_VTM/source/Lib)

include_directories(${VTM_INCLUDE_DIR}
    ${VTM_INCLUDE_DIR}/CommonLib
    ${VTM_INCLUDE_DIR}/DecoderLib
    )

SET(VTM_LIBS libDecoderLib.so libCommonLib.so)
