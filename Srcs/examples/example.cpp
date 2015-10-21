/* Copyright (c) 2015, Nokia Technologies Ltd.
 * All rights reserved.
 *
 * Licensed under the Nokia High-Efficiency Image File Format (HEIF) License (the "License").
 *
 * You may not use the High-Efficiency Image File Format except in compliance with the License.
 * The License accompanies the software and can be found in the file "LICENSE.TXT".
 *
 * You may also obtain the License at:
 * https://nokiatech.github.io/heif/license.txt
 */

/** This file includes some examples about using the reader API
 *  Note that binary execution fails if proper .heic files are not located in the directory */

#include "hevcimagefilereader.hpp"
#include <iostream>

using namespace std;

/// Access and read a cover image
void example1()
{
    HevcImageFileReader reader;
    reader.initialize("test_001.heic");
    const auto& properties = reader.getFileProperties();

    // Verify that the file has a cover image
    if (not properties.fileFeature.hasFeature(ImageFileReaderInterface::FileFeature::HasCoverImage))
    {
        return; // Nothing to do, just return.
    }

    // The cover image is always located in the root level MetaBox, so get MetaBox context ID.
    const uint32_t contextId = properties.rootLevelMetaBoxProperties.contextId;

    // Find the item ID
    const uint32_t itemId = reader.getCoverImageItemId(contextId);

    ImageFileReaderInterface::DataVector data;
    reader.getItemDataWithDecoderParameters(contextId, itemId, data);

    // Feed 'data' to decoder and display the cover image...
}

/// Access and read image item and its thumbnail
void example2()
{
    HevcImageFileReader reader;
    ImageFileReaderInterface::DataVector data;
    ImageFileReaderInterface::IdVector itemIds;

    reader.initialize("test_001.heic");
    const auto& properties = reader.getFileProperties();

    // Verify that the file has one or several images in the MetaBox
    if (not (properties.fileFeature.hasFeature(ImageFileReaderInterface::FileFeature::HasSingleImage) ||
        properties.fileFeature.hasFeature(ImageFileReaderInterface::FileFeature::HasImageCollection)))
    {
        return;
    }

    // Find the item ID of the first master image
    const uint32_t contextId = properties.rootLevelMetaBoxProperties.contextId;
    reader.getItemListByType(contextId, "master", itemIds);
    const uint32_t masterId = itemIds.at(0);

    const auto& metaBoxFeatures = properties.rootLevelMetaBoxProperties.metaBoxFeature; // For convenience
    if (metaBoxFeatures.hasFeature(ImageFileReaderInterface::MetaBoxFeature::HasThumbnails))
    {
        // Thumbnail references ('thmb') are from the thumbnail image to the master image
        reader.getReferencedToItemListByType(contextId, masterId, "thmb", itemIds);
        const uint32_t thumbnailId = itemIds.at(0);

        reader.getItemDataWithDecoderParameters(contextId, thumbnailId, data);
        // ...decode data and display the image, show master image later
    }
    else
    {
        // There was no thumbnail, show just the master image
        reader.getItemDataWithDecoderParameters(contextId, masterId, data);
        // ...decode and display...
    }
}

/// Access and read image items and their thumbnails in a collection
void example3()
{
    HevcImageFileReader reader;
    ImageFileReaderInterface::DataVector data;
    typedef uint32_t MasterItemId;
    typedef uint32_t ThumbnailItemId;
    ImageFileReaderInterface::IdVector itemIds;
    map<MasterItemId, ThumbnailItemId> imageMap;

    reader.initialize("test_002.heic");
    const auto& properties = reader.getFileProperties();

    // Find item IDs of master images
    const uint32_t contextId = properties.rootLevelMetaBoxProperties.contextId;
    reader.getItemListByType(contextId, "master", itemIds);

    // Find thumbnails for each master. There can be several thumbnails for one master image, get narrowest ones here.
    for (const auto masterId : itemIds)
    {
        // Thumbnail references ('thmb') are from the thumbnail image to the master image
        ImageFileReaderInterface::IdVector thumbIds;
        reader.getReferencedToItemListByType(contextId, masterId, "thmb", thumbIds);

        const int thumbId = *min_element(thumbIds.cbegin(), thumbIds.cend(),
            [&](uint32_t a, uint32_t b){ return (reader.getWidth(contextId, a) < reader.getWidth(contextId, b)); } );
        imageMap[masterId] = thumbId;
    }

    // We have now item IDs of thumbnails and master images. Decode and show them from imageMap as wanted.
}

/// Access and read derived images
void example4()
{
    HevcImageFileReader reader;
    ImageFileReaderInterface::IdVector itemIds;

    reader.initialize("test_003.heic");
    const auto& properties = reader.getFileProperties();
    const uint32_t contextId = properties.rootLevelMetaBoxProperties.contextId;

    // Find item IDs of 'iden' (identity transformation) type derived images
    reader.getItemListByType(contextId, "iden", itemIds);

    const uint32_t itemId = itemIds.at(0); // For demo purposes, assume there was one 'iden' item

    // 'dimg' item reference points from the 'iden' derived item to the input(s) of the derivation
    reader.getReferencedFromItemListByType(contextId, itemId, "dimg", itemIds);
    const uint32_t sourceItemId = itemIds.at(0); // For demo purposes, assume there was one

    // Get 'iden' item properties to find out what kind of derivation it is
    const auto itemProperties = reader.getItemProperties(contextId, itemId);

    unsigned int rotation = 0;
    for (const auto& property : itemProperties)
    {
        // For example, handle 'irot' transformational property is anti-clockwise rotation
        if (property.type == ImageFileReaderInterface::ItemPropertyType::IROT)
        {
            // Get property struct by index to access rotation angle
            rotation = reader.getPropertyIrot(contextId, property.index).rotation;
            break; // Assume only one property
        }
    }

    cout << "To render derived image item ID " << itemId << ":" << endl;
    cout << "-retrieve data for source image item ID " << sourceItemId << endl;
    cout << "-rotating image " << rotation << " degrees." << endl;
}

/// Access and read media track samples, thumbnail track samples and timestamps
void example5()
{
    HevcImageFileReader reader;
    ImageFileReaderInterface::IdVector itemIds;

    reader.initialize("test_011.heic");
    const auto& properties = reader.getFileProperties();

    // Print information for every track read
    for (const auto& trackProperties : properties.trackProperties)
    {
         const uint32_t contextId = trackProperties.first;
         cout << "Track ID " << contextId << endl; // Context ID corresponds to the track ID

         if (trackProperties.second.trackFeature.hasFeature(ImageFileReaderInterface::TrackFeature::IsMasterImageSequence))
         {
             cout << "This is a master image sequence." << endl;
         }

         if (trackProperties.second.trackFeature.hasFeature(ImageFileReaderInterface::TrackFeature::IsThumbnailImageSequence))
         {
             // Assume there is only one type track reference, so check reference type and master track ID(s) from the first one.
             const auto tref = trackProperties.second.referenceTrackIds.cbegin();
             cout << "Track reference type is '"<< tref->first << "'" << endl;
             cout << "This is a thumbnail track for track ID ";
             for (const auto masterTrackId : tref->second)
             {
                 cout << masterTrackId << endl;
             }
         }

         ImageFileReaderInterface::TimestampMap timestamps;
         reader.getItemTimestamps(contextId, timestamps);
         cout << "Sample timestamps:" << endl;
         for (const auto& timestamp : timestamps)
         {
             cout << " Timestamp=" << timestamp.first << "ms, sample ID=" << timestamp.second << endl;
         }

         for (const auto& sampleProperties : trackProperties.second.sampleProperties)
         {
             ImageFileReaderInterface::IdVector itemsToDecode;
             const uint32_t sampleId = sampleProperties.first;

             // A sample might have decoding dependencies. The simples way to handle this is just to always ask and
             // decode all dependencies.
             reader.getItemDecodeDependencies(contextId, sampleId, itemsToDecode);
             for (auto dependencyId : itemsToDecode)
             {
                 ImageFileReaderInterface::DataVector data;
                 reader.getItemDataWithDecoderParameters(contextId, dependencyId, data);
                 // Feed data to decoder...
             }
             // Store or show the image...
         }
     }
}

/// Access and read media alternative
void example6()
{
    HevcImageFileReader reader;
    ImageFileReaderInterface::IdVector itemIds;

    reader.initialize("test_030_2.heic");
    const auto& properties = reader.getFileProperties();

    uint32_t trackId = 0;

    if (properties.trackProperties.size() > 0)
    {
        auto iter = properties.trackProperties.cbegin(); // Get first image sequence track
        trackId = iter->first;
        const HevcImageFileReader::TrackProperties& trackProperties = iter->second;

        if (trackProperties.trackFeature.hasFeature(HevcImageFileReader::TrackFeature::HasAlternatives))
        {
            const uint32_t alternativeTrackId = trackProperties.alternateTrackIds.at(0); // Take the first alternative
            const auto alternativeWidth = reader.getDisplayWidth(alternativeTrackId);
            const auto trackWidth = reader.getDisplayWidth(trackId);

            if (trackWidth > alternativeWidth)
            {
                cout << "The alternative track has wider display width, let's use it from now on..." << endl;
                trackId = alternativeTrackId;
            }
        }
    }
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

