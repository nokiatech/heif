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

if(${CMAKE_EXTRA_GENERATOR} MATCHES "Eclipse CDT4")
    set(CMAKE_ECLIPSE_VERSION "4.4" CACHE STRING "Eclipse version" FORCE)
    set(CMAKE_CXX_COMPILER_ARG1 "-std=c++11" CACHE STRING "C++ version for eclipse" FORCE)
endif()

project( common LANGUAGES CXX )

set(COMMON_SRCS
    audiosampleentrybox.cpp
    auxiliarytypeinfobox.cpp
    auxiliarytypeproperty.cpp
    avcconfigurationbox.cpp
    avcdecoderconfigrecord.cpp
    avcparser.cpp
    avcsampleentry.cpp
    bbox.cpp
    bitstream.cpp
    channellayoutbox.cpp
    chunkoffsetbox.cpp
    cleanaperturebox.cpp
    codingconstraintsbox.cpp
    colourinformationbox.cpp
    compositionoffsetbox.cpp
    compositiontodecodebox.cpp
    datainformationbox.cpp
    datareferencebox.cpp
    decodepts.cpp
    directreferencesampleslist.cpp
    editbox.cpp
    elementarystreamdescriptorbox.cpp
    entitytogroupbox.cpp
    filetypebox.cpp
    fourccint.cpp
    freespacebox.cpp
    fullbox.cpp
    groupslistbox.cpp
    handlerbox.cpp
    hevcconfigurationbox.cpp
    hevcdecoderconfigrecord.cpp
    hevcsampleentry.cpp
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
    jpegconfigurationbox.cpp
    jpegparser.cpp
    log.cpp
    mediabox.cpp
    mediadatabox.cpp
    mediaheaderbox.cpp
    mediainformationbox.cpp
    metabox.cpp
    moviebox.cpp
    movieheaderbox.cpp
    mp4audiodecoderconfigrecord.cpp
    mp4audiosampleentrybox.cpp
    nalutil.cpp
    nullmediaheaderbox.cpp
    pixelaspectratiobox.cpp
    pixelinformationproperty.cpp
    primaryitembox.cpp
    protectionschemeinfobox.cpp
    rawpropertybox.cpp
    sampledescriptionbox.cpp
    sampleentrybox.cpp
    samplegroupdescriptionentry.cpp
    samplesizebox.cpp
    sampletablebox.cpp
    sampletochunkbox.cpp
    sampletogroupbox.cpp
    sampletometadataitementry.cpp
    samplegroupdescriptionbox.cpp
    samplingratebox.cpp
    soundmediaheaderbox.cpp
    syncsamplebox.cpp
    timetosamplebox.cpp
    trackbox.cpp
    trackheaderbox.cpp
    trackreferencebox.cpp
    trackreferencetypebox.cpp
    trackrunbox.cpp
    videomediaheaderbox.cpp
    visualequivalenceentry.cpp
    visualsampleentrybox.cpp
    )

set(COMMON_HDRS
    customallocator.hpp
    audiosampleentrybox.hpp
    avccommondefs.hpp
    avcconfigurationbox.hpp
    avcdecoderconfigrecord.hpp
    avcsampleentry.hpp
    auxiliarytypeinfobox.hpp
    auxiliarytypeproperty.hpp
    bbox.hpp
    bitstream.hpp
    channellayoutbox.hpp
    chunkoffsetbox.hpp
    cleanaperturebox.hpp
    codingconstraintsbox.hpp
    colourinformationbox.hpp
    compositionoffsetbox.hpp
    compositiontodecodebox.hpp
    datainformationbox.hpp
    datareferencebox.hpp
    decodepts.hpp
    decoderconfigrecord.hpp
    decoderconfigurationbox.hpp
    directreferencesampleslist.hpp
    editbox.hpp
    elementarystreamdescriptorbox.hpp
    entitytogroupbox.hpp
    filetypebox.hpp
    fourccint.hpp
    freespacebox.hpp
    fullbox.hpp
    groupslistbox.hpp
    handlerbox.hpp
    hevccommondefs.hpp
    hevcconfigurationbox.hpp
    hevcdecoderconfigrecord.hpp
    hevcsampleentry.hpp
    imagegrid.hpp
    imagemirror.hpp
    imageoverlay.hpp
    imagerelativelocationproperty.hpp
    imagerotation.hpp
    imagespatialextentsproperty.hpp
    itemdatabox.hpp
    iteminfobox.hpp
    itemlocationbox.hpp
    itempropertiesbox.hpp
    itempropertyassociation.hpp
    itempropertycontainer.hpp
    itemprotectionbox.hpp
    itemreferencebox.hpp
    jpegconfigurationbox.hpp
    jpegparser.hpp
    log.hpp
    mediabox.hpp
    mediadatabox.hpp
    mediaheaderbox.hpp
    mediainformationbox.hpp
    metabox.hpp
    moviebox.hpp
    moviefragmentsdatatypes.hpp
    movieheaderbox.hpp
    mp4audiosampleentrybox.hpp
    mp4audiodecoderconfigrecord.hpp
    nalutil.hpp
    nullmediaheaderbox.hpp
    pixelaspectratiobox.hpp
    pixelinformationproperty.hpp
    primaryitembox.hpp
    protectionschemeinfobox.hpp
    rawpropertybox.hpp
    sampledescriptionbox.hpp
    sampleentrybox.hpp
    samplegroupdescriptionbox.hpp
    samplegroupdescriptionentry.hpp
    samplesizebox.hpp
    sampletablebox.hpp
    sampletochunkbox.hpp
    sampletogroupbox.hpp
    sampletometadataitementry.hpp
    samplingratebox.hpp
    smallvector.hpp
    soundmediaheaderbox.hpp
    syncsamplebox.hpp
    timetosamplebox.hpp
    trackbox.hpp
    trackheaderbox.hpp
    trackreferencebox.hpp
    trackreferencetypebox.hpp
    trackrunbox.hpp
    videomediaheaderbox.hpp
    visualequivalenceentry.hpp
    visualsampleentrybox.hpp
    writeoncemap.hpp
    )

add_library(common OBJECT ${COMMON_SRCS} ${COMMON_HDRS})
set_property(TARGET common PROPERTY CXX_STANDARD 11)

set_property(TARGET common PROPERTY POSITION_INDEPENDENT_CODE 1)