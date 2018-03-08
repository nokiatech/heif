/* This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2018 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its subsidiaries. All rights are reserved.
 *
 * Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior written consent of Nokia.
 */

/** This file includes some examples about using the reader API
 *  Note that binary execution fails if proper .heic files are not located in the directory */

#include <algorithm>
#include <iostream>
#include <map>
#include "heifreader.h"

using namespace std;
using namespace HEIF;

void example1();
void example2();
void example3();
void example4();
void example5();
void example6();

/// Access and read a cover image
void example1()
{
    auto* reader = HeifReaderInterface::Create();
    reader->initialize("test_001.heic");

    FileInformation info;
    reader->getFileInformation(info);

    // Find the item ID
    ImageId itemId;
    reader->getPrimaryItem(itemId);

    uint32_t memoryBufferSize = 1024 * 1024;
    char* memoryBuffer        = new char[memoryBufferSize];
    reader->getItemDataWithDecoderParameters(itemId, memoryBuffer, memoryBufferSize);

    // Feed 'data' to decoder and display the cover image...

    HeifReaderInterface::Destroy(reader);
}

/// Access and read image item and its thumbnail
void example2()
{
    Array<ImageId> itemIds;
    uint32_t itemSize = 1024 * 1024;
    char* itemData    = new char[itemSize];

    auto* reader = HeifReaderInterface::Create();

    reader->initialize("test_001.heic");
    FileInformation info;
    if (reader->getFileInformation(info) != ErrorCode::OK)
    {
        // handle error here...
        return;
    }

    // Verify that the file has one or several images in the MetaBox
    if (!(info.features & FileFeatureEnum::HasSingleImage ||
          info.features & FileFeatureEnum::HasImageCollection))
    {
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

    HeifReaderInterface::Destroy(reader);
    delete[] itemData;
}

/// Access and read image items and their thumbnails in a collection
void example3()
{
    auto* reader = HeifReaderInterface::Create();
    typedef ImageId MasterItemId;
    typedef ImageId ThumbnailItemId;
    Array<ImageId> itemIds;
    map<MasterItemId, ThumbnailItemId> imageMap;

    reader->initialize("test_002.heic");
    FileInformation info;
    reader->getFileInformation(info);

    // Find item IDs of master images
    reader->getMasterImages(itemIds);

    // Find thumbnails for each master. There can be several thumbnails for one master image, get narrowest ones here.
    for (const auto masterId : itemIds)
    {
        // Thumbnail references ('thmb') are from the thumbnail image to the master image
        Array<ImageId> thumbIds;
        reader->getReferencedToItemListByType(masterId, "thmb", thumbIds);

        const ImageId thumbId = *std::min_element(thumbIds.begin(), thumbIds.end(),
                                                  [&](ImageId a, ImageId b) {
                                                      uint32_t widthA, widthB;
                                                      reader->getWidth(a, widthA);
                                                      reader->getWidth(b, widthB);
                                                      return (widthA < widthB);
                                                  });
        imageMap[masterId]    = thumbId;
    }

    // We have now item IDs of thumbnails and master images. Decode and show them from imageMap as wanted.
    HeifReaderInterface::Destroy(reader);
}

/// Access and read derived images
void example4()
{
    auto* reader = HeifReaderInterface::Create();
    Array<ImageId> itemIds;

    reader->initialize("test_003.heic");
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

    cout << "To render derived image item ID " << itemId << ":" << endl;
    cout << "-retrieve data for source image item ID " << sourceItemId << endl;
    cout << "-rotating image " << rotation << " degrees." << endl;

    HeifReaderInterface::Destroy(reader);
}

/// Access and read media track samples, thumbnail track samples and timestamps
void example5()
{
    auto* reader = HeifReaderInterface::Create();
    Array<uint32_t> itemIds;

    reader->initialize("test_011.heic");
    FileInformation info;
    reader->getFileInformation(info);

    // Print information for every track read
    for (const auto& trackProperties : info.trackInformation)
    {
        const auto sequenceId = trackProperties.trackId;
        cout << "Track ID " << sequenceId << endl;  // Context ID corresponds to the track ID

        if (trackProperties.features & TrackFeatureEnum::IsMasterImageSequence)
        {
            cout << "This is a master image sequence." << endl;
        }

        if (trackProperties.features & TrackFeatureEnum::IsThumbnailImageSequence)
        {
            // Assume there is only one type track reference, so check reference type and master track ID(s) from the first one.
            const auto tref = trackProperties.referenceTrackIds[0];
            cout << "Track reference type is '" << tref.type.value << "'" << endl;
            cout << "This is a thumbnail track for track ID ";
            for (const auto masterTrackId : tref.trackIds)
            {
                cout << masterTrackId << endl;
            }
        }

        Array<TimestampIDPair> timestamps;
        reader->getItemTimestamps(sequenceId, timestamps);
        cout << "Sample timestamps:" << endl;
        for (const auto& timestamp : timestamps)
        {
            cout << " Timestamp=" << timestamp.timeStamp << "ms, sample ID=" << timestamp.itemId << endl;
        }

        for (const auto& sampleProperties : trackProperties.sampleProperties)
        {
            // A sample might have decoding dependencies. The simplest way to handle this is just to always ask and
            // decode all dependencies.
            Array<SequenceImageId> itemsToDecode;
            reader->getDecodeDependencies(sequenceId, sampleProperties.sampleId, itemsToDecode);
            for (auto dependencyId : itemsToDecode)
            {
                uint32_t size    = 1024 * 1024;
                char* sampleData = new char[size];
                reader->getItemDataWithDecoderParameters(sequenceId, dependencyId, sampleData, size);
                // Feed data to decoder...
            }
            // Store or show the image...
        }
    }
    HeifReaderInterface::Destroy(reader);
}

/// Access and read media alternative
void example6()
{
    auto* reader = HeifReaderInterface::Create();
    Array<uint32_t> itemIds;

    reader->initialize("test_030_2.heic");
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
    HeifReaderInterface::Destroy(reader);
}


int main()
{
    example1();
    example2();
    example3();
    example4();
    example5();
    example6();
}
