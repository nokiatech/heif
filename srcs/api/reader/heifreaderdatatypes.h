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

#ifndef HEIFFILEDATATYPES_H
#define HEIFFILEDATATYPES_H

#include <cstddef>
#include <cstdint>
#include "heifcommondatatypes.h"
#include "heifexport.h"

#ifndef _SCL_SECURE_NO_WARNINGS
#define _SCL_SECURE_NO_WARNINGS
#endif


namespace HEIF
{
    enum TrackSampleType
    {
        out_ref,      ///< output reference frames
        out_non_ref,  ///< output non-reference frames
        non_out_ref,  ///< non-output reference frame
        display,      ///< all frame samples in the track which are displayed and in display order
        samples,      ///< all samples in the track in track's entry order
    };

    // Item property related data types

    /// Item Property type identifiers
    enum class ItemPropertyType
    {
        INVALID = 0,  ///< Can be used for error checking. Not a valid property type.

        RAW,   ///< Property of an unrecognized/unknown type. It is accessible only as raw data.
        AUXC,  ///< Image properties for auxiliary images
        AVCC,  ///< AVC configuration
        CLAP,  ///< Clean aperture (crop)
        COLR,  ///< Colour information
        HVCC,  ///< HEVC configuration
        IMIR,  ///< Image mirror
        IROT,  ///< Image rotation
        ISPE,  ///< Image spatial extents
        JPGC,  ///< JPEG configuration
        PASP,  ///< Pixel aspect ratio
        PIXI,  ///< Pixel information
        RLOC   ///< Relative location
    };

    /// Information about a single property associated to an image item
    struct HEIF_DLL_PUBLIC ItemPropertyInfo
    {
        ItemPropertyType type;  ///< Type of the property.
        PropertyId index;       ///< Id(/index) of the property.
        bool essential;         ///< True if the property is defined as essential for the item.
    };

    struct HEIF_DLL_PUBLIC TimestampIDPair
    {
        int64_t timeStamp;
        SequenceImageId itemId;
    };

    namespace FileFeatureEnum
    {
        enum Feature
        {
            HasSingleImage      = 1u,       ///< File copntains only 1 master image
            HasImageCollection  = 1u << 1,  ///< File contains Image Collection (more than 1 master images)
            HasImageSequence    = 1u << 2,  ///< File contains Image Sequence
            HasRootLevelMetaBox = 1u << 3,  ///< File has root level metabox
            HasAlternateTracks  = 1u << 4   ///< The file contains a alternate tracks.
        };
    }

    namespace MetaBoxFeatureEnum
    {
        enum Feature
        {
            IsSingleImage               = 1u,       ///< Metabox contains only 1 image
            IsImageCollection           = 1u << 1,  ///< Metabox contains image collection (more than 1 image)
            HasMasterImages             = 1u << 2,  ///< Metabox contains at least 1 master image
            HasThumbnails               = 1u << 3,  ///< Metabox contains at least 1 thumbnail image
            HasAuxiliaryImages          = 1u << 4,  ///< Metabox contains at least 1 auxiliary image
            HasDerivedImages            = 1u << 5,  ///< Metabox contains at least 1 derived image
            HasPreComputedDerivedImages = 1u << 6,  ///< Metabox contains at least 1 pre-derived coded image
            HasHiddenImages             = 1u << 7,  ///< Metabox contains at least 1 hidden image
            HasGroupLists               = 1u << 8   ///< Metabox contains at least 1 grouping information
        };
    }

    namespace ItemFeatureEnum
    {
        enum Feature
        {
            IsMasterImage    = 1u,       ///< Image is master image e.g. not a thumbnail or an auxiliary image
            IsThumbnailImage = 1u << 1,  ///< Image is thumbnail image. It has 'thmb' reference to other image
            IsAuxiliaryImage = 1u << 2,  ///< Image is auxiliary image. It has 'auxl' reference to other image
            IsPrimaryImage =
                1u << 3,  ///< Image is an primary image. The primary item should be displayed when no other information
                          ///< is available on the preferred displaying method of the image collection.
            IsDerivedImage            = 1u << 4,  ///< Image is a derived from other image.
            IsPreComputedDerivedImage = 1u << 5,  ///< Image is a Pre-derived coded image. for example, a composite HDR
                                                  ///< image derived from exposure-bracketed individual images
            IsHiddenImage = 1u << 6,              ///< Image is hidden. Not intended to be displayed.
            IsCoverImage  = 1u << 7,  ///< Image is cover image, should be displayed when no other information is
                                      ///< available on the preference to display a collection of images.
            IsProtected =
                1u << 8,  ///< There is a Protection Scheme Info Box mapped for this image. The image is not accessible.
            HasLinkedThumbnails              = 1u << 9,   ///< This image has thumbnail image(s) linked to it.
            HasLinkedAuxiliaryImage          = 1u << 10,  ///< This image has auxiliary image(s) linked to it.
            HasLinkedDerivedImage            = 1u << 11,  ///< This image has derived image(s) linked to it.
            HasLinkedPreComputedDerivedImage = 1u << 12,  ///< This image has Pre-derived coded image(s) linked to it.
            HasLinkedTiles = 1u << 13,  ///< This image has linked tiles to it. Used with Relative location 'rloc' Image
                                        ///< Property. Has 'tbas' reference(s) to it.
            HasLinkedMetadata = 1u << 14,  ///< This image has linked external metadata (like Exif). It has 'cdsc' (=
                                           ///< content description) reference(s) to it.

            IsTileImageItem = 1u << 15,  ///< Item is of type 'hvt1' e.g. HEVC tile Item
            IsMetadataItem =
                1u << 16,  ///< Item is metadata item and has 'cdsc' (= content descriptor) reference to other item.
            IsExifItem  = 1u << 17,  ///< Item is metadata of type 'Exif'
            IsXMPItem   = 1u << 18,  ///< Item is metadata of type 'mime' and content type "application/rdf+xml"
            IsMPEG7Item = 1u << 19,  ///< Item is metadata of type 'mime' and not content type "application/rdf+xml"
        };
    }

    struct HEIF_DLL_PUBLIC DecoderConfiguration
    {
        DecoderConfigId decoderConfigId;  ///< Id identifying this particular decoder specific information. More than
                                          ///< one image can use same decoder config.
        Array<DecoderSpecificInfo> decoderSpecificInfo;  ///< Actual decoder specific information (type + payload).
    };

    typedef uint32_t FeatureBitMask;

    struct HEIF_DLL_PUBLIC ItemInformation
    {
        ImageId itemId;
        FourCC type;

        ItemDescription description;

        FeatureBitMask features;  ///< bitmask of ItemFeatureEnum
        uint64_t size;            ///< size of item data in bytes (can be 0 if item doesn't have its own data)
    };

    /**
     * An entity group groups items, and may also contain tracks.
     * This content comes from GroupsListBox.
     */
    struct HEIF_DLL_PUBLIC EntityGrouping
    {
        FourCC type;                ///< Grouping type.
        uint32_t groupId;           ///< Grouping ID, a non-negative integer.
        Array<uint32_t> entityIds;  ///< Grouped entity IDs. These can be image item IDs or track IDs.
    };

    struct HEIF_DLL_PUBLIC MetaBoxInformation
    {
        FeatureBitMask features;  ///< bitmask of MetaBoxFeatureEnum's
        Array<ItemInformation> itemInformations;
        Array<EntityGrouping> entityGroupings;
    };

    namespace TrackFeatureEnum
    {
        /** @brief Media Track or timed image sequence features flag enumeration.
         *
         * A HEVC file may have several media tracks with different feature sets. */
        enum Feature
        {
            IsMasterImageSequence =
                1u,  ///< Track handler type is 'pict', and the track is not referencing any another track.
            IsThumbnailImageSequence = 1u << 1,  ///< Is referencing another track with a 'thmb' type track reference.
            IsAuxiliaryImageSequence = 1u << 2,  ///< Is referencing another track with an 'auxl' type track reference.
            IsEnabled                = 1u << 3,  ///< The track is enabled.
            IsInMovie                = 1u << 4,  ///< The track is used in the presentation.
            IsInPreview              = 1u << 5,  ///< The track is used when previewing the presentation.
            HasAlternatives          = 1u << 6,  ///< The track has alternative track or tracks.
            HasCodingConstraints     = 1u << 7,  ///< From Coding Constraints Box in HevcSampleEntry
            HasSampleGroups          = 1u << 8,  ///< The track has SampleToGroupBoxes.
            HasLinkedAuxiliaryImageSequence = 1u << 9,   ///< There is a 'auxl' track reference pointing to this track.
            HasLinkedThumbnailImageSequence = 1u << 10,  ///< There is a 'thmb' track reference pointing to this track.
            HasSampleToItemGrouping =
                1u << 11,  ///< The track has one or more SampleToMetadataItemEntry ('stmi') sample groups present.
            HasExifSampleEntry      = 1u << 12,  ///< From SampleEntryBox, not implemented yet
            HasXmlSampleEntry       = 1u << 13,  ///< From SampleEntryBox, not implemented yet
            HasEditList             = 1u << 14,  ///< Track has an edit list
            HasInfiniteLoopPlayback = 1u << 15,  ///< Infinite looping has been set on in the edit list
            HasSampleEquivalenceGrouping =
                1u << 16,  ///< The track has one or more VisualSampleGroupEntry ('eqiv') sample group entries present.
            IsAudioTrack      = 1u << 17,  ///< Track is an audio track (handler type 'soun').
            IsVideoTrack      = 1u << 18,  ///< Track is a video track (handler type 'vide').
            DisplayAllSamples = 1u << 19  ///< Edit List presentation indicates 0 or 1 samples. The player should ignore
                                          ///< timestamps and display all non-hidden samples.
        };
    }  // namespace TrackFeatureEnum

    struct HEIF_DLL_PUBLIC FourCCToIds
    {
        FourCC type;
        Array<SequenceId> trackIds;
    };

    /**
     * The SampleAndGroupIds struct combines the id of a sample and the index of a sample group description entry.
     */
    struct HEIF_DLL_PUBLIC SampleAndEntryIds
    {
        SequenceImageId sampleId;              ///< ID of the sample.
        uint32_t sampleGroupDescriptionIndex;  ///< Index of the sample group description entry.
    };

    /**
     * Samples of a track sharing same sample grouping with type and type parameter.
     * As they might still have different sample group description entires, these indexes are
     * listed along every sample in a vector.
     */
    struct HEIF_DLL_PUBLIC SampleGrouping
    {
        FourCC type;                       ///< Grouping type of the sample group.
        uint32_t typeParameter;            ///< Grouping type parameter, the sub-type of the grouping (optional).
        Array<SampleAndEntryIds> samples;  ///< Samples belonging to this group and their group description indexes.
    };

    /**
     * Information from VisualEquivalenceEntry sample group description entry.
     * This information is used when an untimed item is marked as visually equivalent to a sample in a
     * timed sequence ('eqiv' grouping).
     */
    struct HEIF_DLL_PUBLIC SampleVisualEquivalence
    {
        uint32_t sampleGroupDescriptionIndex;  ///< Index of the sample group description entry.
        int16_t timeOffset;  ///< The difference of the related image item(s) and the composition time of the sample.
        uint16_t timescaleMultiplier;  ///< timeOffset unit as multiplier of the media timescale of the track as on 8.8
                                       ///< fixed-point value.
    };

    /**
     * Information from SampleToMetadataItemEntry sample group description entry.
     * This information is used to find Item IDs of metadata that describes samples linked to this entry.
     */
    struct HEIF_DLL_PUBLIC SampleToMetadataItem
    {
        uint32_t sampleGroupDescriptionIndex;  ///< Index of the sample group description entry.
        Array<ImageId> metadataItemIds;        ///< Item Ids to linked metadata.
    };

    /**
     * Information from DirectReferenceSamplesList sample group description entry.
     * This information is used to find Item IDs of reference samples linked to this entry.
     */
    struct HEIF_DLL_PUBLIC DirectReferenceSamples
    {
        uint32_t sampleGroupDescriptionIndex;  ///< Index of the sample group description entry.
        uint32_t sampleId;  ///< When entry corresponds to a reference sample, the value is a positive integer.
                            ///< The value for this field shall be zero for non-reference samples.
        Array<SequenceImageId> referenceItemIds;  ///< sample_id values of the direct reference samples that a sample
                                                  ///< belonging to this group may be predicted from.
    };


    /** @brief SampleType enumeration to indicate the type of the frame. */
    enum SampleType
    {
        OUTPUT_NON_REFERENCE_FRAME,
        OUTPUT_REFERENCE_FRAME,
        NON_OUTPUT_REFERENCE_FRAME
    };

    struct HEIF_DLL_PUBLIC SampleInformation
    {
        SequenceImageId sampleId;         ///< based on the sample's entry order in the sample table
        FourCC sampleEntryType;           ///< coming from SampleDescriptionBox (codingname)
        uint32_t sampleDescriptionIndex;  ///< coming from SampleDescriptionBox index (sample_description_index)
        SampleType sampleType;            ///< coming from sample groupings
        uint64_t sampleDurationTS;        ///< Sample duration in time scale units
        int64_t sampleCompositionOffsetTs;
        bool hasClap;                         ///< CleanApertureBox is present in the sample entry
        bool hasAuxi;                         ///< AuxiliaryTypeInfoBox is present in the sample entry
        CodingConstraints codingConstraints;  ///< CodingConstraints for sample
        uint64_t size;                    ///< size of sample data in bytes
    };

    struct HEIF_DLL_PUBLIC TrackInformation
    {
        SequenceId trackId;
        uint32_t alternateGroupId;
        FeatureBitMask features;                    ///< bitmask of TrackFeatureEnum::Feature
        Array<SequenceId> alternateTrackIds;        ///< other track' IDs with same alternateGroupId.
        Array<FourCCToIds> referenceTrackIds;       ///< <reference_type, reference track IDs>
        Array<SampleGrouping> sampleGroups;         ///< Sample grouping information of samples of this track.
        Array<SampleInformation> sampleProperties;  ///< SampleInformation for each of the samples inside the track.
        Array<SampleVisualEquivalence>
            equivalences;  ///< Data from VisualEquivalenceEntry ('eqiv') sample group entries of this track. Indexed
                           ///< using sampleGroupDescriptionIndex.
        Array<SampleToMetadataItem> metadatas;  ///< Data from SampleToMetadataItemEntry ('stmi') sample group entries
                                                ///< of this track. Indexed using sampleGroupDescriptionIndex.
        Array<DirectReferenceSamples>
            referenceSamples;    ///< Data from  DirectReferenceSamplesList ('refs') sample group entries
                                 ///< of this track. Indexed using sampleGroupDescriptionIndex
        uint64_t maxSampleSize;  ///< Size of largest sample inside the track (can be used to allocate client side read
                                 ///< buffer).
        uint32_t timeScale;      ///< Time scale of the track; useful for video stream procsesing purposes
        EditList editList;       ///< Editlist for this track.
    };

    struct HEIF_DLL_PUBLIC FileInformation
    {
        FeatureBitMask features;  ///< bitmask of FileFeatureEnum's
        MetaBoxInformation rootMetaBoxInformation;
        Array<TrackInformation> trackInformation;
        uint32_t movieTimescale = 0;  ///< In case of Image Sequences/tracks the Movie timescale that can be used in
                                      ///< edit list processing where it is used for EditUnit.durationInMovieTS
    };

}  // namespace HEIF

#endif /* HEIFFILEDATATYPES_H */
