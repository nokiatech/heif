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

// Worked examples for the raw HEIF::Reader / HEIF::Writer C++ API
// (target heif::heif_static + heif::heif_writer_static).
//
// Examples 1-8  : HEVC HEIF read/write.
//                 Example 1 also demonstrates both byte-fetch styles
//                 side by side: the HEIF::readItem* one-liner helper
//                 and the raw two-call (size-query + fetch) pattern
//                 for callers that want to own the buffer themselves
//                 (uint8_t*, char[], JNI direct buffer, wasm heap,
//                 arena, ...).
// Example  9    : write VVC HEIF in MiniBox (compact) format.
// Example  10   : read a VVC MiniBox and expand it to a standard HEIF.
// Example  11   : write a VVC HEIF with a primary image plus alpha layer.
// Example  12   : error-handling protocol -- ErrorCode return + the
//                 thread-local HEIF::ErrorInfo side-channel introduced
//                 with structured info (origin / file:line / cause).
// Example  13   : mux a plain MP4 video ('vide') and AAC-LC audio
//                 ('soun') track into the same file so it also plays
//                 in a regular MP4 player.
// Example  14   : segment-streaming (DASH) playback -- parse an init
//                 segment, feed media segments, read samples, then
//                 invalidate segments on seek.
// Example  15   : raw (custom) property round-trip -- write a HEIF with
//                 a hypothetical future-spec property passed as a raw
//                 bitstream blob, then read it back and verify the
//                 bytes survived.
//
// Notes:
//   * Test files come from https://github.com/nokiatech/heif_conformance --
//     drop them next to the binary or examples that need them quietly skip.
//   * Examples 1-11 keep error checking minimal so the API call shape is
//     easy to follow. Example 12 shows the pattern production code should
//     actually use.
//   * Decoding / rendering of the resulting bitstreams is out of scope --
//     hand the bytes to your decoder of choice.
//
// See also:
//   * srcs/examples/heifpp_example.cpp        -- HEIFPP wrapper (recommended for new C++ apps)

// Standard library
#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <vector>

// HEIF reader + writer + the error-info side-channel
#include "heiferrorinfo.h"
#include "heifreader.h"
#include "heifstreaminterface.h"
#include "heifwriter.h"
#include "heifcommondatatypes.h"
#include "heifreaderdatatypes.h"
#include "heifwriterdatatypes.h"
#include <cstdio>
#include <exception>
#include <stdexcept>
#include <stdint.h>

using namespace std;
using namespace HEIF;

void example1();
void example2();
void example3();
void example4();
void example5();
void example6();
void example7();
void example8();
void example9();
void example10();
void example11();
void example12();
void example13();
void example14();
void example15();
void printInfo(const char* file);


/// Example 1 -- grab the cover image (= primary item) of a HEIF file.
void example1()
{
    // Reader is a heap-allocated handle; pair Create() with Destroy().
    auto* reader = Reader::Create();

    const char* filename = "C003.heic";
    if (reader->initialize(filename) != ErrorCode::OK)
    {
        printInfo(filename);
        Reader::Destroy(reader);
        return;
    }

    // FileInformation is the parsed top-level container summary.
    FileInformation info;
    reader->getFileInformation(info);

    // The "primary item" is the image a viewer should show first.
    ImageId itemId;
    reader->getPrimaryItem(itemId);

    // Pull the encoded bitstream + decoder parameter sets in one call --
    // ready to hand straight to your HEVC decoder.
    auto bytes = readItemDataWithDecoderParameters(reader, itemId);

    // ... feed bytes.elements / bytes.size to your decoder and display ...
    (void) bytes;

    // Alternative: the raw two-call pattern.
    //
    // Use this when you want to own the buffer (uint8_t*, char[],
    // a JNI direct buffer, a wasm heap slab, an arena, ...) instead
    // of accepting the library's Array<uint8_t>. Mechanics:
    //
    //   1. SIZE QUERY: pass nullptr + size=0. The library writes the
    //      required size into `needed` and returns BUFFER_SIZE_TOO_SMALL.
    //   2. ALLOCATE  : whatever shape your code wants.
    //   3. FETCH     : pass the buffer + the size from step 1. On OK,
    //      `needed` is narrowed to bytes actually written.
    //
    // The helper above does exactly these three steps; this is what's
    // happening under the hood.
    uint64_t needed = 0;
    reader->getItemDataWithDecoderParameters(itemId, nullptr, needed);  // size query
    auto* rawBuf = new uint8_t[needed];                                 // your allocator
    reader->getItemDataWithDecoderParameters(itemId, rawBuf, needed);   // fetch
    // ... feed rawBuf / needed to your decoder ...
    delete[] rawBuf;

    Reader::Destroy(reader);
}


/// Example 2 -- show a thumbnail of the first master image, fall back
/// to the master image itself if no thumbnail is linked.
void example2()
{
    auto* reader = Reader::Create();

    const char* filename = "C012.heic";
    if (reader->initialize(filename) != ErrorCode::OK)
    {
        printInfo(filename);
        Reader::Destroy(reader);
        return;
    }

    FileInformation info;
    if (reader->getFileInformation(info) != ErrorCode::OK)
    {
        return;
    }

    // FileFeature flags say at a glance what kinds of things the file holds.
    if (!(info.features & FileFeatureEnum::HasSingleImage || info.features & FileFeatureEnum::HasImageCollection))
    {
        return;
    }

    // Master images = the top-level still images. Auxiliary, thumbnail
    // and tile items hang off these but aren't returned here.
    Array<ImageId> itemIds;
    reader->getMasterImages(itemIds);
    if (itemIds.size == 0) return;

    const ImageId masterId = itemIds[0];

    const auto metaBoxFeatures = info.rootMetaBoxInformation.features;
    if (metaBoxFeatures & MetaBoxFeatureEnum::HasThumbnails)
    {
        // 'thmb' references go FROM the thumbnail TO its master image.
        // getReferencedToItemListByType walks them in that direction.
        reader->getReferencedToItemListByType(masterId, "thmb", itemIds);
        if (itemIds.size == 0) return;
        const auto thumbnailId = itemIds[0];

        auto bytes = readItemDataWithDecoderParameters(reader, thumbnailId.get());
        // ... decode and display the thumbnail, then load the master ...
        (void) bytes;
    }
    else
    {
        // No thumbnail -- show the master directly.
        auto bytes = readItemDataWithDecoderParameters(reader, masterId.get());
        // ... decode and display ...
        (void) bytes;
    }

    Reader::Destroy(reader);
}


/// Example 3 -- build a master->thumbnail map for a whole image collection,
/// picking the narrowest thumbnail when several are linked to one master.
void example3()
{
    auto* reader = Reader::Create();
    typedef ImageId MasterItemId;
    typedef ImageId ThumbnailItemId;
    Array<ImageId> itemIds;
    map<MasterItemId, ThumbnailItemId> imageMap;

    const char* filename = "C012.heic";
    if (reader->initialize(filename) != ErrorCode::OK)
    {
        printInfo(filename);
        Reader::Destroy(reader);
        return;
    }

    FileInformation info;
    reader->getFileInformation(info);

    reader->getMasterImages(itemIds);

    // For every master image, find linked thumbnails and pick the smallest.
    for (const auto masterId : itemIds)
    {
        Array<ImageId> thumbIds;
        reader->getReferencedToItemListByType(masterId, "thmb", thumbIds);

        const auto thumbId = std::min_element(thumbIds.begin(), thumbIds.end(),
                                              [&](ImageId a, ImageId b)
                                              {
                                                  uint32_t widthA, widthB;
                                                  reader->getWidth(a, widthA);
                                                  reader->getWidth(b, widthB);
                                                  return (widthA < widthB);
                                              });
        if (thumbId != thumbIds.end())
        {
            imageMap[masterId] = thumbId->get();
        }
    }

    // imageMap is now ready to drive a thumbnail grid in your UI.
    Reader::Destroy(reader);
}


/// Example 4 -- read a derived ('iden') image and figure out how to
/// render it from its source image plus any transform properties.
void example4()
{
    auto* reader = Reader::Create();
    Array<ImageId> itemIds;

    const char* filename = "C008.heic";
    if (reader->initialize(filename) != ErrorCode::OK)
    {
        printInfo(filename);
        Reader::Destroy(reader);
        return;
    }

    FileInformation info;
    reader->getFileInformation(info);

    // 'iden' = identity-derived image: it points at a source image and
    // applies item properties (rotate, mirror, crop, ...) on top.
    reader->getItemListByType("iden", itemIds);
    if (itemIds.size == 0) return;
    const auto itemId = itemIds[0];

    // 'dimg' references go FROM the derived item TO its input image(s).
    Array<ImageId> referencedImages;
    reader->getReferencedFromItemListByType(itemId, "dimg", referencedImages);
    if (referencedImages.size == 0) return;
    const ImageId sourceItemId = referencedImages[0];

    // Item properties tell us what transform to apply when rendering.
    Array<ItemPropertyInfo> propertyInfos;
    reader->getItemProperties(itemId, propertyInfos);

    unsigned int rotation = 0;
    for (const auto& property : propertyInfos)
    {
        // 'irot' = anti-clockwise rotation in 90-degree steps.
        if (property.type == ItemPropertyType::IROT)
        {
            Rotate irot;
            reader->getProperty(property.index, irot);
            rotation = irot.angle;
            break;
        }
    }

    cout << "  derived item " << itemId.get()
         << " -> render source " << sourceItemId.get()
         << " rotated " << rotation << " deg\n";

    Reader::Destroy(reader);
}


/// Example 5 -- walk image-sequence tracks (the HEIF equivalent of a
/// short video clip) and pull each sample with its decode dependencies.
void example5()
{
    auto* reader = Reader::Create();
    Array<uint32_t> itemIds;

    const char* filename = "C032.heic";
    if (reader->initialize(filename) != ErrorCode::OK)
    {
        printInfo(filename);
        Reader::Destroy(reader);
        return;
    }
    FileInformation info;
    reader->getFileInformation(info);

    for (const auto& trackProperties : info.trackInformation)
    {
        const auto sequenceId = trackProperties.trackId;

        // Track features tell us what kind of track this is.
        if (trackProperties.features & TrackFeatureEnum::IsThumbnailImageSequence
            && trackProperties.referenceTrackIds.size > 0)
        {
            // Thumbnail tracks reference one or more master tracks via 'tref'.
            const auto tref = trackProperties.referenceTrackIds[0];
            (void) tref;  // pair the thumb track with its master(s) here
        }

        // Per-sample timestamps drive playback.
        Array<TimestampIDPair> timestamps;
        reader->getItemTimestamps(sequenceId, timestamps);

        for (const auto& sampleProperties : trackProperties.sampleProperties)
        {
            // P/B frames depend on earlier samples; ask the reader which
            // ones we need to decode this sample correctly.
            Array<SequenceImageId> itemsToDecode;
            reader->getDecodeDependencies(sequenceId, sampleProperties.sampleId, itemsToDecode);
            for (auto dependencyId : itemsToDecode)
            {
                auto sample = readItemDataWithDecoderParameters(reader, sequenceId, dependencyId);

                // ... feed sample.elements / sample.size to your video decoder ...
                (void) sample;
            }
        }
    }

    Reader::Destroy(reader);
}


/// Example 6 -- pick the best alternative track when a HEIF file
/// offers several variants (e.g. different resolutions of the same clip).
void example6()
{
    auto* reader = Reader::Create();

    const char* filename = "C032.heic";
    if (reader->initialize(filename) != ErrorCode::OK)
    {
        printInfo(filename);
        Reader::Destroy(reader);
        return;
    }

    FileInformation info;
    reader->getFileInformation(info);

    SequenceId trackId = 0;
    if (info.trackInformation.size > 0)
    {
        const auto& trackInfo = info.trackInformation[0];
        trackId               = trackInfo.trackId;

        // HasAlternatives means there are equivalent tracks the player can choose from.
        if (trackInfo.features & TrackFeatureEnum::HasAlternatives)
        {
            const SequenceId alternativeTrackId = trackInfo.alternateTrackIds[0];
            uint32_t alternativeWidth, trackWidth;
            reader->getDisplayWidth(alternativeTrackId, alternativeWidth);
            reader->getDisplayWidth(trackId, trackWidth);

            // Trivial picker: take whichever track is wider.
            if (trackWidth < alternativeWidth)
            {
                trackId = alternativeTrackId;
            }
        }
    }
    (void) trackId;  // ... use this trackId from here on ...
    Reader::Destroy(reader);
}


/// Example 7 -- copy a HEIF file: read every master image, feed the
/// decoder configs and bitstreams to a writer, mirror the primary item
/// flag, and copy a representative property (IROT) along the way.
void example7()
{
    auto* reader = Reader::Create();
    auto* writer = Writer::Create();

    OutputConfig writerOutputConf{};
    writerOutputConf.fileName        = "example7_output.heic";
    writerOutputConf.progressiveFile = true;

    const char* filename = "C014.heic";
    if (reader->initialize(filename) != ErrorCode::OK)
    {
        printInfo(filename);
        Reader::Destroy(reader);
        Writer::Destroy(writer);
        return;
    }

    // Mirror the source brands so the output keeps the same identity.
    FourCC inputMajorBrand{};
    reader->getMajorBrand(inputMajorBrand);
    writerOutputConf.majorBrand = inputMajorBrand;

    Array<FourCC> inputCompatibleBrands{};
    reader->getCompatibleBrands(inputCompatibleBrands);
    writerOutputConf.compatibleBrands = inputCompatibleBrands;

    if (writer->initialize(writerOutputConf) != ErrorCode::OK)
    {
        Reader::Destroy(reader);
        Writer::Destroy(writer);
        return;
    }

    FileInformation fileInfo{};
    reader->getFileInformation(fileInfo);

    // Reader and writer use independent ID spaces; map between them.
    map<DecoderConfigId, DecoderConfigId> inputToOutputDecoderConfigIds;
    map<PropertyId, Rotate>               imageRotationProperties;
    map<PropertyId, PropertyId>           inputToOutputImageProperties;

    for (const auto& image : fileInfo.rootMetaBoxInformation.itemInformations)
    {
        if (!(image.features & ItemFeatureEnum::IsMasterImage)) continue;

        // Feed the decoder config exactly once per unique config in the source.
        DecoderConfiguration inputdecoderConfig{};
        reader->getDecoderParameterSets(image.itemId, inputdecoderConfig);
        if (!inputToOutputDecoderConfigIds.count(inputdecoderConfig.decoderConfigId))
        {
            DecoderConfigId outputDecoderConfigId;
            writer->feedDecoderConfig(inputdecoderConfig.decoderSpecificInfo, outputDecoderConfigId);
            inputToOutputDecoderConfigIds[inputdecoderConfig.decoderConfigId] = outputDecoderConfigId;
        }

        // Pull the encoded bitstream and feed it to the writer.
        // bytestreamHeaders=false -> keep the on-wire NAL-length form.
        auto bytes = readItemData(reader, image.itemId, /*bytestreamHeaders=*/false);

        Data imageData{};
        imageData.data            = bytes.elements;
        imageData.size            = bytes.size;
        imageData.mediaFormat     = MediaFormat::HEVC;
        imageData.decoderConfigId = inputToOutputDecoderConfigIds.at(inputdecoderConfig.decoderConfigId);

        MediaDataId outputMediaId;
        writer->feedMediaData(imageData, outputMediaId);

        // Wrap the bitstream into an image item in the output file.
        ImageId outputImageId;
        writer->addImage(outputMediaId, outputImageId);

        // Carry over the primary-item flag.
        if (image.features & ItemFeatureEnum::IsPrimaryImage)
        {
            writer->setPrimaryItem(outputImageId);
        }

        // Carry over IROT as a representative property example.
        Array<ItemPropertyInfo> imageProperties;
        reader->getItemProperties(image.itemId, imageProperties);
        for (const auto& imageProperty : imageProperties)
        {
            if (imageProperty.type != ItemPropertyType::IROT) continue;

            // Read the property struct once per unique source property,
            // re-associate it with each image that needs it.
            if (!imageRotationProperties.count(imageProperty.index))
            {
                Rotate rotateInfo{};
                reader->getProperty(imageProperty.index, rotateInfo);
                imageRotationProperties[imageProperty.index] = rotateInfo;
                writer->addProperty(rotateInfo, inputToOutputImageProperties[imageProperty.index]);
            }
            writer->associateProperty(image.itemId,
                                      inputToOutputImageProperties.at(imageProperty.index),
                                      imageProperty.essential);
        }
    }

    writer->finalize();

    Reader::Destroy(reader);
    Writer::Destroy(writer);
}


/// Example 8 -- pull Exif metadata out of a HEIF file. Exif rides as
/// a 'cdsc' (content-describes) reference attached to the primary image.
void example8()
{
    auto* reader = Reader::Create();

    const char* filename = "C034.heic";
    if (reader->initialize(filename) != ErrorCode::OK)
    {
        printInfo(filename);
        Reader::Destroy(reader);
        return;
    }

    FileInformation fileInfo{};
    reader->getFileInformation(fileInfo);

    ImageId primaryItemId;
    reader->getPrimaryItem(primaryItemId);

    // 'cdsc' references go FROM the metadata item TO the image it describes.
    Array<ImageId> metadataIds;
    reader->getReferencedToItemListByType(primaryItemId, "cdsc", metadataIds);
    if (metadataIds.size == 0) return;
    ImageId exifItemId = metadataIds[0];

    // Belt-and-braces: confirm the referenced item is actually Exif.
    FourCC itemType;
    reader->getItemType(exifItemId, itemType);
    if (itemType != "Exif") return;

    // Pull the bytes and dump them. Note: the payload is preceded by a
    // 4-byte exif_tiff_header_offset field (ISO/IEC 23008-12 ExifDataBlock)
    // -- skip those four bytes before handing to a TIFF/Exif parser.
    auto bytes = readItemData(reader, exifItemId);

    ofstream file("exifdata.bin", ios::out | ios::binary);
    file.write(reinterpret_cast<const char*>(bytes.elements),
               static_cast<std::streamsize>(bytes.size));

    Reader::Destroy(reader);
}


/// Example 9 -- write a VVC HEIF in MiniBox (compact) format. Reads a
/// normal VVC HEIF and re-emits it as a .hmg, copying the primary
/// image plus its pixi/colr/irot/imir properties.
void example9()
{
    auto* reader = Reader::Create();
    const char* filename = "vvc_input.heif";
    if (reader->initialize(filename) != ErrorCode::OK)
    {
        printInfo(filename);
        Reader::Destroy(reader);
        return;
    }

    auto* writer = Writer::Create();
    OutputConfig writerOutputConf{};
    writerOutputConf.fileName  = "example9_output.hmg";
    writerOutputConf.writeMini = true;  // <-- the switch that asks for MiniBox output

    // MiniBox needs the source's major brand + minor version verbatim.
    reader->getMajorBrand(writerOutputConf.majorBrand);
    reader->getMinorVersion(writerOutputConf.minorVersion);

    if (writer->initialize(writerOutputConf) != ErrorCode::OK)
    {
        Reader::Destroy(reader);
        Writer::Destroy(writer);
        return;
    }

    FileInformation fileInfo{};
    reader->getFileInformation(fileInfo);

    for (const auto& item : fileInfo.rootMetaBoxInformation.itemInformations)
    {
        // Feed the decoder config (SPS/PPS/VPS bytes) to the writer.
        DecoderConfigId confId;
        DecoderConfiguration config;
        reader->getDecoderParameterSets(item.itemId, config);
        writer->feedDecoderConfig(config.decoderSpecificInfo, confId);

        // Pull the bitstream. The helper sizes the buffer for us, so any
        // per-codec expansion (compact VVC, multi-layer, ...) is handled
        // by the library, not by this example.
        auto bytes = readItemData(reader, item.itemId, /*bytestreamHeaders=*/false);

        Data imageData{};
        imageData.data            = bytes.elements;
        imageData.size            = bytes.size;
        imageData.mediaFormat     = MediaFormat::VVC;
        imageData.decoderConfigId = confId;

        MediaDataId outputMediaId;
        if (writer->feedMediaData(imageData, outputMediaId) != ErrorCode::OK)
        {
            Reader::Destroy(reader);
            Writer::Destroy(writer);
            return;
        }

        ImageId outputImageId;
        if (writer->addImage(outputMediaId, outputImageId) != ErrorCode::OK)
        {
            Reader::Destroy(reader);
            Writer::Destroy(writer);
            return;
        }

        if (item.features & ItemFeatureEnum::IsPrimaryImage)
        {
            writer->setPrimaryItem(outputImageId);
        }

        // MiniBox needs at least pixi; colr/irot/imir are also supported.
        // ispe is added for free by addImage().
        Array<ItemPropertyInfo> itemProperties;
        reader->getItemProperties(item.itemId, itemProperties);

        for (const auto& prop : itemProperties)
        {
            PropertyId propId;
            switch (prop.type)
            {
            case ItemPropertyType::PIXI:
            { PixelInformation pixi;  reader->getProperty(prop.index, pixi);
              writer->addProperty(pixi, propId); writer->associateProperty(outputImageId, propId); break; }
            case ItemPropertyType::COLR:
            { ColourInformation colr; reader->getProperty(prop.index, colr);
              writer->addProperty(colr, propId); writer->associateProperty(outputImageId, propId); break; }
            case ItemPropertyType::IROT:
            { Rotate irot;            reader->getProperty(prop.index, irot);
              writer->addProperty(irot, propId); writer->associateProperty(outputImageId, propId); break; }
            case ItemPropertyType::IMIR:
            { Mirror imir;            reader->getProperty(prop.index, imir);
              writer->addProperty(imir, propId); writer->associateProperty(outputImageId, propId); break; }
            default: break;  // ispe etc. are auto-added or skipped on purpose
            }
        }
    }

    writer->finalize();
    Writer::Destroy(writer);
    Reader::Destroy(reader);
}


/// Example 10 -- the inverse of example 9: read a VVC MiniBox (.hmg)
/// and expand it to a standard VVC HEIF (.heif) file.
void example10()
{
    auto* reader = Reader::Create();
    const char* filename = "vvc_mini_input.hmg";
    if (reader->initialize(filename) != ErrorCode::OK)
    {
        printInfo(filename);
        Reader::Destroy(reader);
        return;
    }

    auto* writer = Writer::Create();
    OutputConfig writerOutputConf{};
    writerOutputConf.fileName        = "example10_output.heif";
    writerOutputConf.progressiveFile = true;
    writerOutputConf.majorBrand      = "vvic";

    // Standard VVC HEIF needs both 'vvic' and 'mif1' in the compatible-brand list.
    Array<FourCC> compatibleBrands(2);
    compatibleBrands[0] = "vvic";
    compatibleBrands[1] = "mif1";
    writerOutputConf.compatibleBrands = compatibleBrands;

    if (writer->initialize(writerOutputConf) != ErrorCode::OK)
    {
        Reader::Destroy(reader);
        Writer::Destroy(writer);
        return;
    }

    FileInformation fileInfo{};
    reader->getFileInformation(fileInfo);

    for (const auto& item : fileInfo.rootMetaBoxInformation.itemInformations)
    {
        DecoderConfigId confId;
        DecoderConfiguration config;
        reader->getDecoderParameterSets(item.itemId, config);
        writer->feedDecoderConfig(config.decoderSpecificInfo, confId);

        // Same probe-then-allocate idiom as example 9.
        auto bytes = readItemData(reader, item.itemId, /*bytestreamHeaders=*/false);

        Data imageData{};
        imageData.data            = bytes.elements;
        imageData.size            = bytes.size;
        imageData.mediaFormat     = MediaFormat::VVC;
        imageData.decoderConfigId = confId;

        MediaDataId outputMediaId;
        if (writer->feedMediaData(imageData, outputMediaId) != ErrorCode::OK)
        {
            Reader::Destroy(reader);
            Writer::Destroy(writer);
            return;
        }

        ImageId outputImageId;
        writer->addImage(outputMediaId, outputImageId);

        if (item.features & ItemFeatureEnum::IsPrimaryImage)
        {
            writer->setPrimaryItem(outputImageId);
        }

        Array<ItemPropertyInfo> itemProperties;
        reader->getItemProperties(item.itemId, itemProperties);
        for (const auto& prop : itemProperties)
        {
            PropertyId propId;
            switch (prop.type)
            {
            case ItemPropertyType::PIXI:
            { PixelInformation pixi;  reader->getProperty(prop.index, pixi);
              writer->addProperty(pixi, propId); writer->associateProperty(outputImageId, propId); break; }
            case ItemPropertyType::COLR:
            { ColourInformation colr; reader->getProperty(prop.index, colr);
              writer->addProperty(colr, propId); writer->associateProperty(outputImageId, propId); break; }
            case ItemPropertyType::IROT:
            { Rotate irot;            reader->getProperty(prop.index, irot);
              writer->addProperty(irot, propId); writer->associateProperty(outputImageId, propId); break; }
            case ItemPropertyType::IMIR:
            { Mirror imir;            reader->getProperty(prop.index, imir);
              writer->addProperty(imir, propId); writer->associateProperty(outputImageId, propId); break; }
            default: break;
            }
        }
    }

    writer->finalize();
    Writer::Destroy(writer);
    Reader::Destroy(reader);
}


/// Example 11 -- combine two source HEIFs (a base image and an alpha
/// layer) into one output, linked by an 'auxl' auxiliary reference so
/// decoders know to use the alpha as the base image's alpha channel.
void example11()
{
    // Two readers -- one per input file.
    auto* baseReader = Reader::Create();
    if (baseReader->initialize("base_image.heif") != ErrorCode::OK)
    {
        printInfo("base_image.heif");
        Reader::Destroy(baseReader);
        return;
    }

    auto* alphaReader = Reader::Create();
    if (alphaReader->initialize("alpha_layer.heif") != ErrorCode::OK)
    {
        printInfo("alpha_layer.heif");
        Reader::Destroy(alphaReader);
        Reader::Destroy(baseReader);
        return;
    }

    // One writer for the combined output.
    auto* writer = Writer::Create();
    OutputConfig writerOutputConf{};
    writerOutputConf.fileName        = "example11_with_alpha.heif";
    writerOutputConf.progressiveFile = true;
    writerOutputConf.majorBrand      = "vvic";

    Array<FourCC> compatibleBrands(2);
    compatibleBrands[0] = "vvic";
    compatibleBrands[1] = "mif1";
    writerOutputConf.compatibleBrands = compatibleBrands;

    if (writer->initialize(writerOutputConf) != ErrorCode::OK)
    {
        Reader::Destroy(baseReader);
        Reader::Destroy(alphaReader);
        Writer::Destroy(writer);
        return;
    }

    // Local helper: copy one master image (decoder config + bytes +
    // pixi/colr/irot/imir) from a reader into the writer.
    auto copyOneMaster = [&](Reader* src, ImageId& outId, bool markPrimary) {
        FileInformation fi{};
        src->getFileInformation(fi);
        for (const auto& item : fi.rootMetaBoxInformation.itemInformations)
        {
            if (markPrimary && !(item.features & ItemFeatureEnum::IsMasterImage)) continue;

            DecoderConfigId confId;
            DecoderConfiguration config;
            src->getDecoderParameterSets(item.itemId, config);
            writer->feedDecoderConfig(config.decoderSpecificInfo, confId);

            auto bytes = readItemData(src, item.itemId, /*bytestreamHeaders=*/false);

            Data imageData{};
            imageData.data            = bytes.elements;
            imageData.size            = bytes.size;
            imageData.mediaFormat     = MediaFormat::VVC;
            imageData.decoderConfigId = confId;

            MediaDataId mediaId;
            writer->feedMediaData(imageData, mediaId);

            writer->addImage(mediaId, outId);
            if (markPrimary) writer->setPrimaryItem(outId);

            Array<ItemPropertyInfo> props;
            src->getItemProperties(item.itemId, props);
            for (const auto& prop : props)
            {
                PropertyId propId;
                switch (prop.type)
                {
                case ItemPropertyType::PIXI:
                { PixelInformation pixi;  src->getProperty(prop.index, pixi);
                  writer->addProperty(pixi, propId); writer->associateProperty(outId, propId); break; }
                case ItemPropertyType::COLR:
                { ColourInformation colr; src->getProperty(prop.index, colr);
                  writer->addProperty(colr, propId); writer->associateProperty(outId, propId); break; }
                case ItemPropertyType::IROT:
                { Rotate irot;            src->getProperty(prop.index, irot);
                  writer->addProperty(irot, propId); writer->associateProperty(outId, propId); break; }
                case ItemPropertyType::IMIR:
                { Mirror imir;            src->getProperty(prop.index, imir);
                  writer->addProperty(imir, propId); writer->associateProperty(outId, propId); break; }
                default: break;
                }
            }
        }
    };

    // Base goes in first and becomes the primary item; alpha follows.
    ImageId baseOutputImageId, alphaOutputImageId;
    copyOneMaster(baseReader,  baseOutputImageId,  /*markPrimary=*/true);
    copyOneMaster(alphaReader, alphaOutputImageId, /*markPrimary=*/false);

    // Hide the alpha image from collection enumerations and link it
    // to the base via 'auxl'. Decoders read that reference and treat
    // alpha as the base image's alpha plane.
    writer->setImageHidden(alphaOutputImageId, true);
    writer->addAuxiliaryReference(alphaOutputImageId, baseOutputImageId);

    writer->finalize();
    Writer::Destroy(writer);
    Reader::Destroy(baseReader);
    Reader::Destroy(alphaReader);
}


/// Example 12 -- error-handling protocol (the current API).
///
/// Every public HEIF API returns an integer ErrorCode -- that's the
/// contract. On top of that, the library populates a
/// thread-local diagnostic slot (HEIF::ErrorInfo) with origin / message /
/// causeWhat / file:line on failure paths. The integer says *what*
/// failed; the slot says *where AND why*.
///
/// Recommended pattern:
///   1. clearErrorInfo()                       (drains stale state)
///   2. call the API
///   3. if (rc != OK && hasErrorInfo()) ...    (read getErrorInfo())
///
/// Threading: the slot is thread_local, so it's safe to use from many
/// threads at once. Stability: pure additive side-channel -- callers
/// that ignore it see the legacy integer-only behavior unchanged.
///
void example12()
{
    cout << "\n[example 12] error handling demo\n";

    // Helper: pretty-print whatever ended up in the slot.
    auto dumpSlot = [](const char* tag) {
        if (!HEIF::hasErrorInfo()) { cout << "  " << tag << ": slot empty (pure guard return)\n"; return; }
        const HEIF::ErrorInfo& info = HEIF::getErrorInfo();
        cout << "  " << tag << ": "
             << HEIF::errorCodeName(info.code()) << " from "
             << HEIF::errorOriginName(info.origin()) << " at "
             << info.file() << ":" << info.line()
             << " -- " << info.message();
        if (info.causeWhat()[0] != '\0') cout << " (cause: " << info.causeWhat() << ")";
        cout << "\n";
    };

    // (1) Reader::initialize on a missing file -- populates the slot
    //     with origin=Reader_FileOpen and the failing function name.
    {
        HEIF::clearErrorInfo();
        auto* reader = Reader::Create();
        reader->initialize("/tmp/__example12_missing__.heic");
        dumpSlot("missing-file");
        Reader::Destroy(reader);
    }

    // (2) Writer pre-init guard -- by-design pure-guard return, no slot.
    //     Lesson: ErrorCode tells you what; slot tells you where AND why;
    //     sometimes the integer alone is already enough.
    {
        HEIF::clearErrorInfo();
        auto* writer = Writer::Create();
        Array<DecoderSpecificInfo> empty;
        DecoderConfigId id;
        writer->feedDecoderConfig(empty, id);
        dumpSlot("pre-init guard");
        Writer::Destroy(writer);
    }

    // (3) Writer disambiguation -- duplicate AVC PPS NALs trip a
    //     specific guard inside createAvcDecoderConfigurationRecord.
    //     All 21 historical DECODER_CONFIGURATION_ERROR sites used to
    //     look identical from the caller's side; the slot now names
    //     exactly which check fired.
    {
        HEIF::clearErrorInfo();
        auto* writer = Writer::Create();
        OutputConfig cfg;
        cfg.fileName         = "example12_writer_demo.heic";
        cfg.majorBrand       = "mif1";
        cfg.compatibleBrands = {"mif1", "heic", "hevc"};
        if (writer->initialize(cfg) == ErrorCode::OK)
        {
            // Two PPS NALs back-to-back trip the duplicate-NAL branch.
            std::vector<uint8_t> nalBytes1 = {0x68, 0x01, 0x02, 0x03};
            std::vector<uint8_t> nalBytes2 = {0x68, 0x10, 0x20, 0x30};

            Array<DecoderSpecificInfo> nals(2);
            nals.elements[0].decSpecInfoType = DecoderSpecInfoType::AVC_PPS;
            nals.elements[0].id              = 0;
            nals.elements[0].decSpecInfoData = Array<uint8_t>(nalBytes1.size());
            for (size_t i = 0; i < nalBytes1.size(); ++i)
                nals.elements[0].decSpecInfoData.elements[i] = nalBytes1[i];

            nals.elements[1].decSpecInfoType = DecoderSpecInfoType::AVC_PPS;
            nals.elements[1].id              = 0;
            nals.elements[1].decSpecInfoData = Array<uint8_t>(nalBytes2.size());
            for (size_t i = 0; i < nalBytes2.size(); ++i)
                nals.elements[1].decSpecInfoData.elements[i] = nalBytes2[i];

            DecoderConfigId cfgId;
            writer->feedDecoderConfig(nals, cfgId);

            std::vector<uint8_t> payload = {0x00, 0x00, 0x00, 0x01, 0x65};
            Data d;
            d.size            = payload.size();
            d.data            = payload.data();
            d.mediaFormat     = MediaFormat::AVC;
            d.decoderConfigId = cfgId;

            MediaDataId mediaId;
            if (writer->feedMediaData(d, mediaId) == ErrorCode::OK)
            {
                HEIF::clearErrorInfo();
                ImageId imageId;
                writer->addImage(mediaId, imageId);
                dumpSlot("duplicate-PPS");
            }
            else
            {
                // Some build configs reject the bytes earlier in feedMediaData.
                dumpSlot("feedMediaData rejection");
            }
            Writer::Destroy(writer);
            std::remove("example12_writer_demo.heic");
        }
        else
        {
            Writer::Destroy(writer);
        }
    }

    // (4) VVC parser side-channel.
    //
    // The standalone VVC NAL parsers (Sps/Pps/Vps/Opi) don't return
    // ErrorCode -- their contract is "give back a default-constructed
    // Info on failure". Without a side-channel the caller couldn't
    // tell "buffer was empty" from "parser silently recovered". The
    // slot now carries the diagnostic even though the return type is
    // unchanged. The parser headers themselves aren't part of
    // the installed include set so we don't bind one here.
    cout << "  vvc parsers: populate the slot on RBSP-prepare failures\n"
            "               (origin=Parser_VvcRbsp) and per-parser inner\n"
            "               catch sites (Parser_Vvc{Sps,Pps,Vps,Opi}).\n";

    // (5) Slot is unconsulted after a successful call.
    //     Discipline: clearErrorInfo() before, only read on non-OK.
    {
        HEIF::clearErrorInfo();
        auto* reader = Reader::Create();
        const ErrorCode rc = reader->initialize("C003.heic");
        if (rc != ErrorCode::OK)
        {
            dumpSlot("C003 open");
        }
        Reader::Destroy(reader);
        HEIF::clearErrorInfo();
    }
}


/// Example 13 -- mux a plain MP4 video track and an AAC-LC audio track
/// into the same file. Skeleton: shows the API call shape, not real
/// bitstream feeding (drop in your own feedDecoderConfig + feedMediaData
/// + addVideo / addAudio loop in decoding order, as in example 7).
///
/// HEIF and MP4 share the ISOBMFF box machinery, so the same Writer
/// handle produces both. Three handler types can co-exist in one file:
///   * 'pict' / 'auxv'  -- HEIF image-sequence tracks (addImage*)
///   * 'vide'           -- plain MP4 video track     (addVideoTrack)
///   * 'soun'           -- plain MP4 audio track     (addAudioTrack)
///
/// Restrictions: video = AVC / HEVC / VVC only, audio = AAC-LC only,
/// plain progressive MP4 only (no fragmented writer), no DRM.
void example13()
{
    auto* writer = Writer::Create();

    OutputConfig cfg{};
    cfg.fileName         = "example13_mp4.mp4";
    cfg.progressiveFile  = true;
    cfg.majorBrand       = "mp42";
    cfg.compatibleBrands = {"mp42", "isom", "hevc"};

    if (writer->initialize(cfg) != ErrorCode::OK)
    {
        Writer::Destroy(writer);
        return;
    }

    // (1) Plain MP4 video track. From here on, feed bitstream + decoder
    //     parameter sets exactly as for HEIF still images (see example 7),
    //     then append samples with addVideo() instead of addImage().
    SequenceId videoTrackId;
    writer->addVideoTrack({1, 30000}, videoTrackId);  // 30000-tick timescale

    // (2) Plain MP4 audio track. averageBitrate / maxBitrate == 0 means
    //     "unknown" per ISO/IEC 14496-12. The HEIFPP wrapper computes
    //     them from sample sizes; on the raw API you provide them.
    AudioParams audio{};
    audio.channelCount   = 2;
    audio.sampleRate     = 48000;
    audio.averageBitrate = 0;
    audio.maxBitrate     = 0;
    SequenceId audioTrackId;
    writer->addAudioTrack({1, 48000}, audio, audioTrackId);

    // ... feedDecoderConfig + feedMediaData + addVideo / addAudio per
    // sample, in decoding order, then writer->finalize().
    // (Omitted -- this example is about wiring up the entry points;
    // example 7 shows the full feed loop for HEIF images.)

    Writer::Destroy(writer);
    std::remove("example13_mp4.mp4");
}


// printInfo() used to render a verbose diagnostic by hand. Now the
/// Example 14 -- segment-streaming (DASH) playback.
///
/// The reader supports fragmented MP4: feed an initialization segment
/// (moov) once, then parse media segments (moof+mdat) one at a time.
/// Segments can be invalidated when the player seeks, freeing memory
/// without tearing down the reader.
///
/// Asset: MIAF005_fragmented.heic from the heif_conformance suite.
///        The same file serves both as init and media segment here
///        because the conformance asset packs them into one file.

// Minimal StreamInterface backed by std::ifstream.
// Production code would wrap a network buffer, mmap, or CDN fetch.
struct SimpleFileStream : public StreamInterface
{
    std::ifstream mFile;
    StreamInterface::offset_t mSize = 0;

    explicit SimpleFileStream(const char* path)
        : mFile(path, std::ios::binary)
    {
        if (mFile.good())
        {
            mFile.seekg(0, std::ios::end);
            mSize = static_cast<StreamInterface::offset_t>(mFile.tellg());
            mFile.seekg(0);
        }
    }

    StreamInterface::offset_t read(char* buffer, StreamInterface::offset_t size) override
    {
        mFile.read(buffer, size);
        return static_cast<StreamInterface::offset_t>(mFile.gcount());
    }

    bool absoluteSeek(StreamInterface::offset_t offset) override
    {
        mFile.clear();
        mFile.seekg(offset);
        return mFile.good();
    }

    StreamInterface::offset_t tell() override { return static_cast<StreamInterface::offset_t>(mFile.tellg()); }
    StreamInterface::offset_t size() override { return mSize; }
};

void example14()
{
    cout << "Example 14: segment-streaming (DASH) playback" << endl;

    const char* filename = "MIAF005_fragmented.heic";

    // One stream for the init segment, one for media data.
    SimpleFileStream initStream(filename);
    SimpleFileStream mediaStream(filename);
    if (initStream.size() == 0)
    {
        printInfo(filename);
        return;
    }

    auto* reader = Reader::Create();

    // Step 1: parse the initialization segment (moov box).
    // This gives the reader track layout, codec config, and timing.
    if (reader->parseInitializationSegment(&initStream) != ErrorCode::OK)
    {
        cout << "  parseInitializationSegment failed" << endl;
        Reader::Destroy(reader);
        return;
    }

    // Step 2: parse a media segment (moof+mdat).
    // Each segment needs a unique SegmentId.  In a DASH player this
    // would be assigned per HTTP response; here we use 1.
    const SegmentId segId = 1;
    if (reader->parseSegment(&mediaStream, segId) != ErrorCode::OK)
    {
        cout << "  parseSegment failed" << endl;
        Reader::Destroy(reader);
        return;
    }

    // Step 3: query tracks and read samples -- identical to file mode.
    FileInformation info;
    reader->getFileInformation(info);
    cout << "  Tracks: " << info.trackInformation.size << endl;

    for (uint32_t t = 0; t < info.trackInformation.size; ++t)
    {
        const auto trackId = info.trackInformation[t].trackId;
        Array<TimestampIDPair> timestamps;
        reader->getItemTimestamps(trackId, timestamps);
        cout << "  Track " << trackId.get()
             << ": " << timestamps.size << " samples" << endl;

        // Fetch first sample to prove data is accessible.
        if (timestamps.size > 0)
        {
            uint64_t bufSize = 0;
            reader->getItemDataWithDecoderParameters(trackId, timestamps[0].itemId,
                                                     nullptr, bufSize);
            vector<uint8_t> buf(bufSize);
            reader->getItemDataWithDecoderParameters(trackId, timestamps[0].itemId,
                                                     buf.data(), bufSize);
            cout << "  First sample: " << bufSize << " bytes" << endl;
        }
    }

    // Step 4: invalidate the segment -- the reader drops its reference
    // to the media stream.  After this, samples from segment 1 are no
    // longer readable, but the init-segment state (track layout, codec
    // config) stays alive.  A player would now feed the next segment.
    reader->invalidateSegment(segId);
    cout << "  Segment " << segId.get() << " invalidated" << endl;

    Reader::Destroy(reader);
    cout << "  Done." << endl;
}


/// Example 15 -- raw (custom) property round-trip.
///
/// Demonstrates how a caller can attach an arbitrary property to a HEIF
/// image using RawProperty.  This is the escape hatch for box types the
/// library does not yet have typed support for -- if a future spec
/// amendment defines a new descriptive property (say 'myPr'), the
/// caller can still write it today by pre-serialising the box content
/// and passing the blob through Writer::addProperty(RawProperty, ...).
///
/// The example:
///   1. Writes a minimal HEVC HEIF with a primary image.
///   2. Builds a hypothetical 'myPr' property as a raw ISOBMFF box
///      (8-byte header + payload) and attaches it to the image.
///   3. Finalises the file.
///   4. Reads it back, locates the RAW property, and verifies the
///      round-tripped bytes match what was written.
void example15()
{
    cout << endl << "Example 15: raw (custom) property round-trip" << endl;

    // --- Write phase ---------------------------------------------------

    auto* writer = Writer::Create();
    OutputConfig writerCfg{};
    writerCfg.fileName  = "example15_raw_property.heic";
    writerCfg.majorBrand = "mif1";
    writerCfg.compatibleBrands = {"mif1", "heic"};
    if (writer->initialize(writerCfg) != ErrorCode::OK)
    {
        cout << "  writer->initialize failed" << endl;
        Writer::Destroy(writer);
        return;
    }

    // Minimal HEVC decoder config (VPS/SPS/PPS placeholder -- structure
    // is valid enough for the writer to accept but the NALs won't decode
    // a real picture).
    const uint8_t dummyVps[] = {
        0x40, 0x01, 0x0C, 0x01, 0xFF, 0xFF, 0x01, 0x60,
        0x00, 0x00, 0x03, 0x00, 0x80, 0x00, 0x00, 0x03,
        0x00, 0x00, 0x03, 0x00, 0x78, 0x9D, 0xC0, 0x90
    };
    const uint8_t dummySps[] = {
        0x42, 0x01, 0x01, 0x01, 0x60, 0x00, 0x00, 0x00,
        0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0xF0,
        0x24, 0x00, 0xBA, 0x80, 0x01, 0x00, 0x04, 0x01
    };
    const uint8_t dummyPps[] = {0x44, 0x01, 0xC1, 0x73, 0xD1, 0x89};
    Array<DecoderSpecificInfo> decInfo(3);
    decInfo[0].decSpecInfoType = DecoderSpecInfoType::HEVC_VPS;
    decInfo[0].decSpecInfoData = Array<uint8_t>(dummyVps, dummyVps + sizeof(dummyVps));
    decInfo[1].decSpecInfoType = DecoderSpecInfoType::HEVC_SPS;
    decInfo[1].decSpecInfoData = Array<uint8_t>(dummySps, dummySps + sizeof(dummySps));
    decInfo[2].decSpecInfoType = DecoderSpecInfoType::HEVC_PPS;
    decInfo[2].decSpecInfoData = Array<uint8_t>(dummyPps, dummyPps + sizeof(dummyPps));

    DecoderConfigId decCfgId;
    writer->feedDecoderConfig(decInfo, decCfgId);

    // Dummy coded image data (one-byte placeholder).
    uint8_t dummyBitstream[] = {0x00};
    Data imageData{};
    imageData.data            = dummyBitstream;
    imageData.size            = sizeof(dummyBitstream);
    imageData.mediaFormat     = MediaFormat::HEVC;
    imageData.decoderConfigId = decCfgId;

    MediaDataId mediaId;
    writer->feedMediaData(imageData, mediaId);

    ImageId imageId;
    writer->addImage(mediaId, imageId);
    writer->setPrimaryItem(imageId);

    // Build the raw property.  The blob must be a complete ISOBMFF box:
    //   bytes 0..3  = uint32 big-endian box size (header + payload)
    //   bytes 4..7  = four-character box type
    //   bytes 8..N  = payload
    // A FullBox would add version(1) + flags(3) after the type.
    const char boxType[4] = {'m', 'y', 'P', 'r'};
    const uint8_t payload[] = {0xCA, 0xFE, 0xBA, 0xBE, 0x01, 0x02, 0x03, 0x04};
    const uint32_t boxSize  = 8 + sizeof(payload);  // 16 bytes

    // Assemble the raw box into a flat buffer.
    vector<uint8_t> rawBox(boxSize);
    rawBox[0] = static_cast<uint8_t>((boxSize >> 24) & 0xFF);
    rawBox[1] = static_cast<uint8_t>((boxSize >> 16) & 0xFF);
    rawBox[2] = static_cast<uint8_t>((boxSize >>  8) & 0xFF);
    rawBox[3] = static_cast<uint8_t>((boxSize >>  0) & 0xFF);
    rawBox[4] = static_cast<uint8_t>(boxType[0]);
    rawBox[5] = static_cast<uint8_t>(boxType[1]);
    rawBox[6] = static_cast<uint8_t>(boxType[2]);
    rawBox[7] = static_cast<uint8_t>(boxType[3]);
    std::copy(payload, payload + sizeof(payload), rawBox.begin() + 8);

    RawProperty rawProp{};
    rawProp.type = FourCC("myPr");
    rawProp.data = Array<uint8_t>(rawBox.data(), rawBox.data() + rawBox.size());

    PropertyId rawPropId;
    if (writer->addProperty(rawProp, /*isTransformative=*/false, rawPropId) != ErrorCode::OK)
    {
        cout << "  addProperty(RawProperty) failed" << endl;
        Writer::Destroy(writer);
        return;
    }
    writer->associateProperty(imageId, rawPropId, /*isEssential=*/false);

    if (writer->finalize() != ErrorCode::OK)
    {
        cout << "  writer->finalize failed" << endl;
        Writer::Destroy(writer);
        return;
    }
    Writer::Destroy(writer);
    cout << "  Written: " << writerCfg.fileName << endl;

    // --- Read phase ----------------------------------------------------

    auto* reader = Reader::Create();
    if (reader->initialize(writerCfg.fileName) != ErrorCode::OK)
    {
        cout << "  reader->initialize failed (file just written)" << endl;
        Reader::Destroy(reader);
        return;
    }

    ImageId primaryId;
    reader->getPrimaryItem(primaryId);

    Array<ItemPropertyInfo> props;
    reader->getItemProperties(primaryId, props);

    bool found = false;
    for (uint32_t i = 0; i < props.size; ++i)
    {
        if (props[i].type != ItemPropertyType::RAW) continue;

        // Retrieve the raw blob.
        RawProperty readBack{};
        if (reader->getProperty(props[i].index, readBack) != ErrorCode::OK)
        {
            cout << "  getProperty(RawProperty) failed for index "
                 << props[i].index.get() << endl;
            continue;
        }

        cout << "  Found raw property:  type='" << readBack.type.value
             << "'  size=" << readBack.data.size << " bytes" << endl;

        // Verify the payload survived the round-trip.
        if (readBack.data.size == rawBox.size())
        {
            bool match = true;
            for (size_t b = 0; b < rawBox.size(); ++b)
            {
                if (readBack.data[b] != rawBox[b]) { match = false; break; }
            }
            cout << "  Round-trip: "
                 << (match ? "PASS (bytes match)" : "FAIL (mismatch)")
                 << endl;
        }
        else
        {
            cout << "  Round-trip: FAIL (size mismatch: wrote "
                 << rawBox.size() << ", read " << readBack.data.size << ")" << endl;
        }
        found = true;
    }
    if (!found)
    {
        cout << "  No RAW property found on primary item (unexpected)" << endl;
    }

    Reader::Destroy(reader);
    cout << "  Done." << endl;
}


// stderr sink installed in main() prints the [heif] line automatically
// the moment the slot is written, so we only need the human hint here.
void printInfo(const char* filename)
{
    cout << "Can't find input file: " << filename
         << "  (download from https://github.com/nokiatech/heif_conformance "
         << "and place next to the executable.)\n";
}


int main()
{
    // One-line install: every C-tier diagnostic now surfaces as a "[heif] ..."
    // line on stderr. Embedders typically swap this for their own logger
    // (Android Log, syslog, GUI dialog, crash reporter).
    HEIF::setErrorSink(&HEIF::stderrErrorSink, stderr);

    // Top-level firewall. HEIFPP/HEIF return ErrorCode for expected
    // failures, but MAY throw in two cases:
    //   * std::logic_error -- a HEIF_ASSERT tripped; treat as a bug.
    //   * other std::exception -- an unanticipated path escaped a lower
    //                             catch; treat as malformed input.
    // The library leaves itself in a defined state after the throw, so
    // ONE handler at the entry point is enough. Copy this block into
    // your own application's main().
    try
    {
        example1();
        example2();
        example3();
        example4();
        example5();
        example6();
        example7();
        example8();
        example9();
        example10();
        example11();
        example12();
        example13();
        example14();
        example15();
        return 0;
    }
    catch (const std::logic_error& e)
    {
        std::cerr << "[example] HEIF invariant violated: " << e.what()
                  << "  (this is a bug -- please file a report)\n";
        return 2;
    }
    catch (const std::exception& e)
    {
        std::cerr << "[example] HEIF threw: " << e.what()
                  << "  (see [heif] sink output above for origin)\n";
        return 1;
    }
    catch (...)
    {
        std::cerr << "[example] HEIF threw an unknown exception "
                     "(should never happen -- please file a report)\n";
        return 1;
    }
}
