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

set(COMMON_SRCS
    accessibilitytext.cpp
    ambientviewingenvironmentbox.cpp
    audiosampleentrybox.cpp
    auxiliarytypeinfobox.cpp
    auxiliarytypeproperty.cpp
    avcconfigurationbox.cpp
    avcdecoderconfigrecord.cpp
    avcparser.cpp
    avcsampleentry.cpp
    bbox.cpp
    bitstream.cpp
    boxfactory.cpp
    brandandcompatiblebrandsbasebox.cpp
    channellayoutbox.cpp
    chunkoffsetbox.cpp
    cleanaperturebox.cpp
    codingconstraintsbox.cpp
    colourinformationbox.cpp
    compositionoffsetbox.cpp
    compositiontodecodebox.cpp
    contentlightlevelbox.cpp
    contentcolourvolumebox.cpp
    creationtimeinformation.cpp
    datainformationbox.cpp
    datareferencebox.cpp
    decodepts.cpp
    directreferencesampleslist.cpp
    editbox.cpp
    elementarystreamdescriptorbox.cpp
    entitytogroupbox.cpp
    extendedtypebox.cpp
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
    imagescaling.cpp
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
    layerselectorproperty.cpp
    lhevcconfigurationbox.cpp
    lhevcdecoderconfigrecord.cpp
    log.cpp
    masteringdisplaycolourvolumebox.cpp
    mediabox.cpp
    mediadatabox.cpp
    mediaheaderbox.cpp
    mediainformationbox.cpp
    metabox.cpp
    modificationtimeinformation.cpp
    moviebox.cpp
    movieextendsbox.cpp
    movieextendsheaderbox.cpp
    moviefragmentbox.cpp
    moviefragmentheaderbox.cpp
    movieheaderbox.cpp
    mp4audiodecoderconfigrecord.cpp
    mp4audiosampleentrybox.cpp
    nalutil.cpp
    nominaldiffusewhitebox.cpp
    nullmediaheaderbox.cpp
    operatingpointsinformationproperty.cpp
    pixelaspectratiobox.cpp
    pixelinformationproperty.cpp
    primaryitembox.cpp
    protectionschemeinfobox.cpp
    rawpropertybox.cpp
    rectangularregiongroupentry.cpp
    referenceviewingenvironmentbox.cpp
    requiredreferencetypesproperty.cpp
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
    segmentindexbox.cpp
    segmenttypebox.cpp
    soundmediaheaderbox.cpp
    subsampleinformationproperty.cpp
    syncsamplebox.cpp
    targetolsproperty.cpp
    timetosamplebox.cpp
    tonemapimage.cpp
    trackbox.cpp
    trackextendsbox.cpp
    trackfragmentbasemediadecodetimebox.cpp
    trackfragmentbox.cpp
    trackfragmentheaderbox.cpp
    trackgroupbox.cpp
    trackgrouptypebox.cpp
    trackheaderbox.cpp
    trackreferencebox.cpp
    trackreferencetypebox.cpp
    trackrunbox.cpp
    tracktypebox.cpp
    typecombinationbox.cpp
    userdescriptionproperty.cpp
    videomediaheaderbox.cpp
    visualequivalenceentry.cpp
    visualsampleentrybox.cpp
    vvcconfigurationbox.cpp
    vvcdecoderconfigrecord.cpp
    vvcnaluconfigbox.cpp
    vvcsampleentry.cpp
    vvcsubpicidentry.cpp
    vvcsubpicorderentry.cpp
    vvcsubpicsampleentry.cpp
    vvcsubpicturelayoutmapentry.cpp
    vvcmixednalunittypepicentry.cpp
    )


set(COMMON_HDRS
    accessibilitytext.hpp
    ambientviewingenvironmentbox.hpp
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
    boxfactory.hpp
    brandandcompatiblebrandsbasebox.hpp
    channellayoutbox.hpp
    chunkoffsetbox.hpp
    cleanaperturebox.hpp
    codingconstraintsbox.hpp
    colourinformationbox.hpp
    compositionoffsetbox.hpp
    compositiontodecodebox.hpp
    contentlightlevelbox.hpp
    contentcolourvolumebox.hpp
    creationtimeinformation.hpp
    datainformationbox.hpp
    datareferencebox.hpp
    decodepts.hpp
    decoderconfigrecord.hpp
    decoderconfigurationbox.hpp
    directreferencesampleslist.hpp
    editbox.hpp
    elementarystreamdescriptorbox.hpp
    entitytogroupbox.hpp
    extendedtypebox.hpp
    filetypebox.hpp
    fourccint.hpp
    freespacebox.hpp
    fullbox.hpp
    groupslistbox.hpp
    handlerbox.hpp
    ../api/common/heiferrorinfo.h
    hevccommondefs.hpp
    hevcconfigurationbox.hpp
    hevcdecoderconfigrecord.hpp
    hevcsampleentry.hpp
    imagegrid.hpp
    imagemirror.hpp
    imageoverlay.hpp
    imagerelativelocationproperty.hpp
    imagerotation.hpp
    imagescaling.hpp
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
    layerselectorproperty.hpp
    lhevcconfigurationbox.hpp
    lhevcdecoderconfigrecord.hpp
    log.hpp
    masteringdisplaycolourvolumebox.hpp
    mediabox.hpp
    mediadatabox.hpp
    mediaheaderbox.hpp
    mediainformationbox.hpp
    metabox.hpp
    modificationtimeinformation.hpp
    moviebox.hpp
    movieextendsbox.hpp
    movieextendsheaderbox.hpp
    moviefragmentbox.cpp
    moviefragmentheaderbox.cpp
    moviefragmentsdatatypes.hpp
    movieheaderbox.hpp
    mp4audiosampleentrybox.hpp
    mp4audiodecoderconfigrecord.hpp
    nalutil.hpp
    nominaldiffusewhitebox.hpp
    nullmediaheaderbox.hpp
    pixelaspectratiobox.hpp
    pixelinformationproperty.hpp
    primaryitembox.hpp
    protectionschemeinfobox.hpp
    rawpropertybox.hpp
    referenceviewingenvironmentbox.hpp
    rectangularregiongroupentry.hpp
    requiredreferencetypesproperty.hpp
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
    segmentindexbox.hpp
    segmenttypebox.hpp
    smallvector.hpp
    soundmediaheaderbox.hpp
    subsampleinformationproperty.hpp
    syncsamplebox.hpp
    targetolsproperty.hpp
    timetosamplebox.hpp
    tonemapimage.hpp
    trackbox.hpp
    trackextendsbox.hpp
    trackfragmentbasemediadecodetimebox.cpp
    trackfragmentbox.hpp
    trackfragmentheaderbox.hpp
    trackgroupbox.hpp
    trackgrouptypebox.hpp
    trackheaderbox.hpp
    trackreferencebox.hpp
    trackreferencetypebox.hpp
    trackrunbox.hpp
    tracktypebox.hpp
    typecombinationbox.hpp
    userdescriptionproperty.hpp
    videomediaheaderbox.hpp
    visualequivalenceentry.hpp
    visualsampleentrybox.hpp
    vvccommondefs.hpp
    vvcconfigurationbox.hpp
    vvcdecoderconfigrecord.hpp
    vvcnaluconfigbox.hpp
    vvcsampleentry.hpp
    vvcsubpicidentry.hpp
    vvcsubpicorderentry.hpp
    vvcsubpicsampleentry.hpp
    vvcsubpicturelayoutmapentry.hpp
    vvcmixednalunittypepicentry.hpp
    writeoncemap.hpp
    )

add_library(common OBJECT ${COMMON_SRCS} ${COMMON_HDRS})
set_property(TARGET common PROPERTY CXX_STANDARD 17)

# On Windows the OBJECT library must define HEIF_BUILDING_LIB so that
# HEIF_DLL_PUBLIC expands to nothing (not __declspec(dllimport)).
# On non-Windows we must NOT define it, because the -fvisibility=hidden
# flag is in effect and HEIF_DLL_PUBLIC must expand to
# __attribute__((visibility("default"))) for symbols that belong in the
# shared-library ABI surface (e.g. the trace-sink API in log.cpp).
if(WIN32)
    target_compile_definitions(common PRIVATE HEIF_BUILDING_LIB)
endif()

set_property(TARGET common PROPERTY POSITION_INDEPENDENT_CODE 1)

#Include api common files like heifreadercommondatatypes.h for HEIF::ErrorCode
target_include_directories(common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../api/common)

target_link_libraries(common PUBLIC vvcparser)
# vvc-parser is an internal codec helper; keep its include dir PRIVATE so it
# does not bleed onto the PUBLIC interface of in-tree consumers.
target_include_directories(common PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../vvc-parser)


