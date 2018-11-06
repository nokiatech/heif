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

#ifndef HEIFFILEDATATYPESINTERNAL_HPP
#define HEIFFILEDATATYPESINTERNAL_HPP

#include <cstdint>
#include <set>
#include "customallocator.hpp"
#include "heifreaderdatatypes.h"


namespace HEIF
{
    class FileFeature
    {
    public:
        typedef Set<FileFeatureEnum::Feature> FileFeatureSet;

        bool hasFeature(FileFeatureEnum::Feature feature) const
        {
            return (mFileFeatureSet.count(feature) != 0);
        }
        void setFeature(FileFeatureEnum::Feature feature)
        {
            mFileFeatureSet.insert(feature);
        }
        uint32_t getFeatureMask() const
        {
            uint32_t mask = 0;
            for (auto set : mFileFeatureSet)
            {
                mask |= (uint32_t) set;
            }
            return mask;
        }

    private:
        FileFeatureSet mFileFeatureSet;
    };

    class TrackFeature
    {
    public:
        typedef Set<TrackFeatureEnum::Feature> TrackFeatureSet;

        bool hasFeature(TrackFeatureEnum::Feature feature) const
        {
            return (mTrackFeatureSet.count(feature) != 0);
        }
        void setFeature(TrackFeatureEnum::Feature feature)
        {
            mTrackFeatureSet.insert(feature);
        }
        uint32_t getFeatureMask() const
        {
            uint32_t mask = 0;
            for (auto set : mTrackFeatureSet)
            {
                mask |= (uint32_t) set;
            }
            return mask;
        }

    private:
        TrackFeatureSet mTrackFeatureSet;
    };

    /** @brief Represents the features of entities in Meta Box at file, moov or track level. */
    class MetaBoxFeature
    {
    public:
        typedef Set<MetaBoxFeatureEnum::Feature> MetaBoxFeatureSet;

        bool hasFeature(MetaBoxFeatureEnum::Feature feature) const
        {
            return (mMetaBoxFeatureSet.count(feature) != 0);
        }
        void setFeature(MetaBoxFeatureEnum::Feature feature)
        {
            mMetaBoxFeatureSet.insert(feature);
        }
        uint32_t getFeatureMask() const
        {
            uint32_t mask = 0;
            for (auto set : mMetaBoxFeatureSet)
            {
                mask |= (uint32_t) set;
            }
            return mask;
        }

    private:
        MetaBoxFeatureSet mMetaBoxFeatureSet;
    };

    /** @brief Each item in a MetaBox has different features */
    class ItemFeature
    {
    public:
        typedef Set<ItemFeatureEnum::Feature> ItemFeatureSet;

        bool hasFeature(ItemFeatureEnum::Feature feature) const
        {
            return (mItemFeatureSet.count(feature) != 0);
        }
        void setFeature(ItemFeatureEnum::Feature feature)
        {
            mItemFeatureSet.insert(feature);
        }
        uint32_t getFeatureMask() const
        {
            uint32_t mask = 0;
            for (auto set : mItemFeatureSet)
            {
                mask |= (uint32_t) set;
            }
            return mask;
        }

    private:
        ItemFeatureSet mItemFeatureSet;
    };

    // Forward declarations
    struct TrackProperties;
    struct SampleProperties;


    // Convenience types
    typedef std::int64_t Timestamp;
    typedef std::uint32_t ItemId;
    typedef Vector<std::pair<ItemId, Timestamp>> DecodingOrderVector;
    typedef Vector<std::uint32_t> IdVector;
    typedef Vector<std::uint8_t> DataVector;
    typedef Map<FourCC, IdVector> TypeToIdsMap;
    typedef Vector<EntityGrouping> Groupings;
    typedef Map<DecoderSpecInfoType, DataVector> ParameterSetMap;
    typedef Vector<ItemPropertyInfo> PropertyTypeVector;


    // In the type definitions, the first element of each map represents the file-given ID of the relevant data
    // structure. In he below mentioned type definitions, key value is the ID of the entity.
    typedef Map<ImageId, ItemFeature> ItemFeaturesMap;
    typedef Map<SequenceId, TrackProperties> TrackPropertiesMap;  ///< <track_id/context_id, TrackProperties>
    typedef Map<std::uint32_t, SampleProperties> SamplePropertiesMap;

    /** @brief MetaBox Property definition that contains image and item features */
    struct MetaBoxProperties
    {
        std::uint32_t contextId;
        MetaBoxFeature metaBoxFeature;
        ItemFeaturesMap itemFeaturesMap;
        Groupings entityGroupings;
    };

    /** @brief Moov Property definition that may contain a MetaBox */
    struct MoovProperties
    {
        std::uint32_t moovId;
        MetaBoxProperties metaBoxProperties;
    };

    /** @brief Sample Property definition */
    struct SampleProperties
    {
        std::uint32_t sampleId;                ///< based on the sample's entry order in the sample table
        FourCC sampleEntryType;                ///< coming from SampleDescriptionBox (codingname)
        std::uint32_t sampleDescriptionIndex;  ///< coming from SampleDescriptionBox index (sample_description_index)
        SampleType sampleType;                 ///< coming from sample groupings
        uint64_t sampleDurationTS;             ///< duration of sample in timescale
        int64_t sampleCompositionOffsetTs;
        bool hasClap;  ///< CleanApertureBox is present in the sample entry
        bool hasAuxi;  ///< AuxiliaryTypeInfo box is present in the sample entry
        CodingConstraints codingConstraints;
        uint64_t size; ///< Size of sample data in bytes
    };

    /** @brief Track Property definition which contain sample properties.
     *
     * In the samplePropertiesMap, samples of the track are listed in the same order they appear
     * in the sample size or sample to chunk boxes.
     * Each sample is given an ID, which is used as the key of the map.
     */
    struct TrackProperties
    {
        SequenceId trackId;  ///< trackId is also context id
        std::uint32_t alternateGroupId;
        TrackFeature trackFeature;
        SamplePropertiesMap sampleProperties;
        IdVector alternateTrackIds;            ///< other tracks IDs with the same alternate_group id.
        TypeToIdsMap referenceTrackIds;        ///< <reference_type, reference track ID> (coming from 'tref')
        Array<SampleGrouping> groupedSamples;  ///< Sample groupings of the track.
        Array<SampleVisualEquivalence>
            equivalences;                       ///< Information from VisualEquivalenceEntry() 'eqiv' sample groups.
        Array<SampleToMetadataItem> metadatas;  ///< Data from SampleToMetadataItemEntry ('stmi') sample group entries
        Array<DirectReferenceSamples>
            referenceSamples;    ///< Data of DirectReferenceSamplesList ('refs') sample group entries
        uint64_t maxSampleSize;  ///< Size of largest sample inside the track (can be used to allocate client side read
                                 ///< buffer).
        uint32_t timeScale;
        EditList editList;  ///< Editlist for this track.
    };

    /** @brief Overall File Property definition which contains file's properties.*/
    struct FileInformationInternal
    {
        FileFeature fileFeature;
        TrackPropertiesMap trackProperties;
        MetaBoxProperties rootLevelMetaBoxProperties;
        uint32_t movieTimescale;
    };


    typedef std::uint32_t ContextId;                           ///< Context (= meta box and track) identifiers
    typedef std::uint32_t ItemId;                              ///< Sample/image/item identifiers
    typedef std::pair<ContextId, ItemId> Id;                   ///< Convenience type combining context and item IDs
    typedef std::pair<ItemId, Timestamp> ItemIdTimestampPair;  ///< Pair of Item/sample ID and timestamp

}  // namespace HEIF

#endif /* HEIFFILEDATATYPESINTERNAL_HPP */
