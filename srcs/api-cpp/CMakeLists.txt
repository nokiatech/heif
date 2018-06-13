# This file is part of Nokia HEIF library
#
# Copyright (c) 2015-2018 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
#
# Contact: heif@nokia.com
#
# This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its subsidiaries. All rights are reserved.
#
# Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior written consent of Nokia.


cmake_minimum_required(VERSION 3.2 FATAL_ERROR)

set(HEIFPP_LIB_NAME heifpp)

project(${HEIFPP_LIB_NAME} LANGUAGES CXX)

add_library(${HEIFPP_LIB_NAME} STATIC 
    ${PROJECT_SOURCE_DIR}/Heif.h
    ${PROJECT_SOURCE_DIR}/Heif.cpp

    ${PROJECT_SOURCE_DIR}/EntityGroup.h
    ${PROJECT_SOURCE_DIR}/EntityGroup.cpp
    ${PROJECT_SOURCE_DIR}/EqivGroup.h
    ${PROJECT_SOURCE_DIR}/EqivGroup.cpp

    ${PROJECT_SOURCE_DIR}/AlternativeTrackGroup.h
    ${PROJECT_SOURCE_DIR}/AlternativeTrackGroup.cpp

    ${PROJECT_SOURCE_DIR}/DecoderConfiguration.h
    ${PROJECT_SOURCE_DIR}/DecoderConfiguration.cpp
    ${PROJECT_SOURCE_DIR}/AACDecoderConfiguration.h
    ${PROJECT_SOURCE_DIR}/AACDecoderConfiguration.cpp
    ${PROJECT_SOURCE_DIR}/AVCDecoderConfiguration.h
    ${PROJECT_SOURCE_DIR}/AVCDecoderConfiguration.cpp
    ${PROJECT_SOURCE_DIR}/HEVCDecoderConfiguration.h
    ${PROJECT_SOURCE_DIR}/HEVCDecoderConfiguration.cpp

    ${PROJECT_SOURCE_DIR}/Track.h
    ${PROJECT_SOURCE_DIR}/Track.cpp
    ${PROJECT_SOURCE_DIR}/AudioTrack.h
    ${PROJECT_SOURCE_DIR}/AudioTrack.cpp
    ${PROJECT_SOURCE_DIR}/VideoTrack.h
    ${PROJECT_SOURCE_DIR}/VideoTrack.cpp

    ${PROJECT_SOURCE_DIR}/ImageSequence.h
    ${PROJECT_SOURCE_DIR}/ImageSequence.cpp

    ${PROJECT_SOURCE_DIR}/Sample.h
    ${PROJECT_SOURCE_DIR}/Sample.cpp
    ${PROJECT_SOURCE_DIR}/AudioSample.h
    ${PROJECT_SOURCE_DIR}/AudioSample.cpp
    ${PROJECT_SOURCE_DIR}/VideoSample.h
    ${PROJECT_SOURCE_DIR}/VideoSample.cpp

    ${PROJECT_SOURCE_DIR}/ItemProperty.h
    ${PROJECT_SOURCE_DIR}/ItemProperty.cpp
    ${PROJECT_SOURCE_DIR}/RawProperty.h
    ${PROJECT_SOURCE_DIR}/RawProperty.cpp
    ${PROJECT_SOURCE_DIR}/DescriptiveProperty.h
    ${PROJECT_SOURCE_DIR}/DescriptiveProperty.cpp
    ${PROJECT_SOURCE_DIR}/TransformativeProperty.h
    ${PROJECT_SOURCE_DIR}/TransformativeProperty.cpp

    ${PROJECT_SOURCE_DIR}/Item.h
    ${PROJECT_SOURCE_DIR}/Item.cpp

    ${PROJECT_SOURCE_DIR}/MetaItem.h
    ${PROJECT_SOURCE_DIR}/MetaItem.cpp
    ${PROJECT_SOURCE_DIR}/ExifItem.h
    ${PROJECT_SOURCE_DIR}/ExifItem.cpp
    ${PROJECT_SOURCE_DIR}/MimeItem.h
    ${PROJECT_SOURCE_DIR}/MimeItem.cpp
    ${PROJECT_SOURCE_DIR}/XMPItem.h
    ${PROJECT_SOURCE_DIR}/XMPItem.cpp
    ${PROJECT_SOURCE_DIR}/MPEG7Item.h
    ${PROJECT_SOURCE_DIR}/MPEG7Item.cpp

    ${PROJECT_SOURCE_DIR}/ImageItem.h
    ${PROJECT_SOURCE_DIR}/ImageItem.cpp
    ${PROJECT_SOURCE_DIR}/CodedImageItem.h
    ${PROJECT_SOURCE_DIR}/CodedImageItem.cpp
    ${PROJECT_SOURCE_DIR}/AVCCodedImageItem.h
    ${PROJECT_SOURCE_DIR}/AVCCodedImageItem.cpp
    ${PROJECT_SOURCE_DIR}/HEVCCodedImageItem.h
    ${PROJECT_SOURCE_DIR}/HEVCCodedImageItem.cpp
    ${PROJECT_SOURCE_DIR}/JPEGCodedImageItem.h
    ${PROJECT_SOURCE_DIR}/JPEGCodedImageItem.cpp

    ${PROJECT_SOURCE_DIR}/DerivedImageItem.h
    ${PROJECT_SOURCE_DIR}/DerivedImageItem.cpp
    ${PROJECT_SOURCE_DIR}/IdentityImageItem.h
    ${PROJECT_SOURCE_DIR}/IdentityImageItem.cpp
    ${PROJECT_SOURCE_DIR}/GridImageItem.h
    ${PROJECT_SOURCE_DIR}/GridImageItem.cpp
    ${PROJECT_SOURCE_DIR}/OverlayImageItem.h
    ${PROJECT_SOURCE_DIR}/OverlayImageItem.cpp

    ${PROJECT_SOURCE_DIR}/ErrorCodes.h
    ${PROJECT_SOURCE_DIR}/ErrorCodes.cpp
    ${PROJECT_SOURCE_DIR}/H26xTools.h
    ${PROJECT_SOURCE_DIR}/H26xTools.cpp
    ${PROJECT_SOURCE_DIR}/helpers.h
    ${PROJECT_SOURCE_DIR}/helpers.cpp
)

set_property(TARGET ${HEIFPP_LIB_NAME} PROPERTY CXX_STANDARD 11)

target_include_directories(${HEIFPP_LIB_NAME} PUBLIC ${PROJECT_SOURCE_DIR})

if(IOS)
    if(${IOS_PLATFORM} STREQUAL "OS")
        set(HEIF_STATIC_LIB_NAME heif_static_iphoneos)
        set(HEIF_STATIC_WRITER_LIB_NAME heif_writer_static_iphoneos)
    elseif(${IOS_PLATFORM} STREQUAL "SIMULATOR64")
        set(HEIF_STATIC_LIB_NAME heif_static_iphonesimulator)
        set(HEIF_STATIC_WRITER_LIB_NAME heif_writer_static_iphonesimulator)
    endif()
else()
    set(HEIF_STATIC_LIB_NAME heif_static)
    set(HEIF_STATIC_WRITER_LIB_NAME heif_writer_static)
endif()

target_link_libraries(${HEIFPP_LIB_NAME} ${HEIF_STATIC_LIB_NAME} ${HEIF_STATIC_WRITER_LIB_NAME})
