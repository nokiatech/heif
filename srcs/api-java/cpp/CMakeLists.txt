# This file is part of Nokia HEIF library
#
# Copyright (c) 2015-2018 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
#
# Contact: heif@nokia.com
#
# This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its subsidiaries. All rights are reserved.
#
# Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior written consent of Nokia.

cmake_minimum_required(VERSION 3.4.1)

project(heifjni LANGUAGES CXX C)

if (TARGET heifpp)
    message("using existing heifpp target")
else()
    add_subdirectory("../../" ${PROJECT_BINARY_DIR}/heif)
endif()

set(api_cpp_path "../../api-cpp")
if(NOT ANDROID)
    find_package(JNI REQUIRED)
    include_directories(${JNI_INCLUDE_DIRS})
endif()
set(sources
    CleanAperturePropertyJNI.cpp
    HEIFJNI.cpp
    Helpers.cpp
    Helpers.h
    ImageItemJNI.cpp
    ItemJNI.cpp
    ItemPropertyJNI.cpp
    GridImageItemJNI.cpp
    CodedImageItemJNI.cpp
    DerivedImageItemJNI.cpp
    DecoderConfigJNI.cpp
    HEVCImageItemJNI.cpp
    RawPropertyJNI.cpp
    RotatePropertyJNI.cpp
    PixelInformationPropertyJNI.cpp
    PixelAspectRatioJNI.cpp
    RelativeLocationPropertyJNI.cpp
    OverlayImageItemJNI.cpp
    AuxiliaryPropertyJNI.cpp
    MirrorPropertyJNI.cpp
    ColourInformationPropertyJNI.cpp
    IdentityImageItemJNI.cpp
    AVCDecoderConfigJNI.cpp
    AVCImageItemJNI.cpp
    HEVCDecoderConfigJNI.cpp
    MPEG7ItemJNI.cpp
    XMPItemJNI.cpp
    MimeItemJNI.cpp
    ExifItemJNI.cpp
    InputStream.cpp
    OutputStream.cpp
    AudioTrackJNI.cpp
    VideoTrackJNI.cpp
    TrackJNI.cpp
    SampleJNI.cpp
    HEVCSampleJNI.cpp
    AACDecoderConfigJNI.cpp
    AACSampleJNI.cpp
    AVCSampleJNI.cpp
    AlternativeTrackGroupJNI.cpp
    EntityGroupJNI.cpp
    EquivalenceGroupJNI.cpp
    AlternateGroupJNI.cpp
    StereoPairGroupJNI.cpp
    ImageSequenceJNI.cpp
    )


add_library( # Sets the name of the library.
             heifjni
             # Sets the library as a shared library.
             SHARED
             ${sources}
             )

target_link_libraries(heifjni heifpp)
