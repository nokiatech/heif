/* This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2026 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its
 * subsidiaries. All rights are reserved.
 *
 * Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior
 * written consent of Nokia.
 */

#include "readerapp.hpp"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <vector>

#include "buildinfo.hpp"
#include "customallocator.hpp"
#include "heiferrorinfo.h"
#include "log.hpp"
#include "heifreader.h"
#include "heifreaderdatatypes.h"
#include <cctype>
#include <exception>
#include <stdexcept>
#include <stdint.h>
#include <stdio.h>
#include <utility>

using namespace std;
using namespace HEIF;

// Per Phase G design: instead of draining the HEIF::ErrorInfo slot at every
// call site, we install a process-wide sink in main() that fires whenever
// any reader/writer/parser call writes to the slot. Diagnostics surface
// automatically as a single "[heif] ..." line on stderr without per-call-
// site boilerplate. Embedders can install their own sink (logger, GUI
// callback, crash reporter) by calling HEIF::setErrorSink(myFn, myUser);
// see srcs/api/common/heiferrorinfo.h.

template <typename T>
String to_String(T a)
{
    std::basic_ostringstream<char, std::char_traits<char>, Allocator<char>> st;
    st << a;
    return st.str();
}

template <typename T>
std::string to_string(T a)
{
    std::ostringstream st;
    st << a;
    return st.str();
}

int main(int argc, char* argv[])
{
    // Surface every native diagnostic on stderr automatically. Reference
    // sink ships with the library; embedders typically replace this with
    // their own logger.
    HEIF::setErrorSink(&HEIF::stderrErrorSink, stderr);

    ReaderApp app(argc, argv);
    try
    {
        app.processFile();
    }
    catch (ISOBMFF::Exception& exc)
    {
        Log::setLevel(Log::LogLevel::INFO); //Verbose log level to tell the user what caused the error
        logError() << "Error Exception: " << exc.what() << std::endl;
        app.printHelp();
        return EXIT_FAILURE;
    }
    catch (std::exception& e)
    {
        ///@todo Handle FilereaderException error codes here.
        Log::setLevel(Log::LogLevel::INFO); //Verbose log level to tell the user what caused the error
        logError() << "Error std:exception: " << e.what() << std::endl;
        app.printHelp();
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

ReaderApp::ReaderApp(int argc, char* argv[])
    : mMultipleOutputFiles(false)
    , mWriteMasterOnly(false)
    , mDumpMode(false)
    , mDumpOnlyMode(false)
    , mExecutablePath(argv[0])
{
    for (int i = 1; i < argc; ++i)
    {
        mParameters.emplace_back(argv[i]);
    }
}

void ReaderApp::processFile()
{
    const bool isCleanExit = handleArguments();
    const auto inputFile   = findFilename();

    if (inputFile.empty())
    {
        if (isCleanExit)
        {
            return;
        }
        throw std::runtime_error("Input configuration file name is required.");
    }

    logInfo() << "Reading file: " << inputFile << std::endl;
    Reader* reader = Reader::Create();
    if (reader->initialize(inputFile.c_str()) != ErrorCode::OK)
    {
        // Sink installed in main() already printed the verbose diagnostic.
        throw std::runtime_error("Failure while opening the file.");
    }
    logInfo() << "File read successfully." << std::endl;
    logInfo() << std::endl;

    if (mDumpMode || mDumpOnlyMode)
    {
        dumpFileInfo(reader);
        if (mDumpOnlyMode)
        {
            Reader::Destroy(reader);
            return;
        }
    }

    ImageId primaryItemId;
    if (ErrorCode::OK != reader->getPrimaryItem(primaryItemId))
    {
        logInfo() << "No primary item defined." << std::endl;
    }
    handleRootMeta(reader);
    handleTracks(reader);
    Reader::Destroy(reader);
}

static std::string fourccStr(const FourCC& fcc)
{
    return std::string(fcc.value, 4);
}

static std::string propertyTypeName(ItemPropertyType t)
{
    switch (t)
    {
    case ItemPropertyType::ISPE: return "ispe";
    case ItemPropertyType::COLR: return "colr";
    case ItemPropertyType::PIXI: return "pixi";
    case ItemPropertyType::IROT: return "irot";
    case ItemPropertyType::IMIR: return "imir";
    case ItemPropertyType::ISCL: return "iscl";
    case ItemPropertyType::CLAP: return "clap";
    case ItemPropertyType::PASP: return "pasp";
    case ItemPropertyType::HVCC: return "hvcC";
    case ItemPropertyType::AVCC: return "avcC";
    case ItemPropertyType::VVCC: return "vvcC";
    case ItemPropertyType::LHVC: return "lhvC";
    case ItemPropertyType::JPGC: return "jpgC";
    case ItemPropertyType::AUXC: return "auxC";
    case ItemPropertyType::RLOC: return "rloc";
    case ItemPropertyType::LSEL: return "lsel";
    case ItemPropertyType::TOLS: return "tols";
    case ItemPropertyType::OINF: return "oinf";
    case ItemPropertyType::SUBS: return "subs";
    case ItemPropertyType::CLLI: return "clli";
    case ItemPropertyType::MDCV: return "mdcv";
    case ItemPropertyType::CCLV: return "cclv";
    case ItemPropertyType::AMVE: return "amve";
    case ItemPropertyType::REVE: return "reve";
    case ItemPropertyType::NDWT: return "ndwt";
    case ItemPropertyType::ALTT: return "altt";
    case ItemPropertyType::CRTT: return "crtt";
    case ItemPropertyType::MDFT: return "mdft";
    case ItemPropertyType::RREF: return "rref";
    case ItemPropertyType::UDES: return "udes";
    case ItemPropertyType::RAW:  return "raw";
    default: return "unknown";
    }
}

namespace
{
std::string fileFeatureFlags(FeatureBitMask mask)
{
    struct Entry { FeatureBitMask bit; const char* name; };
    static const Entry table[] = {
        {FileFeatureEnum::HasSingleImage,      "HasSingleImage"},
        {FileFeatureEnum::HasImageCollection,  "HasImageCollection"},
        {FileFeatureEnum::HasImageSequence,    "HasImageSequence"},
        {FileFeatureEnum::HasRootLevelMetaBox, "HasRootLevelMetaBox"},
        {FileFeatureEnum::HasAlternateTracks,  "HasAlternateTracks"},
    };
    std::string out = "[";
    bool first = true;
    for (const auto& e : table)
    {
        if (mask & e.bit)
        {
            if (!first) out += ", ";
            out += "\"";
            out += e.name;
            out += "\"";
            first = false;
        }
    }
    out += "]";
    return out;
}

std::string itemFeatureFlags(FeatureBitMask mask)
{
    struct Entry { FeatureBitMask bit; const char* name; };
    static const Entry table[] = {
        {ItemFeatureEnum::IsMasterImage,                    "IsMasterImage"},
        {ItemFeatureEnum::IsThumbnailImage,                 "IsThumbnailImage"},
        {ItemFeatureEnum::IsAuxiliaryImage,                 "IsAuxiliaryImage"},
        {ItemFeatureEnum::IsPrimaryImage,                   "IsPrimaryImage"},
        {ItemFeatureEnum::IsDerivedImage,                   "IsDerivedImage"},
        {ItemFeatureEnum::IsPreComputedDerivedImage,        "IsPreComputedDerivedImage"},
        {ItemFeatureEnum::IsHiddenImage,                    "IsHiddenImage"},
        {ItemFeatureEnum::IsCoverImage,                     "IsCoverImage"},
        {ItemFeatureEnum::IsProtected,                      "IsProtected"},
        {ItemFeatureEnum::HasLinkedThumbnails,              "HasLinkedThumbnails"},
        {ItemFeatureEnum::HasLinkedAuxiliaryImage,          "HasLinkedAuxiliaryImage"},
        {ItemFeatureEnum::HasLinkedDerivedImage,            "HasLinkedDerivedImage"},
        {ItemFeatureEnum::HasLinkedPreComputedDerivedImage, "HasLinkedPreComputedDerivedImage"},
        {ItemFeatureEnum::HasLinkedTiles,                   "HasLinkedTiles"},
        {ItemFeatureEnum::HasLinkedMetadata,                "HasLinkedMetadata"},
        {ItemFeatureEnum::IsTileImageItem,                  "IsTileImageItem"},
        {ItemFeatureEnum::IsMetadataItem,                   "IsMetadataItem"},
        {ItemFeatureEnum::IsExifItem,                       "IsExifItem"},
        {ItemFeatureEnum::IsXMPItem,                        "IsXMPItem"},
        {ItemFeatureEnum::IsMPEG7Item,                      "IsMPEG7Item"},
        {ItemFeatureEnum::HasUnrecognzedRequiredReferences, "HasUnrecognizedRequiredReferences"},
        {ItemFeatureEnum::IsPredictivelyCodedImage,         "IsPredictivelyCodedImage"},
        {ItemFeatureEnum::IsGainmapImage,                   "IsGainmapImage"},
    };
    std::string out = "[";
    bool first = true;
    for (const auto& e : table)
    {
        if (mask & e.bit)
        {
            if (!first) out += ", ";
            out += "\"";
            out += e.name;
            out += "\"";
            first = false;
        }
    }
    out += "]";
    return out;
}

std::string trackFeatureFlags(FeatureBitMask mask)
{
    struct Entry { FeatureBitMask bit; const char* name; };
    static const Entry table[] = {
        {TrackFeatureEnum::IsMasterImageSequence,             "IsMasterImageSequence"},
        {TrackFeatureEnum::IsThumbnailImageSequence,          "IsThumbnailImageSequence"},
        {TrackFeatureEnum::IsAuxiliaryImageSequence,          "IsAuxiliaryImageSequence"},
        {TrackFeatureEnum::IsEnabled,                         "IsEnabled"},
        {TrackFeatureEnum::IsInMovie,                         "IsInMovie"},
        {TrackFeatureEnum::IsInPreview,                       "IsInPreview"},
        {TrackFeatureEnum::HasAlternatives,                   "HasAlternatives"},
        {TrackFeatureEnum::HasCodingConstraints,              "HasCodingConstraints"},
        {TrackFeatureEnum::HasSampleGroups,                   "HasSampleGroups"},
        {TrackFeatureEnum::HasLinkedAuxiliaryImageSequence,   "HasLinkedAuxiliaryImageSequence"},
        {TrackFeatureEnum::HasLinkedThumbnailImageSequence,   "HasLinkedThumbnailImageSequence"},
        {TrackFeatureEnum::HasSampleToItemGrouping,           "HasSampleToItemGrouping"},
        {TrackFeatureEnum::HasExifSampleEntry,                "HasExifSampleEntry"},
        {TrackFeatureEnum::HasXmlSampleEntry,                 "HasXmlSampleEntry"},
        {TrackFeatureEnum::HasEditList,                       "HasEditList"},
        {TrackFeatureEnum::HasInfiniteLoopPlayback,           "HasInfiniteLoopPlayback"},
        {TrackFeatureEnum::HasSampleEquivalenceGrouping,      "HasSampleEquivalenceGrouping"},
        {TrackFeatureEnum::IsAudioTrack,                      "IsAudioTrack"},
        {TrackFeatureEnum::IsVideoTrack,                      "IsVideoTrack"},
        {TrackFeatureEnum::DisplayAllSamples,                 "DisplayAllSamples"},
    };
    std::string out = "[";
    bool first = true;
    for (const auto& e : table)
    {
        if (mask & e.bit)
        {
            if (!first) out += ", ";
            out += "\"";
            out += e.name;
            out += "\"";
            first = false;
        }
    }
    out += "]";
    return out;
}

std::string metaBoxFeatureFlags(FeatureBitMask mask)
{
    struct Entry { FeatureBitMask bit; const char* name; };
    static const Entry table[] = {
        {MetaBoxFeatureEnum::IsSingleImage,               "IsSingleImage"},
        {MetaBoxFeatureEnum::IsImageCollection,           "IsImageCollection"},
        {MetaBoxFeatureEnum::HasMasterImages,             "HasMasterImages"},
        {MetaBoxFeatureEnum::HasThumbnails,               "HasThumbnails"},
        {MetaBoxFeatureEnum::HasAuxiliaryImages,          "HasAuxiliaryImages"},
        {MetaBoxFeatureEnum::HasDerivedImages,            "HasDerivedImages"},
        {MetaBoxFeatureEnum::HasPreComputedDerivedImages, "HasPreComputedDerivedImages"},
        {MetaBoxFeatureEnum::HasHiddenImages,             "HasHiddenImages"},
        {MetaBoxFeatureEnum::HasGroupLists,               "HasGroupLists"},
    };
    std::string out = "[";
    bool first = true;
    for (const auto& e : table)
    {
        if (mask & e.bit)
        {
            if (!first) out += ", ";
            out += "\"";
            out += e.name;
            out += "\"";
            first = false;
        }
    }
    out += "]";
    return out;
}
}  // anonymous namespace

void ReaderApp::dumpFileInfo(Reader* reader)
{
    using std::cout;
    using std::endl;

    FileInformation fileInfo;
    if (reader->getFileInformation(fileInfo) != ErrorCode::OK)
    {
        throw std::runtime_error("getFileInformation() failed");
    }

    cout << "{" << endl;

    // Library version
    cout << "  \"readerVersion\": \"" << Reader::GetVersion() << "\"," << endl;

    // File features
    cout << "  \"fileFeatures\": " << fileFeatureFlags(fileInfo.features) << "," << endl;

    // Primary item
    ImageId primaryId;
    if (reader->getPrimaryItem(primaryId) == ErrorCode::OK)
    {
        cout << "  \"primaryItemId\": " << primaryId.get() << "," << endl;
    }

    // Major brand
    FourCC majorBrand;
    if (reader->getMajorBrand(majorBrand) == ErrorCode::OK)
    {
        cout << "  \"majorBrand\": \"" << fourccStr(majorBrand) << "\"," << endl;
    }

    // Minor version
    uint32_t minorVersion = 0;
    if (reader->getMinorVersion(minorVersion) == ErrorCode::OK)
    {
        const char c[4] = {static_cast<char>((minorVersion >> 24) & 0xFF),
                           static_cast<char>((minorVersion >> 16) & 0xFF),
                           static_cast<char>((minorVersion >> 8) & 0xFF),
                           static_cast<char>(minorVersion & 0xFF)};
        const bool printable = std::isprint(static_cast<unsigned char>(c[0])) &&
                               std::isprint(static_cast<unsigned char>(c[1])) &&
                               std::isprint(static_cast<unsigned char>(c[2])) &&
                               std::isprint(static_cast<unsigned char>(c[3]));
        if (printable)
        {
            cout << "  \"minorVersion\": \"" << c[0] << c[1] << c[2] << c[3] << "\"," << endl;
        }
        else
        {
            cout << "  \"minorVersion\": " << minorVersion << "," << endl;
        }
    }

    // Compatible brands
    Array<FourCC> compatBrands;
    if (reader->getCompatibleBrands(compatBrands) == ErrorCode::OK && compatBrands.size > 0)
    {
        cout << "  \"compatibleBrands\": [";
        for (uint32_t b = 0; b < compatBrands.size; ++b)
        {
            cout << (b ? ", " : "") << "\"" << fourccStr(compatBrands[b]) << "\"";
        }
        cout << "]," << endl;
    }

    // Compatible brand combinations (typa boxes)
    Array<Array<FourCC>> combos;
    if (reader->getCompatibleBrandCombinations(combos) == ErrorCode::OK && combos.size > 0)
    {
        cout << "  \"compatibleBrandCombinations\": [" << endl;
        for (uint32_t c = 0; c < combos.size; ++c)
        {
            cout << "    [";
            for (uint32_t b = 0; b < combos[c].size; ++b)
            {
                cout << (b ? ", " : "") << "\"" << fourccStr(combos[c][b]) << "\"";
            }
            cout << "]" << (c + 1 < combos.size ? "," : "") << endl;
        }
        cout << "  ]," << endl;
    }

    // Metabox features
    cout << "  \"metaBoxFeatures\": " << metaBoxFeatureFlags(fileInfo.rootMetaBoxInformation.features) << "," << endl;

    // Movie-level matrix (only meaningful when tracks are present)
    if (fileInfo.trackInformation.size > 0)
    {
        Array<int32_t> matrix;
        if (reader->getMatrix(matrix) == ErrorCode::OK && matrix.size == 9)
        {
            cout << "  \"matrix\": [";
            for (uint32_t m = 0; m < matrix.size; ++m)
            {
                cout << (m ? ", " : "") << matrix[m];
            }
            cout << "]," << endl;
        }
    }

    // Items
    cout << "  \"items\": [" << endl;
    const auto& items = fileInfo.rootMetaBoxInformation.itemInformations;
    for (uint32_t i = 0; i < items.size; ++i)
    {
        const auto& item = items[i];
        cout << "    {" << endl;
        cout << "      \"id\": " << item.itemId.get() << "," << endl;
        cout << "      \"type\": \"" << fourccStr(item.type) << "\"," << endl;
        cout << "      \"size\": " << item.size << "," << endl;
        cout << "      \"features\": " << itemFeatureFlags(item.features) << "," << endl;

        // Item description (name / contentType) when present
        if (item.description.name.size > 1)
        {
            cout << "      \"name\": \"";
            for (uint32_t n = 0; n < item.description.name.size && item.description.name[n]; ++n)
                cout << item.description.name[n];
            cout << "\"," << endl;
        }
        if (item.description.contentType.size > 1)
        {
            cout << "      \"contentType\": \"";
            for (uint32_t n = 0; n < item.description.contentType.size && item.description.contentType[n]; ++n)
                cout << item.description.contentType[n];
            cout << "\"," << endl;
        }

        // Item references
        static const char* refTypes[] = {"thmb", "auxl", "cdsc", "dimg", "base", "pred"};
        bool hasAnyRef = false;
        for (const auto* rt : refTypes)
        {
            Array<ImageId> refs;
            if (reader->getReferencedFromItemListByType(item.itemId, FourCC(rt), refs) == ErrorCode::OK && refs.size > 0)
            {
                if (!hasAnyRef)
                {
                    cout << "      \"references\": {" << endl;
                    hasAnyRef = true;
                }
                cout << "        \"" << rt << "\": [";
                for (uint32_t r = 0; r < refs.size; ++r)
                    cout << (r ? ", " : "") << refs[r].get();
                cout << "]," << endl;
            }
        }
        if (hasAnyRef)
            cout << "      }," << endl;

        // Spatial extents (dimensions)
        Array<ItemPropertyInfo> props;
        if (reader->getItemProperties(item.itemId, props) == ErrorCode::OK)
        {
            cout << "      \"properties\": [" << endl;
            for (uint32_t p = 0; p < props.size; ++p)
            {
                const auto& prop = props[p];
                cout << "        { \"type\": \"" << propertyTypeName(prop.type)
                     << "\", \"id\": " << prop.index.get()
                     << ", \"essential\": " << (prop.essential ? "true" : "false");

                // Inline values for key property types
                if (prop.type == ItemPropertyType::ISPE)
                {
                    SpatialExtents ispe{};
                    if (reader->getProperty(prop.index, ispe) == ErrorCode::OK)
                    {
                        cout << ", \"width\": " << ispe.mImageWidth
                             << ", \"height\": " << ispe.mImageHeight;
                    }
                }
                else if (prop.type == ItemPropertyType::IROT)
                {
                    Rotate irot{};
                    if (reader->getProperty(prop.index, irot) == ErrorCode::OK)
                    {
                        cout << ", \"angle\": " << irot.angle;
                    }
                }
                else if (prop.type == ItemPropertyType::IMIR)
                {
                    Mirror imir{};
                    if (reader->getProperty(prop.index, imir) == ErrorCode::OK)
                    {
                        cout << ", \"horizontalAxis\": " << (imir.horizontalAxis ? "true" : "false");
                    }
                }
                else if (prop.type == ItemPropertyType::COLR)
                {
                    ColourInformation colr{};
                    if (reader->getProperty(prop.index, colr) == ErrorCode::OK)
                    {
                        cout << ", \"colourType\": \"" << fourccStr(colr.colourType) << "\"";
                        if (fourccStr(colr.colourType) == "nclx")
                        {
                            cout << ", \"colourPrimaries\": " << colr.colourPrimaries
                                 << ", \"transferCharacteristics\": " << colr.transferCharacteristics
                                 << ", \"matrixCoefficients\": " << colr.matrixCoefficients
                                 << ", \"fullRangeFlag\": " << (colr.fullRangeFlag ? "true" : "false");
                        }
                        else
                        {
                            cout << ", \"iccProfileSize\": " << colr.iccProfile.size;
                        }
                    }
                }
                else if (prop.type == ItemPropertyType::PIXI)
                {
                    PixelInformation pixi{};
                    if (reader->getProperty(prop.index, pixi) == ErrorCode::OK)
                    {
                        cout << ", \"bitsPerChannel\": [";
                        for (uint32_t c = 0; c < pixi.bitsPerChannel.size; ++c)
                        {
                            if (c > 0) cout << ", ";
                            cout << static_cast<int>(pixi.bitsPerChannel[c]);
                        }
                        cout << "]";
                    }
                }
                else if (prop.type == ItemPropertyType::CLAP)
                {
                    CleanAperture clap{};
                    if (reader->getProperty(prop.index, clap) == ErrorCode::OK)
                    {
                        cout << ", \"widthN\": " << clap.widthN
                             << ", \"widthD\": " << clap.widthD
                             << ", \"heightN\": " << clap.heightN
                             << ", \"heightD\": " << clap.heightD;
                    }
                }
                else if (prop.type == ItemPropertyType::CLLI)
                {
                    ContentLightLevel clli{};
                    if (reader->getProperty(prop.index, clli) == ErrorCode::OK)
                    {
                        cout << ", \"maxContentLightLevel\": " << clli.maxContentLightLevel
                             << ", \"maxPicAverageLightLevel\": " << clli.maxPicAverageLightLevel;
                    }
                }
                else if (prop.type == ItemPropertyType::LSEL)
                {
                    LayerSelector lsel{};
                    if (reader->getProperty(prop.index, lsel) == ErrorCode::OK)
                    {
                        cout << ", \"layerId\": " << lsel.layerId;
                    }
                }
                else if (prop.type == ItemPropertyType::TOLS)
                {
                    TargetOls tols{};
                    if (reader->getProperty(prop.index, tols) == ErrorCode::OK)
                    {
                        cout << ", \"targetOlsIndex\": " << tols.targetOlsIndex;
                    }
                }
                else if (prop.type == ItemPropertyType::PASP)
                {
                    PixelAspectRatio pasp{};
                    if (reader->getProperty(prop.index, pasp) == ErrorCode::OK)
                    {
                        cout << ", \"hSpacing\": " << pasp.relativeWidth
                             << ", \"vSpacing\": " << pasp.relativeHeight;
                    }
                }
                else if (prop.type == ItemPropertyType::AUXC)
                {
                    AuxiliaryType auxC{};
                    if (reader->getProperty(prop.index, auxC) == ErrorCode::OK)
                    {
                        cout << ", \"auxType\": \"";
                        for (uint32_t a = 0; a < auxC.auxType.size && auxC.auxType[a]; ++a)
                            cout << auxC.auxType[a];
                        cout << "\"";
                    }
                }
                else if (prop.type == ItemPropertyType::ISCL)
                {
                    Scale iscl{};
                    if (reader->getProperty(prop.index, iscl) == ErrorCode::OK)
                    {
                        cout << ", \"targetWidthN\": " << iscl.targetWidthN
                             << ", \"targetWidthD\": " << iscl.targetWidthD
                             << ", \"targetHeightN\": " << iscl.targetHeightN
                             << ", \"targetHeightD\": " << iscl.targetHeightD;
                    }
                }
                else if (prop.type == ItemPropertyType::RLOC)
                {
                    RelativeLocation rloc{};
                    if (reader->getProperty(prop.index, rloc) == ErrorCode::OK)
                    {
                        cout << ", \"horizontalOffset\": " << rloc.horizontalOffset
                             << ", \"verticalOffset\": " << rloc.verticalOffset;
                    }
                }
                else if (prop.type == ItemPropertyType::MDCV)
                {
                    MasteringDisplayColorVolume mdcv{};
                    if (reader->getProperty(prop.index, mdcv) == ErrorCode::OK)
                    {
                        cout << ", \"maxDisplayMasteringLuminance\": " << mdcv.maxDisplayMasteringLuminance
                             << ", \"minDisplayMasteringLuminance\": " << mdcv.minDisplayMasteringLuminance;
                    }
                }
                else if (prop.type == ItemPropertyType::CRTT)
                {
                    CreationTimeInformation crtt{};
                    if (reader->getProperty(prop.index, crtt) == ErrorCode::OK)
                    {
                        cout << ", \"time\": " << crtt.time;
                    }
                }
                else if (prop.type == ItemPropertyType::MDFT)
                {
                    ModificationTimeInformation mdft{};
                    if (reader->getProperty(prop.index, mdft) == ErrorCode::OK)
                    {
                        cout << ", \"time\": " << mdft.time;
                    }
                }

                cout << " }" << (p + 1 < props.size ? "," : "") << endl;
            }
            cout << "      ]" << endl;
        }

        cout << "    }" << (i + 1 < items.size ? "," : "") << endl;
    }
    cout << "  ]," << endl;

    // Entity groups
    cout << "  \"entityGroups\": [" << endl;
    const auto& groups = fileInfo.rootMetaBoxInformation.entityGroupings;
    for (uint32_t g = 0; g < groups.size; ++g)
    {
        const auto& group = groups[g];
        cout << "    { \"type\": \"" << fourccStr(group.type)
             << "\", \"groupId\": " << group.groupId.get()
             << ", \"entityIds\": [";
        for (uint32_t e = 0; e < group.entityIds.size; ++e)
        {
            if (e > 0) cout << ", ";
            cout << group.entityIds[e];
        }
        cout << "] }" << (g + 1 < groups.size ? "," : "") << endl;
    }
    cout << "  ]," << endl;

    // Tracks
    cout << "  \"tracks\": [" << endl;
    for (uint32_t t = 0; t < fileInfo.trackInformation.size; ++t)
    {
        const auto& track = fileInfo.trackInformation[t];
        const auto seqId = SequenceId(track.trackId.get());
        cout << "    {" << endl;
        cout << "      \"trackId\": " << track.trackId.get() << "," << endl;
        cout << "      \"features\": " << trackFeatureFlags(track.features) << "," << endl;
        cout << "      \"sampleCount\": " << track.sampleProperties.size << "," << endl;
        cout << "      \"timeScale\": " << track.timeScale << "," << endl;
        cout << "      \"maxSampleSize\": " << track.maxSampleSize << "," << endl;

        if (track.alternateGroupId != 0)
        {
            cout << "      \"alternateGroupId\": " << track.alternateGroupId << "," << endl;
            if (track.alternateTrackIds.size > 0)
            {
                cout << "      \"alternateTrackIds\": [";
                for (uint32_t a = 0; a < track.alternateTrackIds.size; ++a)
                    cout << (a ? ", " : "") << track.alternateTrackIds[a].get();
                cout << "]," << endl;
            }
        }

        if (track.referenceTrackIds.size > 0)
        {
            cout << "      \"trackReferences\": {";
            for (uint32_t r = 0; r < track.referenceTrackIds.size; ++r)
            {
                const auto& ref = track.referenceTrackIds[r];
                cout << (r ? ", " : " ") << "\"" << fourccStr(ref.type) << "\": [";
                for (uint32_t id = 0; id < ref.trackIds.size; ++id)
                    cout << (id ? ", " : "") << ref.trackIds[id].get();
                cout << "]";
            }
            cout << " }," << endl;
        }

        if (track.trackGroupIds.size > 0)
        {
            cout << "      \"trackGroups\": {";
            for (uint32_t g = 0; g < track.trackGroupIds.size; ++g)
            {
                const auto& grp = track.trackGroupIds[g];
                cout << (g ? ", " : " ") << "\"" << fourccStr(grp.type) << "\": [";
                for (uint32_t id = 0; id < grp.trackIds.size; ++id)
                    cout << (id ? ", " : "") << grp.trackIds[id].get();
                cout << "]";
            }
            cout << " }," << endl;
        }

        if (track.hasTypeInformation)
        {
            cout << "      \"trackType\": { \"majorBrand\": \"" << fourccStr(track.type.majorBrand)
                 << "\", \"minorVersion\": " << track.type.minorVersion;
            if (track.type.compatibleBrands.size > 0)
            {
                cout << ", \"compatibleBrands\": [";
                for (uint32_t b = 0; b < track.type.compatibleBrands.size; ++b)
                    cout << (b ? ", " : "") << "\"" << fourccStr(track.type.compatibleBrands[b]) << "\"";
                cout << "]";
            }
            cout << " }," << endl;
        }

        uint32_t displayWidth = 0, displayHeight = 0;
        if (reader->getDisplayWidth(seqId, displayWidth) == ErrorCode::OK &&
            reader->getDisplayHeight(seqId, displayHeight) == ErrorCode::OK)
        {
            cout << "      \"displayWidth\": " << displayWidth << "," << endl;
            cout << "      \"displayHeight\": " << displayHeight << "," << endl;
        }

        double duration = 0.0;
        if (reader->getPlaybackDurationInSecs(seqId, duration) == ErrorCode::OK)
        {
            cout << "      \"durationSecs\": " << duration << "," << endl;
        }

        if (track.frameRate.den != 0)
        {
            cout << "      \"frameRate\": " << static_cast<double>(track.frameRate.num) / track.frameRate.den << "," << endl;
        }
        if (track.sampleProperties.size > 0)
        {
            cout << "      \"sampleEntryType\": \"" << fourccStr(track.sampleProperties[0].sampleEntryType) << "\"" << endl;
        }
        cout << "    }" << (t + 1 < fileInfo.trackInformation.size ? "," : "") << endl;
    }
    cout << "  ]" << endl;

    cout << "}" << endl;
}

void ReaderApp::handleRootMeta(Reader* fileReader)
{
    FileInformation properties;
    if (fileReader->getFileInformation(properties) != ErrorCode::OK)
    {
        throw std::runtime_error("Calling reader API getFileInformation() failed.");
    }
    if ((properties.features & FileFeatureEnum::HasRootLevelMetaBox) == false)
    {
        return;
    }

    logInfo() << "Handling root meta" << endl;

    struct Type
    {
        FeatureBitMask feature;
        string fileBaseName;
    };

    const Vector<Type> outputs = {{HEIF::ItemFeatureEnum::IsMasterImage, "master"},
                                  {HEIF::ItemFeatureEnum::IsThumbnailImage, "thumbs"},
                                  {HEIF::ItemFeatureEnum::IsAuxiliaryImage, "auxs"},
                                  {HEIF::ItemFeatureEnum::IsGainmapImage, "gainmap"}};

    // Multi-layer VVC pre-pass: detect items that belong to one shared
    // multi-layer VVC stream (vvcC has multi_layer_flag=1, items live on
    // different nuh_layer_ids and share the same vvcC) and emit them as a
    // single combined "multilayer.h266" Annex-B file. The decoder is then
    // expected to be invoked on this single file with a layer-selection
    // flag. Items handled this way are excluded from the per-feature loop
    // below so we don't ALSO emit master.h266 + auxs.h266 for the same
    // bytes.
    Vector<HEIF::ImageId> allMetaImageIds;
    for (const auto& image : properties.rootMetaBoxInformation.itemInformations)
    {
        allMetaImageIds.push_back(image.itemId);
    }
    const std::set<HEIF::ImageId> multilayerHandled =
        writeMultiLayerVvcStreams(fileReader, "multilayer", allMetaImageIds);

    for (const auto& entry : outputs)
    {
        Vector<ImageId> imageIds;
        for (const auto& image : properties.rootMetaBoxInformation.itemInformations)
        {
            if ((image.features & entry.feature) && multilayerHandled.count(image.itemId) == 0u)
            {
                imageIds.push_back(image.itemId);
            }
        }
        writeCollectionBitstreams(fileReader, entry.fileBaseName, imageIds, mMultipleOutputFiles);

        if (entry.fileBaseName == "master" && mWriteMasterOnly)
        {
            return;
        }
    }

    if (properties.rootMetaBoxInformation.features & MetaBoxFeatureEnum::HasGroupLists)
    {
        logInfo() << "rootLevelMetaBoxProperties.metaBoxFeature has hasGroupLists set:" << endl;
        for (const auto& entry : properties.rootMetaBoxInformation.entityGroupings)
        {
            logInfo() << "Group name \"" << std::string(static_cast<const char*>(entry.type.value))
                      << "\" has array of entity IDs:" << endl;
            for (auto id : entry.entityIds)
            {
                logInfo() << id << " ";
            }
            logInfo() << std::endl;
        }
    }

    logInfo() << endl;
}

void ReaderApp::printSampleInformation(const HEIF::TrackInformation& trackInformation, HEIF::Reader* fileReader) const

{
    logInfo() << "Processing sample properties" << endl;
    for (const auto& sampleProperties : trackInformation.sampleProperties)
    {
        logInfo() << " sample ID=" << sampleProperties.sampleId.get() << endl;
        logInfo() << "  sample decode dependencies:" << endl;

        Array<SequenceImageId> ids;
        if (fileReader->getDecodeDependencies(trackInformation.trackId, sampleProperties.sampleId, ids) !=
            ErrorCode::OK)
        {
            throw std::runtime_error("Calling reader API getDecodeDependencies() failed.");
        }
        if (ids.size > 0)
        {
            logInfo() << "   dependencies: ";
            for (const auto& id : ids)
            {
                logInfo() << id.get() << " ";
            }
            logInfo() << endl;
        }
    }
}

void ReaderApp::handleTracks(Reader* fileReader)
{
    FileInformation properties;
    if (fileReader->getFileInformation(properties) != ErrorCode::OK)
    {
        throw std::runtime_error("Calling reader API getFileInformation() failed.");
    }
    for (const auto& trackProperties : properties.trackInformation)
    {
        const auto sequenceId = trackProperties.trackId;
        logInfo() << "Processing properties of track ID " << sequenceId.get() << endl;
        if (trackProperties.sampleProperties.size == 0)
        {
            logInfo() << "No sample properties set -- continuing to the next track if any." << endl;
            continue;
        }

        if (mWriteMasterOnly && !(trackProperties.features & TrackFeatureEnum::IsMasterImageSequence))
        {
            logInfo() << "Skipping non-master tracks as requested." << endl;
            continue;
        }

        printSampleInformation(trackProperties, fileReader);

        Array<TimestampIDPair> timestamps;
        if (fileReader->getItemTimestamps(sequenceId, timestamps) != ErrorCode::OK)
        {
            throw std::runtime_error("Calling reader API getItemTimestamps() failed.");
        }
        logInfo() << "Sample timestamps:" << endl;
        for (const auto& timestampIDPair : timestamps)
        {
            logInfo() << " Timestamp=" << timestampIDPair.timeStamp
                      << " sample decoding order=" << timestampIDPair.itemId.get() << std::endl;
        }

        FourCC codeType;
        Vector<SequenceImageId> imageIds;
        for (const auto& sampleProperties : trackProperties.sampleProperties)
        {
            imageIds.push_back(sampleProperties.sampleId);
            codeType = sampleProperties.sampleEntryType;
        }
        const string outfileName = "output_track_" + to_string(sequenceId.get());
        writeBitStreamFile(fileReader, outfileName, codeType, sequenceId, imageIds, false);
    }
}

void ReaderApp::writeCollectionBitstreams(Reader* fileReader,
                                          const string& fileName,
                                          const Vector<ImageId>& imageIds,
                                          bool createMultipleStreams)
{
    // Classify images by coding type.
    std::map<FourCC, Vector<ImageId>> items;
    for (const auto itemId : imageIds)
    {
        FourCC codeType;
        if (fileReader->getDecoderCodeType(itemId, codeType) == ErrorCode::OK)
        {
            items[codeType].push_back(itemId);
        }
    }

    // Write bitstreams for all found types.
    for (const auto& codecType : items)
    {
        writeBitStreamFile(fileReader, fileName, codecType.first, codecType.second, createMultipleStreams);
    }
}

// Annex-B start code prepended to every emitted NAL.
static const uint8_t kStartCode[4] = {0x00, 0x00, 0x00, 0x01};

// A canonical VVC AUD NAL for an IRAP access unit on layer 0:
//   byte0 = 0x00              forbidden=0, reserved=0, nuh_layer_id=0
//   byte1 = (20<<3) | 1 = 0xa1 nal_unit_type=AUD_NUT(20), nuh_temporal_id_plus1=1
//   byte2 = 0x88              aud_irap_or_gdr_flag=1, aud_pic_type=0 (I-slice),
//                              rbsp_trailing_bits = 1 + zero-padding
// VVC streams may legally start with this regardless of how many layers
// follow; multi-layer decoders use the AUD as an access-unit boundary
// marker only.
static const uint8_t kVvcIrapAud[3] = {0x00, 0xa1, 0x88};

// Skip Annex-B start code at the head of a NAL buffer and return the
// (nuh_layer_id, nal_unit_type) pair of the FIRST NAL inside `buf`.
// Returns false if a start code or two-byte header could not be found.
static bool sniffFirstNalHeader(const uint8_t* buf, std::size_t size,
                                uint8_t& layerOut, uint8_t& nutOut)
{
    std::size_t i = 0;
    while (i + 2 < size && buf[i] == 0x00)
    {
        ++i;
    }
    if (i >= size || buf[i] != 0x01 || i + 2 >= size)
    {
        return false;
    }
    ++i;  // step past 0x01
    layerOut = static_cast<uint8_t>(buf[i] & 0x3F);
    nutOut   = static_cast<uint8_t>((buf[i + 1] >> 3) & 0x1F);
    return true;
}

std::set<HEIF::ImageId> ReaderApp::writeMultiLayerVvcStreams(Reader* fileReader,
                                                             const std::string& baseFileName,
                                                             const Vector<ImageId>& imageIds)
{
    std::set<ImageId> handled;

    // VVC NAL unit types we care about for the bundle classification.
    enum : uint8_t { NUT_VPS = 14, NUT_SPS = 15, NUT_PPS = 16,
                     NUT_PREFIX_APS = 17, NUT_SUFFIX_APS = 18 };

    // ----- Pass 1: collect every multi-layer VVC item and remember its
    //               decoded layer-id and Annex-B item bitstream. -----
    struct LayerItem
    {
        uint8_t                  layerId;
        ImageId                  id;
        std::vector<uint8_t>     bytes;   // Annex-B with start codes
    };
    std::vector<LayerItem>          items;
    Array<DecoderSpecificInfo>      bundle;       // captured once, all-layers
    bool                            haveBundle = false;

    for (const auto itemId : imageIds)
    {
        FourCC codeType;
        if (fileReader->getDecoderCodeType(itemId, codeType) != ErrorCode::OK ||
            codeType != FourCC("vvc1"))
        {
            continue;
        }

        DecoderConfiguration single;
        DecoderConfiguration all;
        if (fileReader->getDecoderParameterSets(itemId, single) != ErrorCode::OK ||
            fileReader->getDecoderParameterSetsForAllLayers(itemId, all) != ErrorCode::OK)
        {
            continue;
        }
        // Same-size bundles mean the item is single-layer (no extra per-layer
        // SPS/PPS to merge); leave it for the regular per-feature pass so we
        // don't double-emit.
        if (all.decoderSpecificInfo.size <= single.decoderSpecificInfo.size)
        {
            continue;
        }

        uint64_t dataSize = 1024 * 1024 * 10;
        std::vector<uint8_t> buffer(dataSize);
        if (fileReader->getItemData(itemId, buffer.data(), dataSize, /*bytestreamHeaders=*/true) !=
            ErrorCode::OK)
        {
            throw std::runtime_error("Calling reader API getItemData() failed for multilayer item.");
        }
        buffer.resize(static_cast<std::size_t>(dataSize));

        uint8_t layerId = 0;
        uint8_t nut     = 0;
        if (!sniffFirstNalHeader(buffer.data(), buffer.size(), layerId, nut))
        {
            logInfo() << "Multilayer item " << itemId.get()
                      << ": failed to sniff NAL header, defaulting to layer 0." << std::endl;
        }

        items.push_back(LayerItem{layerId, itemId, std::move(buffer)});
        if (!haveBundle)
        {
            bundle     = all.decoderSpecificInfo;
            haveBundle = true;
        }
    }

    if (items.empty())
    {
        return handled;
    }

    // ----- Pass 2: classify the all-layers parameter-set bundle by NAL
    //               unit type and by nuh_layer_id, so each NAL is placed
    //               with the picture it actually belongs to. APS is
    //               placed by its OWN nuh_layer_id (per ITU-T H.266
    //               Section 7.4.2.4.3: every non-VCL NAL of a picture must
    //               precede that picture's first VCL NAL unit).
    using NalRef = const Array<uint8_t>*;
    NalRef                                vps = nullptr;            // CVS-global
    std::map<uint8_t, std::vector<NalRef>> spsByLayer, ppsByLayer, apsByLayer;

    for (const auto& info : bundle)
    {
        const auto& nalArr = info.decSpecInfoData;
        // Each bundle entry is one Annex-B-framed NAL: skip start code, then
        // read the 2-byte NAL header to learn (layer, type).
        uint8_t layer = 0;
        uint8_t nut   = 0;
        if (!sniffFirstNalHeader(&nalArr[0], nalArr.size, layer, nut))
        {
            continue;
        }
        switch (nut)
        {
            case NUT_VPS:        vps = &nalArr; break;
            case NUT_SPS:        spsByLayer[layer].push_back(&nalArr); break;
            case NUT_PPS:        ppsByLayer[layer].push_back(&nalArr); break;
            case NUT_PREFIX_APS:
            case NUT_SUFFIX_APS: apsByLayer[layer].push_back(&nalArr); break;
            default:             /* ignored */ break;
        }
    }

    // Sort items ascending by layer id so layer 0 comes first (lower-layer
    // pictures always precede higher-layer pictures within an AU).
    std::sort(items.begin(), items.end(),
              [](const LayerItem& a, const LayerItem& b) { return a.layerId < b.layerId; });

    // ----- Pass 3: write the file as a single multi-layer access unit:
    //
    //   AUD                                          (AU delimiter, layer 0)
    //   VPS                                          (CVS-global)
    //   for each layer L in ascending order:
    //       SPS-L, PPS-L, APS-L, [other non-VCL-L]   (all non-VCL of layer L)
    //       VCL-L                                    (the IDR access unit)
    //
    // This satisfies VVC's two ordering rules that VTM enforces:
    //   1) All non-VCL NALs of a picture precede that picture's VCL NALs.
    //   2) All NALs of a lower-layer picture in an AU precede all NALs of
    //      any higher-layer picture in the same AU.
    //
    // The original vvcC's storage layout (AUD, VPS, SPS-L0, PPS-L0, APS,
    // SPS-L1, PPS-L1) is a parameter-set BOX layout, NOT an AU emission
    // layout -- APS in that box is on nuh_layer_id=0 and so belongs with
    // layer-0's non-VCL group, before layer-0's VCL.
    const std::string outputPath = baseFileName + ".h266";
    logInfo() << "Writing multi-layer H.266/VVC bitstream file '" << outputPath << "'" << std::endl;
    std::ofstream outfile(outputPath, std::ofstream::binary);
    if (!outfile)
    {
        throw std::runtime_error("Failed to open multilayer output file '" + outputPath + "'");
    }

    auto writeAnnexB = [&outfile](const uint8_t* nalBody, std::size_t nalBodyLen) {
        outfile.write(reinterpret_cast<const char*>(kStartCode), sizeof(kStartCode));
        outfile.write(reinterpret_cast<const char*>(nalBody),
                      static_cast<std::streamsize>(nalBodyLen));
    };

    auto writePrebuiltAnnexB = [&outfile](NalRef nal) {
        // Bundle entries already contain a leading Annex-B start code, so
        // they go out as-is.
        outfile.write(reinterpret_cast<const char*>(&(*nal)[0]),
                      static_cast<std::streamsize>(nal->size));
    };

    // 1) AUD -- synthesized, layer 0, IRAP/I-slice.
    writeAnnexB(kVvcIrapAud, sizeof(kVvcIrapAud));
    logInfo() << " wrote AUD (synthesized, 3 B body)" << std::endl;

    // 2) VPS -- single CVS-global NAL; spec requires VPS before any SPS.
    if (vps != nullptr)
    {
        writePrebuiltAnnexB(vps);
        logInfo() << " wrote VPS (" << vps->size << " B incl. start code)" << std::endl;
    }

    // 3..N) For each layer in ascending order: full non-VCL group then VCL.
    for (const auto& it : items)
    {
        const uint8_t layer = it.layerId;

        const auto spsIt = spsByLayer.find(layer);
        if (spsIt != spsByLayer.end())
        {
            for (NalRef n : spsIt->second) { writePrebuiltAnnexB(n); }
            logInfo() << " wrote SPS for layer " << static_cast<int>(layer)
                      << " (" << spsIt->second.size() << " NAL"
                      << (spsIt->second.size() == 1 ? "" : "s") << ")" << std::endl;
        }
        const auto ppsIt = ppsByLayer.find(layer);
        if (ppsIt != ppsByLayer.end())
        {
            for (NalRef n : ppsIt->second) { writePrebuiltAnnexB(n); }
            logInfo() << " wrote PPS for layer " << static_cast<int>(layer)
                      << " (" << ppsIt->second.size() << " NAL"
                      << (ppsIt->second.size() == 1 ? "" : "s") << ")" << std::endl;
        }
        const auto apsIt = apsByLayer.find(layer);
        if (apsIt != apsByLayer.end())
        {
            for (NalRef n : apsIt->second) { writePrebuiltAnnexB(n); }
            logInfo() << " wrote APS for layer " << static_cast<int>(layer)
                      << " (" << apsIt->second.size() << " NAL"
                      << (apsIt->second.size() == 1 ? "" : "s") << ")" << std::endl;
        }

        // Item bytes already carry their own Annex-B start code(s).
        outfile.write(reinterpret_cast<const char*>(it.bytes.data()),
                      static_cast<std::streamsize>(it.bytes.size()));
        handled.insert(it.id);
        logInfo() << " wrote IDR for layer " << static_cast<int>(layer)
                  << " (item " << it.id.get() << ", " << it.bytes.size() << " B)" << std::endl;
    }

    return handled;
}

bool compareSets(const Array<DecoderSpecificInfo>& a, const Array<DecoderSpecificInfo>& b)
{
    if (a.size != b.size)
    {
        return false;
    }
    for (uint32_t i = 0; i < a.size; ++i)
    {
        if (a[i].decSpecInfoType != b[i].decSpecInfoType)
        {
            return false;
        }
        if (a[i].decSpecInfoData.size != b[i].decSpecInfoData.size)
        {
            return false;
        }
        if (std::memcmp(&a[i].decSpecInfoData[0], &b[i].decSpecInfoData[0], a[i].decSpecInfoData.size) != 0)
        {
            return false;
        }
    }
    return true;
}

void ReaderApp::writeBitStreamFile(Reader* fileReader,
                                   const string& fileName,
                                   const FourCC& coding,
                                   const Vector<ImageId>& imageIds,
                                   bool createMultipleStreams)
{
    struct Type
    {
        std::string mediaName;
        std::string fileExtension;
    };

    const std::map<FourCC, Type> CODING_TYPES = {{"avc1", {"H.264/AVC", ".h264"}},
                                                 {"hvc1", {"H.265/HEVC", ".h265"}},
                                                 {"vvc1", {"H.266/VVC", ".h266"}},
                                                 {"mp4a", {"AAC-LC", ".aac"}},
                                                 {"raw1", {"RAW", ".raw"}},};

    if (imageIds.size() == 0)
    {
        return;
    }

    if (CODING_TYPES.count(coding) == 0)
    {
        logInfo() << "Not creating bitstream file '" + to_string(fileName) + "' for unsupported code type '" +
                         std::string(coding.value) + "'"
                  << std::endl;
        return;
    }

    const auto mediaName     = CODING_TYPES.at(coding).mediaName;
    const auto fileExtension = CODING_TYPES.at(coding).fileExtension;

    std::ofstream outfile;
    if (!createMultipleStreams)
    {
        logInfo() << "Writing " << mediaName << " bitstream file '" << fileName << fileExtension << "'" << endl;
        outfile.open(fileName + fileExtension, std::ofstream::binary);
    }

    bool decoderParametersWritten = false;
    Array<DecoderSpecificInfo> previousParameterSet;
    unsigned int imageCount         = 0;
    unsigned int configurationCount = 0;
    bool printWroteImage            = false;
    bool skipFirstItemByte          = false;
    for (const auto imageId : imageIds)
    {
        if (createMultipleStreams)
        {
            std::ostringstream ss;
            ss << fileName << std::setw(3) << std::setfill('0') << imageCount << fileExtension;
            outfile.close();
            logInfo() << "Writing " << mediaName << " bitstream file '" << ss.str() << "'" << endl;
            outfile.open(ss.str(), std::ofstream::binary);
            decoderParametersWritten = false;
        }

        uint64_t dataSize      = 1024 * 1024 * 10;
        auto* imageData        = new uint8_t[dataSize];
        bool bytestreamHeaders = true;
        if (coding == FourCC("mp4a"))
        {
            bytestreamHeaders = false;
        }
        if (fileReader->getItemData(imageId, imageData, dataSize, bytestreamHeaders) != ErrorCode::OK)
        {
            throw std::runtime_error("Calling reader API getItemData() failed.");
        }
        // The first item coming after decoder parameter sets needs one zero being
        // skipped.

        DecoderConfiguration parameterSet;
        if (fileReader->getDecoderParameterSets(imageId, parameterSet) != ErrorCode::OK)
        {
            throw std::runtime_error("Calling reader API getDecoderParameterSets() failed.");
        }

        if ((decoderParametersWritten == false) || !compareSets(parameterSet.decoderSpecificInfo, previousParameterSet))
        {
            ///@todo this should write parameter sets associated per item, identified by nuh_layer_id
            ///@bug some logic error writes wrong sets. Find out why and what parametersets are needed for per item
            /// Like; VPS is associated for both layers, but what should be written out?
            ///But if the VPS, that contains the PTL, is written for both separate items, is the decoder able to handle it 
            ///(having references to multiple layers)?
            previousParameterSet = parameterSet.decoderSpecificInfo;
            for (auto& params : parameterSet.decoderSpecificInfo)
            {
                outfile.write(reinterpret_cast<char*>(&params.decSpecInfoData[0]),
                              static_cast<streamsize>(params.decSpecInfoData.size));
            }

            // If decoder settings are written, skip the first '0' of image data.
            skipFirstItemByte = true;
            logInfo() << " wrote configuration #" << ++configurationCount << endl;
            decoderParametersWritten = true;
            printWroteImage          = false;
        }
        if (skipFirstItemByte && coding != FourCC("raw1"))
        {
            outfile.write(reinterpret_cast<char*>(imageData + 1), static_cast<int32_t>(dataSize) - 1);
            skipFirstItemByte = false;
        }
        else
        {
            outfile.write(reinterpret_cast<char*>(imageData), static_cast<int32_t>(dataSize));
        }

        delete[] imageData;

        if (printWroteImage == false)
        {
            logInfo() << " wrote image";
            printWroteImage = true;
        }
        logInfo() << " #" << ++imageCount << "(ID " << imageId.get() << ")" << std::endl;
    }
    logInfo() << endl;
}

void ReaderApp::writeBitStreamFile(Reader* fileReader,
                                   const string& fileName,
                                   const FourCC& coding,
                                   const SequenceId sequenceId,
                                   const Vector<SequenceImageId>& imageIds,
                                   bool createMultipleStreams)
{
    struct Type
    {
        std::string mediaName;
        std::string fileExtension;
    };

    const std::map<FourCC, Type> CODING_TYPES = {{"avc1", {"H.264/AVC", ".h264"}},
                                                 {"hvc1", {"H.265/HEVC", ".h265"}},
                                                 {"vvc1", {"H.266/VVC", ".h266"}},
                                                 {"mp4a", {"AAC-LC", ".aac"}},
                                                 {"avif", {"AV1", ".av1"}}};


    if (imageIds.size() == 0)
    {
        return;
    }

    if (CODING_TYPES.count(coding) == 0)
    {
        logInfo() << "Not creating bitstream file '" + to_string(fileName) + "' for unsupported code type '" +
                         std::string(coding.value) + "'"
                  << std::endl;
        return;
    }

    const auto mediaName     = CODING_TYPES.at(coding).mediaName;
    const auto fileExtension = CODING_TYPES.at(coding).fileExtension;

    std::ofstream outfile;
    if (!createMultipleStreams)
    {
        logInfo() << "Writing " << mediaName << " bitstream file '" << fileName << fileExtension << "'" << endl;
        outfile.open(fileName + fileExtension, std::ofstream::binary);
    }

    bool decoderParametersWritten = false;
    Array<DecoderSpecificInfo> previousParameterSet;
    unsigned int imageCount         = 0;
    unsigned int configurationCount = 0;
    bool printWroteImage            = false;
    bool skipFirstItemByte          = false;
    for (const auto imageId : imageIds)
    {
        if (createMultipleStreams)
        {
            std::ostringstream ss;
            ss << fileName << std::setw(3) << std::setfill('0') << imageCount << fileExtension;
            outfile.close();
            logInfo() << "Writing " << mediaName << " bitstream file '" << ss.str() << "'" << endl;
            outfile.open(ss.str(), std::ofstream::binary);
            decoderParametersWritten = false;
        }

        uint64_t dataSize      = 1024 * 1024 * 10;
        auto* imageData        = new uint8_t[dataSize];
        bool bytestreamHeaders = true;
        if (coding == FourCC("mp4a"))
        {
            bytestreamHeaders = false;
        }
        if (fileReader->getItemData(sequenceId, imageId, imageData, dataSize, bytestreamHeaders) != ErrorCode::OK)
        {
            throw std::runtime_error("Calling reader API getItemData() failed.");
        }
        // The first item coming after decoder parameter sets needs one zero being
        // skipped.

        DecoderConfiguration parameterSet;
        if (fileReader->getDecoderParameterSets(sequenceId, imageId, parameterSet) != ErrorCode::OK)
        {
            throw std::runtime_error("Calling reader API getDecoderParameterSets() failed.");
        }

        if ((decoderParametersWritten == false) || !compareSets(parameterSet.decoderSpecificInfo, previousParameterSet))
        {
            previousParameterSet = parameterSet.decoderSpecificInfo;
            for (auto& params : parameterSet.decoderSpecificInfo)
            {
                outfile.write(reinterpret_cast<char*>(&params.decSpecInfoData[0]),
                              static_cast<streamsize>(params.decSpecInfoData.size));
            }

            // If decoder settings are written, skip the first '0' of image data.
            skipFirstItemByte = true;
            logInfo() << " wrote configuration #" << ++configurationCount << endl;
            decoderParametersWritten = true;
            printWroteImage          = false;
        }
        if (skipFirstItemByte)
        {
            outfile.write(reinterpret_cast<char*>(imageData + 1), static_cast<int32_t>(dataSize) - 1);
            skipFirstItemByte = false;
        }
        else
        {
            outfile.write(reinterpret_cast<char*>(imageData), static_cast<int32_t>(dataSize));
        }
        if (printWroteImage == false)
        {
            logInfo() << " wrote image";
            printWroteImage = true;
        }
        logInfo() << " #" << ++imageCount << "(ID " << imageId.get() << ")" << std::endl;
    }
    logInfo() << endl;
}


void ReaderApp::printHelp()
{
    Log::setLevel(Log::LogLevel::INFO);
    logInfo() << "Usage: " << mExecutablePath << " [options] input-heic-file" << endl
              << "Supported options:" << endl
              << "--dump           Print file structure as JSON, then extract normally" << std::endl
              << "--dump-only      Print file structure as JSON to stdout and exit" << std::endl
              << "--help           Print this help" << std::endl
              << "--master-only    Dump only master bitstreams" << std::endl
              << "--multiple, -m   Output multiple bitstream files" << std::endl
              << "--no-warnings    Print errors only" << endl
              << "--verbose, -v    Print debug information" << endl
              << "--version        Print version number" << endl;
}

bool ReaderApp::handleArguments()
{
    bool exitSuccess = false;
    for (auto param : mParameters)
    {
        if (param == "-v" || param == "--verbose")
        {
            Log::setLevel(Log::LogLevel::INFO);
        }
        else if (param == "--help")
        {
            printHelp();
            exitSuccess = true;
        }
        else if (param == "--version")
        {
            std::cout << "HEIF reader build version " << Reader::GetVersion() << " date " << BuildInfo::Time
                      << std::endl;
            exitSuccess = true;
        }
        else if (param == "--no-warnings")
        {
            Log::setLevel(Log::LogLevel::ERROR);
        }
        else if (param == "-m" || param == "--multiple")
        {
            mMultipleOutputFiles = true;
        }
        else if (param == "--master-only")
        {
            mWriteMasterOnly = true;
        }
        else if (param == "--dump")
        {
            mDumpMode = true;
            Log::setLevel(Log::LogLevel::INFO);
        }
        else if (param == "--dump-only")
        {
            mDumpOnlyMode = true;
        }
        else if (param.at(0) == '-')
        {
            throw std::runtime_error("Unknown parameter '" + param + "'");
        }
    }

    return exitSuccess;
}

string ReaderApp::findFilename() const
{
    string filename;
    // Treat an argument without "-" prefix as the input configuration file name
    for (const auto& parameter : mParameters)
    {
        if (parameter.at(0) != '-' && filename.empty())
        {
            filename = parameter;
        }
        else if (parameter.at(0) != '-' && !filename.empty())
        {
            throw std::runtime_error("Multiple input files?");
        }
    }

    return filename;
}
