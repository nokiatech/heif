/* This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2018 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its
 * subsidiaries. All rights are reserved.
 *
 * Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior
 * written consent of Nokia.
 */

/** This file includes some examples about using the reader API and writer API (example7)
 *  Note that binary execution fails if proper .heic files are not located in the directory */

#include <algorithm>  // std::min_element
#include <fstream>
#include <iostream>
#include <map>
#include "heifreader.h"
#include "heifwriter.h"

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

/// Access and read a cover image
void example1()
{
    auto* reader = Reader::Create();
    // Input file available from https://github.com/nokiatech/heif_conformance
    const char* filename = "C003.heic";
    if (reader->initialize(filename) == ErrorCode::OK)
    {
        FileInformation info;
        reader->getFileInformation(info);

        // Find the item ID
        ImageId itemId;
        reader->getPrimaryItem(itemId);

        uint64_t memoryBufferSize = 1024 * 1024;
        uint8_t* memoryBuffer     = new uint8_t[memoryBufferSize];
        reader->getItemDataWithDecoderParameters(itemId, memoryBuffer, memoryBufferSize);

        // Feed 'data' to decoder and display the cover image...

        delete[] memoryBuffer;
    }
    else
    {
        cout << "Can't find input file: " << filename << ". "
             << "Please download it from https://github.com/nokiatech/heif_conformance "
             << "and place it in same directory with the executable." << endl;
    }

    Reader::Destroy(reader);
}

/// Access and read image item and its thumbnail
void example2()
{
    Array<ImageId> itemIds;
    uint64_t itemSize = 1024 * 1024;
    uint8_t* itemData = new uint8_t[itemSize];

    auto* reader = Reader::Create();

    // Input file available from https://github.com/nokiatech/heif_conformance
    const char* filename = "C012.heic";
    if (reader->initialize(filename) == ErrorCode::OK)
    {
        FileInformation info;
        if (reader->getFileInformation(info) != ErrorCode::OK)
        {
            // handle error here...
            delete[] itemData;
            return;
        }

        // Verify that the file has one or several images in the MetaBox
        if (!(info.features & FileFeatureEnum::HasSingleImage || info.features & FileFeatureEnum::HasImageCollection))
        {
            delete[] itemData;
            return;
        }

        // Find the item ID of the first master image
        reader->getMasterImages(itemIds);
        const ImageId masterId = itemIds[0];

        const auto metaBoxFeatures = info.rootMetaBoxInformation.features;  // For convenience
        if (metaBoxFeatures & MetaBoxFeatureEnum::HasThumbnails)
        {
            // Thumbnail references ('thmb') are from the thumbnail image to the master image
            reader->getReferencedToItemListByType(masterId, "thmb", itemIds);
            const auto thumbnailId = itemIds[0];

            if (reader->getItemDataWithDecoderParameters(thumbnailId.get(), itemData, itemSize) == ErrorCode::OK)
            {
                // ...decode data and display the image, show master image later
            }
        }
        else
        {
            // There was no thumbnail, show just the master image
            if (reader->getItemDataWithDecoderParameters(masterId.get(), itemData, itemSize) == ErrorCode::OK)
            {
                // ...decode and display...
            }
        }
    }
    else
    {
        cout << "Can't find input file: " << filename << ". "
             << "Please download it from https://github.com/nokiatech/heif_conformance "
             << "and place it in same directory with the executable." << endl;
    }
    Reader::Destroy(reader);
    delete[] itemData;
}

/// Access and read image items and their thumbnails in a collection
void example3()
{
    auto* reader = Reader::Create();
    typedef ImageId MasterItemId;
    typedef ImageId ThumbnailItemId;
    Array<ImageId> itemIds;
    map<MasterItemId, ThumbnailItemId> imageMap;

    // Input file available from https://github.com/nokiatech/heif_conformance
    const char* filename = "C012.heic";
    if (reader->initialize(filename) == ErrorCode::OK)
    {
        FileInformation info;
        reader->getFileInformation(info);

        // Find item IDs of master images
        reader->getMasterImages(itemIds);

        // Find thumbnails for each master. There can be several thumbnails for one master image, get narrowest ones
        // here.
        for (const auto masterId : itemIds)
        {
            // Thumbnail references ('thmb') are from the thumbnail image to the master image
            Array<ImageId> thumbIds;
            reader->getReferencedToItemListByType(masterId, "thmb", thumbIds);

            const ImageId thumbId = *std::min_element(thumbIds.begin(), thumbIds.end(), [&](ImageId a, ImageId b) {
                uint32_t widthA, widthB;
                reader->getWidth(a, widthA);
                reader->getWidth(b, widthB);
                return (widthA < widthB);
            });
            imageMap[masterId]    = thumbId;
        }
    }
    else
    {
        cout << "Can't find input file: " << filename << ". "
             << "Please download it from https://github.com/nokiatech/heif_conformance "
             << "and place it in same directory with the executable." << endl;
    }

    // We have now item IDs of thumbnails and master images. Decode and show them from imageMap as wanted.
    Reader::Destroy(reader);
}

/// Access and read derived images
void example4()
{
    auto* reader = Reader::Create();
    Array<ImageId> itemIds;

    // Input file available from https://github.com/nokiatech/heif_conformance
    const char* filename = "C008.heic";
    if (reader->initialize(filename) == ErrorCode::OK)
    {
        FileInformation info;
        reader->getFileInformation(info);

        // Find item IDs of 'iden' (identity transformation) type derived images
        reader->getItemListByType("iden", itemIds);

        const auto itemId = itemIds[0];  // For demo purposes, assume there was one 'iden' item

        // 'dimg' item reference points from the 'iden' derived item to the input(s) of the derivation
        Array<ImageId> referencedImages;
        reader->getReferencedFromItemListByType(itemId, "dimg", referencedImages);
        const ImageId sourceItemId = referencedImages[0];  // For demo purposes, assume there was one

        // Get 'iden' item properties to find out what kind of derivation it is
        Array<ItemPropertyInfo> propertyInfos;
        reader->getItemProperties(itemId, propertyInfos);

        unsigned int rotation = 0;
        for (const auto& property : propertyInfos)
        {
            // For example, handle 'irot' transformational property is anti-clockwise rotation
            if (property.type == ItemPropertyType::IROT)
            {
                // Get property struct by index to access rotation angle
                Rotate irot;
                reader->getProperty(property.index, irot);
                rotation = irot.angle;
                break;  // Assume only one property
            }
        }

        cout << "To render derived image item ID " << itemId.get() << ":" << endl;
        cout << "-retrieve data for source image item ID " << sourceItemId.get() << endl;
        cout << "-rotating image " << rotation << " degrees." << endl;
    }
    else
    {
        cout << "Can't find input file: " << filename << ". "
             << "Please download it from https://github.com/nokiatech/heif_conformance "
             << "and place it in same directory with the executable." << endl;
    }

    Reader::Destroy(reader);
}

/// Access and read media track samples, thumbnail track samples and timestamps
void example5()
{
    auto* reader = Reader::Create();
    Array<uint32_t> itemIds;

    // Input file available from https://github.com/nokiatech/heif_conformance
    const char* filename = "C032.heic";
    if (reader->initialize(filename) == ErrorCode::OK)
    {
        FileInformation info;
        reader->getFileInformation(info);

        // Print information for every track read
        for (const auto& trackProperties : info.trackInformation)
        {
            const auto sequenceId = trackProperties.trackId;
            cout << "Track ID " << sequenceId.get() << endl;  // Context ID corresponds to the track ID

            if (trackProperties.features & TrackFeatureEnum::IsMasterImageSequence)
            {
                cout << "This is a master image sequence." << endl;
            }

            if (trackProperties.features & TrackFeatureEnum::IsThumbnailImageSequence)
            {
                // Assume there is only one type track reference, so check reference type and master track ID(s) from
                // the first one.
                const auto tref = trackProperties.referenceTrackIds[0];
                cout << "Track reference type is '" << tref.type.value << "'" << endl;
                cout << "This is a thumbnail track for track ID ";
                for (const auto masterTrackId : tref.trackIds)
                {
                    cout << masterTrackId.get() << endl;
                }
            }

            Array<TimestampIDPair> timestamps;
            reader->getItemTimestamps(sequenceId, timestamps);
            cout << "Sample timestamps:" << endl;
            for (const auto& timestamp : timestamps)
            {
                cout << " Timestamp=" << timestamp.timeStamp << "ms, sample ID=" << timestamp.itemId.get() << endl;
            }

            for (const auto& sampleProperties : trackProperties.sampleProperties)
            {
                // A sample might have decoding dependencies. The simplest way to handle this is just to always ask and
                // decode all dependencies.
                Array<SequenceImageId> itemsToDecode;
                reader->getDecodeDependencies(sequenceId, sampleProperties.sampleId, itemsToDecode);
                for (auto dependencyId : itemsToDecode)
                {
                    uint64_t size       = 1024 * 1024;
                    uint8_t* sampleData = new uint8_t[size];
                    reader->getItemDataWithDecoderParameters(sequenceId, dependencyId, sampleData, size);

                    // Feed data to decoder...

                    delete[] sampleData;
                }
                // Store or show the image...
            }
        }
    }
    else
    {
        cout << "Can't find input file: " << filename << ". "
             << "Please download it from https://github.com/nokiatech/heif_conformance "
             << "and place it in same directory with the executable." << endl;
    }

    Reader::Destroy(reader);
}

/// Access and read media alternative
void example6()
{
    auto* reader = Reader::Create();
    Array<uint32_t> itemIds;

    // Input file available from https://github.com/nokiatech/heif_conformance
    const char* filename = "C032.heic";
    if (reader->initialize(filename) == ErrorCode::OK)
    {
        FileInformation info;
        reader->getFileInformation(info);

        SequenceId trackId = 0;

        if (info.trackInformation.size > 0)
        {
            const auto& trackInfo = info.trackInformation[0];
            trackId               = trackInfo.trackId;

            if (trackInfo.features & TrackFeatureEnum::HasAlternatives)
            {
                const SequenceId alternativeTrackId = trackInfo.alternateTrackIds[0];  // Take the first alternative
                uint32_t alternativeWidth;
                reader->getDisplayWidth(alternativeTrackId, alternativeWidth);
                uint32_t trackWidth;
                reader->getDisplayWidth(trackId, trackWidth);

                if (trackWidth > alternativeWidth)
                {
                    cout << "The alternative track has wider display width, let's use it from now on..." << endl;
                    trackId = alternativeTrackId;
                }
            }
        }
    }
    else
    {
        cout << "Can't find input file: " << filename << ". "
             << "Please download it from https://github.com/nokiatech/heif_conformance "
             << "and place it in same directory with the executable." << endl;
    }
    Reader::Destroy(reader);
}

// Example about reading file and writing it to output. Note most error checking omitted.
void example7()
{
    // create reader and writer instances
    auto* reader = Reader::Create();
    auto* writer = Writer::Create();

    // partially configure writer output
    OutputConfig writerOutputConf{};
    writerOutputConf.fileName        = "example7_output.heic";
    writerOutputConf.progressiveFile = true;

    // Input file available from https://github.com/nokiatech/heif_conformance
    const char* filename = "C014.heic";
    if (reader->initialize(filename) == ErrorCode::OK)
    {
        // read major brand of file and store it to writer config.
        FourCC inputMajorBrand{};
        reader->getMajorBrand(inputMajorBrand);

        // add major brand to writer config
        writerOutputConf.majorBrand = inputMajorBrand;

        // read compatible brands of file and store it to writer config.
        Array<FourCC> inputCompatibleBrands{};
        reader->getCompatibleBrands(inputCompatibleBrands);

        // add compatible brands to writer config
        writerOutputConf.compatibleBrands = inputCompatibleBrands;

        // initialize writer now that we have all the needed information from reader
        if (writer->initialize(writerOutputConf) == ErrorCode::OK)
        {
            // get information about all input file content
            FileInformation fileInfo{};
            reader->getFileInformation(fileInfo);

            // map which input decoder config id matches the writer output decoder config ids
            map<DecoderConfigId, DecoderConfigId> inputToOutputDecoderConfigIds;

            // Image Rotation property as an example of Image Properties:
            map<PropertyId, Rotate> imageRotationProperties;

            // map which input image property maach the writer output property.
            map<PropertyId, PropertyId> inputToOutputImageProperties;

            // go through all items in input file and store master image decoder configs
            for (const auto& image : fileInfo.rootMetaBoxInformation.itemInformations)
            {
                if (image.features & ItemFeatureEnum::IsMasterImage)
                {
                    // read image decoder config and store its id if not seen before
                    DecoderConfiguration inputdecoderConfig{};
                    reader->getDecoderParameterSets(image.itemId, inputdecoderConfig);
                    if (!inputToOutputDecoderConfigIds.count(inputdecoderConfig.decoderConfigId))
                    {
                        // feed new decoder config to writer and store input to output id pairing information
                        DecoderConfigId outputDecoderConfigId;
                        writer->feedDecoderConfig(inputdecoderConfig.decoderSpecificInfo, outputDecoderConfigId);
                        inputToOutputDecoderConfigIds[inputdecoderConfig.decoderConfigId] = outputDecoderConfigId;
                    }

                    // read image data
                    Data imageData{};
                    imageData.size = image.size;
                    imageData.data = new uint8_t[imageData.size];
                    reader->getItemData(image.itemId, imageData.data, imageData.size, false);

                    // feed image data to writer
                    imageData.mediaFormat     = MediaFormat::HEVC;
                    imageData.decoderConfigId = inputToOutputDecoderConfigIds.at(inputdecoderConfig.decoderConfigId);
                    MediaDataId outputMediaId;
                    writer->feedMediaData(imageData, outputMediaId);
                    delete[] imageData.data;

                    // create new image based on that data:
                    ImageId outputImageId;
                    writer->addImage(outputMediaId, outputImageId);

                    // if this input image was the primary image -> also mark output image as primary image
                    if (image.features & ItemFeatureEnum::IsPrimaryImage)
                    {
                        writer->setPrimaryItem(outputImageId);
                    }

                    // copy other properties over
                    Array<ItemPropertyInfo> imageProperties;
                    reader->getItemProperties(image.itemId, imageProperties);

                    for (const auto& imageProperty : imageProperties)
                    {
                        switch (imageProperty.type)
                        {
                        case ItemPropertyType::IROT:
                        {
                            if (!imageRotationProperties.count(imageProperty.index))
                            {
                                // if we haven't yet read this property for other image -> do so and add it to writer as
                                // well
                                Rotate rotateInfo{};
                                reader->getProperty(imageProperty.index, rotateInfo);
                                imageRotationProperties[imageProperty.index] = rotateInfo;

                                // create new property for images in writer
                                writer->addProperty(rotateInfo, inputToOutputImageProperties[imageProperty.index]);
                            }
                            // associate property with this output image
                            writer->associateProperty(image.itemId,
                                                      inputToOutputImageProperties.at(imageProperty.index),
                                                      imageProperty.essential);
                            break;
                        }
                        //
                        // ... other properties
                        //
                        default:
                            break;
                        }
                    }
                }
            }

            writer->finalize();
        }
    }
    else
    {
        cout << "Can't find input file: " << filename << ". "
             << "Please download it from https://github.com/nokiatech/heif_conformance "
             << "and place it in same directory with the executable." << endl;
    }
    Reader::Destroy(reader);
    Writer::Destroy(writer);
}

/// Read Exif metadata. Note that most error checking is omitted.
void example8()
{
    auto* reader = Reader::Create();

    // Try opening a file with an "Exif" item.
    // The file is available from https://github.com/nokiatech/heif_conformance
    const char* filename = "C034.heic";
    if (reader->initialize(filename) != ErrorCode::OK)
    {
        cout << "Can't find input file: " << filename << ". "
             << "Please download it from https://github.com/nokiatech/heif_conformance "
             << "and place it in same directory with the executable." << endl;
        return;
    }

    FileInformation fileInfo{};
    reader->getFileInformation(fileInfo);

    // Find the primary item ID.
    ImageId primaryItemId;
    reader->getPrimaryItem(primaryItemId);

    // Find item(s) referencing to the primary item with "cdsc" (content describes) item reference.
    Array<ImageId> metadataIds;
    reader->getReferencedToItemListByType(primaryItemId, "cdsc", metadataIds);
    ImageId exifItemId = metadataIds[0];

    // Optional: verify the item ID we got is really of "Exif" type.
    FourCC itemType;
    reader->getItemType(exifItemId, itemType);
    if (itemType != "Exif")
    {
        return;
    }

    // Get item size from parsed information. For simplicity, assume it is the first and only non-image item in the
    // file.
    if (fileInfo.rootMetaBoxInformation.itemInformations[0].itemId != exifItemId)
    {
        return;
    }
    auto itemSize = fileInfo.rootMetaBoxInformation.itemInformations[0].size;

    // Request item data.
    uint8_t* memoryBuffer = new uint8_t[itemSize];
    reader->getItemData(metadataIds[0], memoryBuffer, itemSize);

    // Write Exif item data to a file.
    // Note this data does not contain Exif payload only. The payload is preceded by 4-byte exif_tiff_header_offset
    // field as defined by class ExifDataBlock() in ISO/IEC 23008-12:2017.
    ofstream file("exifdata.bin", ios::out | ios::binary);
    file.write(reinterpret_cast<char*>(memoryBuffer), static_cast<std::streamsize>(itemSize));
    delete[] memoryBuffer;

    Reader::Destroy(reader);
}

int main()
{
    example1();
    example2();
    example3();
    example4();
    example5();
    example6();
    example7();
    example8();
}
