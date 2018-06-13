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

#ifndef WRITERDATATYPESINTERNAL_HPP
#define WRITERDATATYPESINTERNAL_HPP

#include "customallocator.hpp"
#include "fourccint.hpp"
#include "heifwriterdatatypes.h"

namespace HEIF
{
    IdType(std::uint32_t, TrackId);
    IdType(std::uint16_t, AlternateGroupId);

    /// Data of one image/sample/frame
    struct MediaData
    {
        MediaDataId id;
        MediaFormat mediaFormat;
        DecoderConfigId decoderConfigId;
        uint64_t offset;  ///< Data offset. When mdat is after ftyp this is fileoffset. When mdat is lcoated after moov
                          ///< this is offset from mdat start.
        size_t size;
    };

    struct Dimensions
    {
        uint32_t width;
        uint32_t height;
    };

    struct ImageSequence
    {
        SequenceId id;
        TrackId trackId;
        FourCCInt handlerType;
        MediaFormat mediaFormat;

        Rational timeBase;
        Dimensions maxDimensions;
        uint64_t duration;
        uint32_t creationTime;
        uint32_t modificationTime;

        bool trackEnabled;
        bool trackInMovie;
        bool trackPreview;

        bool containsHidden;
        bool containsReferenceSamples;
        bool containsEquivalenceGroupSamples;
        bool containsCleanApertureBox;
        AlternateGroupId alternateGroup;
        Map<FourCCInt, Set<TrackId>> trackReferences;
        String auxiliaryType;

        struct Sample
        {
            uint32_t sampleIndex;
            MediaDataId mediaDataId;  // Reference to the sample data and decoder configurations.
            SequenceImageId sequenceImageId;
            uint64_t dts;
            uint32_t sampleDuration;
            int64_t compositionOffset;
            uint32_t decoderConfigIndex;
            bool isSyncSample;
            bool isHidden;
            Vector<std::uint32_t> referenceSamples;
            Map<GroupId, EquivalenceTimeOffset> equivalenceGroups;
            Set<MetadataItemId> metadataItemsIds;
        };
        Vector<Sample> samples;
        Vector<DecoderConfigId> decoderConfigs;
        bool anyNonSyncSample;
        CodingConstraints codingConstraints;  // for image sequences.
        AudioParams audioParams;              // for audio tracks.
        CleanAperture clap;
        EditList editList;
        Vector<int32_t> matrix;
    };

    struct PropertyInformation
    {
        bool isTransformative;
    };

    struct ImageCollection
    {
        struct PropertyAssociation
        {
            PropertyId propertyId;
            bool essential;
        };

        struct Image
        {
            ImageId imageId;
            bool isHidden = false;
            Vector<PropertyAssociation> descriptiveProperties;
            Vector<PropertyAssociation> transformativeProperties;
        };

        Map<ImageId, Image> images;
    };

    struct EntityGroup
    {
        FourCC type;
        GroupId id;

        struct Entity
        {
            uint32_t id;
            enum class Type
            {
                ITEM,
                SEQUENCE
            };
            Type type;  /// < Id can be either an item or a track id. This type field is only for debugging purposes.
        };
        Vector<Entity> entities;
    };

    typedef uint16_t PropertyIndex;  ///< Propery index in the ipco box.

    /// Image size for handling 'ispe' properties.
    struct ImageSize
    {
        uint32_t width;
        uint32_t height;
        bool operator<(const ImageSize& rhs) const
        {
            return std::tie(width, height) < std::tie(rhs.width, rhs.height);
        }
    };
}  // namespace HEIF

#endif  // WRITERDATATYPESINTERNAL_HPP
