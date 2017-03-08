# Copyright (c) 2015-2017, Nokia Technologies Ltd.
# All rights reserved.
#
# Licensed under the Nokia High-Efficiency Image File Format (HEIF) License (the "License").
#
# You may not use the High-Efficiency Image File Format except in compliance with the License.
# The License accompanies the software and can be found in the file "LICENSE.TXT".
#
# You may also obtain the License at:
# https://nokiatech.github.io/heif/license.txt

include_directories(${PROJECT_BINARY_DIR})
include_directories(${PROJECT_SOURCE_DIR}/Srcs/common)
include_directories(${PROJECT_SOURCE_DIR}/Srcs/parser)
include_directories(${PROJECT_SOURCE_DIR}/Srcs/parser/avcparser)
include_directories(${PROJECT_SOURCE_DIR}/Srcs/parser/h265parser)

set(WRITER_SRCS
    auxiliaryimagewriter.cpp
    context.cpp
    datastore.cpp
    deriveditemmediawriter.cpp
    editwriter.cpp
    entitygroupwriter.cpp
    filemaker.cpp
    filewriter.cpp
    ftypwriter.cpp
    imagemasterwriter.cpp
    imagemediawriter.cpp
    imagethumbswriter.cpp
    layerimagewriter.cpp
    metaboxwriter.cpp
    metadatamediawriter.cpp
    metadatawriter.cpp
    metaderivedimagewriter.cpp
    metapropertywriter.cpp
    metawriter.cpp
    moovwriter.cpp
    primaryitemwriter.cpp
    refsgroup.cpp
    rootmetaimagewriter.cpp
    samplegroup.cpp
    services.cpp
    timeutility.cpp
    timewriter.cpp
    trackaltrepwriter.cpp
    trackmasterwriter.cpp
    trackthumbswriter.cpp
    trackwriter.cpp
    trefwriter.cpp)

add_library(heifwriter ${WRITER_SRCS})
target_link_libraries(heifwriter common h265parser)
