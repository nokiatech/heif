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

include_directories(${PROJECT_SOURCE_DIR}/Srcs/common)
include_directories(${PROJECT_SOURCE_DIR}/Srcs/parser)
include_directories(${PROJECT_SOURCE_DIR}/Srcs/parser/avcparser)
include_directories(${PROJECT_SOURCE_DIR}/Srcs/parser/h265parser)

set(COMMON_SRCS
    auxiliarytypeproperty.cpp
    avcconfigurationbox.cpp
    avcdecoderconfigrecord.cpp
    avcsampleentry.cpp
    bbox.cpp
    bitstream.cpp
    chunkoffsetbox.cpp
    cleanaperture.cpp
    codingconstraintsbox.cpp
    compositionoffsetbox.cpp
    compositiontodecodebox.cpp
    datainformationbox.cpp
    datareferencebox.cpp
    decodepts.cpp
    editbox.cpp
    entitytogroupbox.cpp
    filetypebox.cpp
    fourccint.cpp
    fullbox.cpp
    groupslistbox.cpp
    handlerbox.cpp
    hevcconfigurationbox.cpp
    hevcdecoderconfigrecord.cpp
    hevcsampleentry.cpp
    idspace.cpp
    imagemirror.cpp
    imagespatialextentsproperty.cpp
    imagerotation.cpp
    imagerelativelocationproperty.cpp
    imagegrid.cpp
    imageoverlay.cpp
    itemdatabox.cpp
    iteminfobox.cpp
    itemlocationbox.cpp
    itempropertiesbox.cpp
    itempropertyassociation.cpp
    itempropertycontainer.cpp
    itemprotectionbox.cpp
    itemreferencebox.cpp
    layeredhevcconfigurationitemproperty.cpp
    layerselectorproperty.cpp
    lhevcdecoderconfigrecord.cpp
    log.cpp
    mediabox.cpp
    mediadatabox.cpp
    mediaheaderbox.cpp
    mediainformationbox.cpp
    metabox.cpp
    moviebox.cpp
    movieheaderbox.cpp
    operatingpointsinformation.cpp
    primaryitembox.cpp
    protectionschemeinfobox.cpp
    sampledescriptionbox.cpp
    sampleentrybox.cpp
    samplesizebox.cpp
    sampletablebox.cpp
    sampletochunkbox.cpp
    sampletogroupbox.cpp
    samplegroupdescriptionbox.cpp
    samplegroupentry.cpp
    syncsamplebox.cpp
    targetolsproperty.cpp
    timetosamplebox.cpp
    trackbox.cpp
    trackheaderbox.cpp
    trackreferencebox.cpp
    trackreferencetypebox.cpp
    videomediaheaderbox.cpp
    visualsampleentrybox.cpp)

# Also show these standalone header files in the project tree
set(COMMON_STANDALONE_HDRS
    androidhacks.hpp
    avccommondefs.hpp
    hevccommondefs.hpp)

add_library(common ${COMMON_SRCS} ${COMMON_STANDALONE_HDRS})
target_link_libraries(common h265parser avcparser)
