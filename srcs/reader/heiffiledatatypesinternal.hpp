/* This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2021 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
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
#include "decodepts.hpp"
#include "heifid.h"
#include "heifreaderdatatypes.h"
#include "heifstreaminternal.hpp"
#include "moviefragmentsdatatypes.hpp"
#include "segmenttypebox.hpp"
#include "tracktypebox.hpp"


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
                mask |= static_cast<uint32_t>(set);
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
                mask |= static_cast<uint32_t>(set);
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
                mask |= static_cast<uint32_t>(set);
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
                mask |= static_cast<uint32_t>(set);
            }
            return mask;
        }

    private:
        ItemFeatureSet mItemFeatureSet;
    };

    /** @brief 'MOOV' level features flag enumeration. */
    class MoovFeature
    {
    public:
        /// Enumerated list of Moov Features
        enum Feature
        {
            HasMoovLevelMetaBox,
            HasCoverImage
        };
        typedef Set<Feature> MoovFeatureSet;

        bool hasFeature(Feature feature) const
        {
            return mMoovFeatureSet.count(feature) != 0;
        }
        void setFeature(Feature feature)
        {
            mMoovFeatureSet.insert(feature);
        }

    private:
        MoovFeatureSet mMoovFeatureSet;
    };

    // Forward declarations
    struct SampleProperties;
    struct InitTrackInfo;

    struct SampleSizeInPixels
    {
        uint32_t width;
        uint32_t height;
    };

    // Convenience types
    typedef std::int64_t Timestamp;
    typedef Vector<TimestampIDPair> DecodingOrderVector;
    typedef Vector<std::uint8_t> DataVector;
    typedef Map<FourCC, Vector<SequenceId>> TypeToIdsMap;
    typedef Vector<EntityGrouping> Groupings;
    typedef Map<DecoderSpecInfoType, DataVector> ParameterSetMap;
    typedef Vector<ItemPropertyInfo> PropertyTypeVector;

    IdType(std::uint32_t, Sequence);
    IdType(std::uint32_t, SampleDescriptionIndex);

    typedef std::pair<SegmentId, SequenceId> SegmentTrackId;
    typedef std::pair<SequenceId, SampleDescriptionIndex> TrackSampleEntryIndex;

    typedef std::pair<SequenceId, SequenceImageId>
        SequenceImageIdPair;  ///< Unique identifier for an sample/sequence image in a sequence

    // In the type definitions, the first element of each map represents the file-given ID of the relevant data
    // structure. In he below mentioned type definitions, key value is the ID of the entity.
    typedef Map<ImageId, ItemFeature> ItemFeaturesMap;
    typedef Map<std::uint32_t, SampleProperties> SamplePropertiesMap;
    typedef Map<SequenceId, InitTrackInfo> InitTrackInfoMap;

    typedef std::pair<SequenceImageId, Timestamp> ItemIdTimestampPair;  ///< Pair of Item/sample ID and timestamp

    typedef Map<SequenceImageIdPair, SampleDescriptionIndex> SampleToParameterSetMap;
    typedef Array<SegmentInformation> SegmentIndex;


    /** @brief MetaBox Property definition that contains image and item features */
    struct MetaBoxProperties
    {
        MetaBoxFeature metaBoxFeature;
        ItemFeaturesMap itemFeaturesMap;
        Groupings entityGroupings;
    };

    /** @brief Moov Property definition */
    struct MoovProperties
    {
        MoovFeature moovFeature;
        uint32_t movieTimescale;
        Vector<std::int32_t> mMatrix;  ///< Video transformation matrix from the Movie Header Box

        // movie fragment support related properties, used internally only:
        uint64_t fragmentDuration;
        Vector<MOVIEFRAGMENTS::SampleDefaults> fragmentSampleDefaults;
    };

    struct TrackGroupInfo
    {
        Vector<SequenceId> ids;
    };
    using TrackGroupInfoMap = Map<FourCCInt, TrackGroupInfo>;

    /** @brief Sample Property definition */
    struct SampleProperties
    {
        SequenceImageId sampleId =
            0;                    ///< based on the sample's entry order in the sample table; this is in decoding order
        SegmentId segmentId = 0;  ///< Segment Id of the sample.

        FourCCInt sampleEntryType = 0;  ///< coming from SampleDescriptionBox (codingname)
        SampleType sampleType;          ///< coming from sample groupings
        SampleDescriptionIndex sampleDescriptionIndex =
            0;  ///< coming from SampleDescriptionBox index (sample_description_index)

        CodingConstraints codingConstraints;

        std::uint32_t sampleDurationTS         = 0;  ///< Duration of sample in time scale units.
        std::int64_t sampleCompositionOffsetTs = 0;
        Vector<std::int64_t> compositionTimes;     ///< Timestamps of the sample. Possible edit list is considered here.
        Vector<std::uint64_t> compositionTimesTS;  ///< Timestamps of the sample in time scale units. Possible
                                                   ///< edit list is considered here.
        std::uint64_t dataOffset = 0;              ///< File offset of sample data in bytes
        std::uint32_t dataLength = 0;              ///< Length of sample in bytes
        std::uint32_t width      = 0;              ///< Width of the frame
        std::uint32_t height     = 0;              ///< Height of the frame
        SampleFlags sampleFlags;  ///< Sample Flags Field as defined in 8.8.3.1 of ISO/IEC 14496-12:2015(E)
        Vector<SequenceImageId> decodeDependencies;  ///< Direct decoding dependencies

        bool hasClap = false;  ///< CleanApertureBox is present in the sample entry
        bool hasAuxi = false;  ///< AuxiliaryTypeInfo box is present in the sample entry
    };

    /** @brief Track Property definition which contain sample properties.
     *
     * Information about a track, extracted from the initialization segment.
     */
    struct InitTrackInfo
    {
        SequenceId trackId;
        Array<SampleGrouping> groupedSamples;  ///< Sample groupings of the track.
        Array<SampleVisualEquivalence>
            equivalences;                       ///< Information from VisualEquivalenceEntry() 'eqiv' sample groups.
        Array<SampleToMetadataItem> metadatas;  ///< Data from SampleToMetadataItemEntry ('stmi') sample group entries
        Array<DirectReferenceSamples>
            referenceSamples;    ///< Data of DirectReferenceSamplesList ('refs') sample group entries
        uint64_t maxSampleSize;  ///< Size of largest sample inside the track (can be used to allocate client side read
                                 ///< buffer).
        uint32_t timeScale;

        std::uint32_t alternateGroupId;
        TrackFeature trackFeature;
        Vector<SequenceId> alternateTrackIds;  ///< other tracks IDs with the same alternate_group id.
        TypeToIdsMap referenceTrackIds;        ///< <reference_type, reference track ID> (coming from 'tref')
        TrackGroupInfoMap trackGroupInfoMap;  ///< <group_type, track group info> ... coming from Track Group Box 'trgr'
        EditList editList;                    ///< Editlist for this track.
        std::shared_ptr<const EditBox> editBox;  ///< If set, an edit list box exists

        std::uint32_t width;   ///< display width in pixels, from 16.16 fixed point in TrackHeaderBox
        std::uint32_t height;  ///< display height in pixels, from 16.16 fixed point in TrackHeaderBox

        FourCCInt sampleEntryType;  /// sample type from this track. Passed to segments sampleproperties.

        Map<SampleDescriptionIndex, ParameterSetMap> parameterSetMaps;  ///< Extracted decoder parameter sets
        Map<SampleDescriptionIndex, SampleSizeInPixels>
            sampleSizeInPixels;  ///< Clean sample size information from sample description entries
        Map<SampleDescriptionIndex, std::uint8_t> nalLengthSizeMinus1;

        Vector<int32_t> matrix;  ///< transformation matrix of the track (from track header box)

        Map<SampleDescriptionIndex, CleanAperture>
            clapProperties;  ///< Clean aperture data from sample description entries
        Map<SampleDescriptionIndex, AuxiliaryType>
            auxiProperties;  ///< Clean aperture data from sample description entries
    };

    typedef Vector<SampleProperties> SamplePropertyVector;

    /// Information about samples of a track in a segment.
    struct TrackInfoInSegment
    {
        SequenceImageId itemIdBase;
        SamplePropertyVector samples;  ///< Information about each sample in the TrackBox

        DecodePts::PresentationTimeTS durationTS    = 0;  ///< Track duration in time scale units, from TrackHeaderBox
        DecodePts::PresentationTimeTS earliestPTSTS = 0;  ///< Time of the first sample in time scale units
        DecodePts::PresentationTimeTS noSidxFallbackPTSTS = 0;  ///< Start PTS for next segment if no sidx

        /** Time to use as the base when adding the next track run; this value is initialized with
        some derived value upon first use and the incremented by trackrun duration when one is read. */
        DecodePts::PresentationTimeTS nextPTSTS = 0;

        WriteOnceMap<SequenceImageId, FourCCInt> decoderCodeTypeMap;  ///< Extracted decoder code types

        DecodePts::PMap pMap;      ///< Display timestamps, from edit list
        DecodePts::PMapTS pMapTS;  ///< Display timestamps in time scale units, from edit list

        /// @todo Move to another structs.
        bool hasEditList = false;  ///< Used to determine if updateCompositionTimes should edit the time of last sample
                                   ///< to match track duration
        bool hasTtyp = false;
        TrackTypeBox ttyp;  ///< TrackType info in case if available
        double duration;    ///< Track duration in seconds, from TrackHeaderBox
        double repetitions;
    };

    struct StreamIO
    {
        UniquePtr<InternalStream> stream;
        UniquePtr<StreamInterface> fileStream;  // only used when reading from files
        std::int64_t size = 0;
    };

    struct SegmentProperties
    {
        SegmentId segmentId;
        Set<Sequence> sequences;  ///< Generated sequence number for this segment instead of sequence numbers from the
                                  ///< movie fragment header box.

        StreamIO io;

        SegmentTypeBox styp;  ///< Segment Type Box for later information retrieval

        Map<SequenceId, TrackInfoInSegment> trackInfos;

        SampleToParameterSetMap sampleToParameterSetMap;  ///< Map from every sample to parameter set map entry
    };

    typedef Map<SegmentId, SegmentProperties> SegmentPropertiesMap;
    typedef Map<Sequence, SegmentId> SequenceToSegmentMap;

    /** @brief Overall File Property definition which contains file's properties.*/
    struct FileInformationInternal
    {
        FileFeature fileFeature;
        MetaBoxProperties rootLevelMetaBoxProperties;

        MoovProperties moovProperties;

        Map<SequenceId, InitTrackInfo> initTrackInfos;

        SegmentIndex segmentIndex;
        SegmentPropertiesMap segmentPropertiesMap;
        SequenceToSegmentMap sequenceToSegment;
    };
}  // namespace HEIF

#endif /* HEIFFILEDATATYPESINTERNAL_HPP */
