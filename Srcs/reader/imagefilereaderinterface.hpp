/* Copyright (c) 2015-2017, Nokia Technologies Ltd.
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

#ifndef IMAGEFILEREADERINTERFACE_HPP
#define IMAGEFILEREADERINTERFACE_HPP

#include <cstdint>
#include <exception>
#include <map>
#include <set>
#include <string>
#include <vector>

/** @brief Interface for reading an Image File (e.g. HEVC Image File) from the filesystem. */
class ImageFileReaderInterface
{
public:
    ImageFileReaderInterface() = default;
    virtual ~ImageFileReaderInterface() = default;

    /**
     * FileReaderException exception class.
     */
    class FileReaderException : public std::exception
    {
    public:
        /** @brief File reader StatusCode enumeration. */
        enum class StatusCode
        {
            FILE_OPEN_ERROR = 0,
            FILE_HEADER_ERROR,
            FILE_READ_ERROR,
            UNSUPPORTED_CODE_TYPE,
            INVALID_FUNCTION_PARAMETER,
            INVALID_CONTEXT_ID,
            INVALID_ITEM_ID,
            INVALID_PROPERTY_INDEX,
            INVALID_SAMPLE_DESCRIPTION_INDEX,
            PROTECTED_ITEM,
            UNPROTECTED_ITEM,
            UNINITIALIZED,
            NOT_APPLICABLE
            /** @todo Add new status code types here if needed **/
        };

        FileReaderException(StatusCode status, const std::string& message = "") :
            mStatusCode(status),
            mMessage(message)
        {
        }

        const char* what() const throw ()
        {
            if (mMessage != "")
            {
                return mMessage.c_str();
            }

            switch (mStatusCode)
            {
                case StatusCode::FILE_OPEN_ERROR:
                    return "Unable to open input file.";
                case StatusCode::FILE_HEADER_ERROR:
                    return "Error while reading file header.";
                case StatusCode::FILE_READ_ERROR:
                    return "Error while reading file.";
                case StatusCode::UNSUPPORTED_CODE_TYPE:
                    return "Unsupported code type.";
                case StatusCode::INVALID_FUNCTION_PARAMETER:
                    return "Invalid API method parameter.";
                case StatusCode::INVALID_CONTEXT_ID:
                    return "Invalid context ID parameter.";
                case StatusCode::INVALID_ITEM_ID:
                    return "Invalid item ID parameter.";
                case StatusCode::INVALID_PROPERTY_INDEX:
                    return "Invalid item property index.";
                case StatusCode::INVALID_SAMPLE_DESCRIPTION_INDEX:
                    return "Invalid sample description index.";
                case StatusCode::PROTECTED_ITEM:
                    return "The item is unaccessible because it is protected.";
                case StatusCode::UNPROTECTED_ITEM:
                    return "The item is unaccessible because it is protected.";
                case StatusCode::UNINITIALIZED:
                    return "Reader not initialized. Call initialize() first.";
                case StatusCode::NOT_APPLICABLE:
                default:
                    return "Unspecified error.";
            }
        }

        StatusCode getStatusCode() const
        {
            return mStatusCode;
        }

    private:
        StatusCode mStatusCode;
        std::string mMessage;
    };

    // Item property related data types

    /// Item Property type identifiers
    enum class ItemPropertyType
    {
        UNKNOWN, ///< An unknown property type
        AUXC, ///< Image properties for auxiliary images
        AVCC, ///< AVC configuration
        CLAP, ///< Clean aperture (crop)
        HVCC, ///< HEVC configuration
        IMIR, ///< Image mirror
        IROT, ///< Image rotation
        ISPE, ///< Image spatial extents
        LHVC, ///< Layered HEVC configuration
        LSEL, ///< Layer selection
        OINF, ///< Operating points info
        RLOC, ///< Relative location
        TOLS  ///< Target output layer set
    };

    /// Information about a single property associated to an image item
    struct ItemPropertyInfo
    {
        ItemPropertyType type; ///< Type of the property
        std::uint32_t index;   ///< Meta box(/context) -specific index of the property
        bool essential;        ///< True if the property is defined as essential for the item
    };

    /// Data of image properties for auxiliary images
    struct AuxProperty
    {
        std::string auxType;               ///< Type of the associated auxiliary image item
        std::vector<std::uint8_t> subType; ///< Aux subtype, semantics depends on the auxType value
    };

    /// Data of item property Image mirror
    struct ImirProperty
    {
        bool horizontalAxis; ///< Mirror axis (\c true = horizontal, \c false = vertical)
    };

    /// Data of item property Image rotation
    struct IrotProperty
    {
        unsigned int rotation; ///< Anti-clockwise rotation direction in degrees (0, 90, 180, 270)
    };

    /// Data of item property Relative location
    struct RlocProperty
    {
        std::uint32_t horizontalOffset;
        std::uint32_t verticalOffset;
    };

    /// Data of item property Clean aperture
    struct ClapProperty
    {
        std::uint32_t widthN;            ///< Numerator of the fractional number width
        std::uint32_t widthD;            ///< Denominator of the fractional number width
        std::uint32_t heightN;           ///< Numerator of the fractional number height
        std::uint32_t heightD;           ///< Denominator of the fractional number height
        std::uint32_t horizontalOffsetN; ///< Numerator of the fractional number horizontal offset
        std::uint32_t horizontalOffsetD; ///< Denominator of the fractional number horizontal offset
        std::uint32_t verticalOffsetN;   ///< Numerator of the fractional number vertical offset
        std::uint32_t verticalOffsetD;   ///< Denominator of the fractional number vertical offset
    };

    /// Data of item property Target output layer set
    struct TolsProperty
    {
        std::uint16_t targetOlsIndex; ///<  The output layer set index to be provided to the L-HEVC decoding process as the value of TargetOlsIdx
    };

    /// Data of item property Layer selector property
    struct LselProperty
    {
        std::uint16_t layerId; ///< Layer identifier of the image among the reconstructed images.
    };

    // Non-image item type definitions

    /// Data of item type Image overlay
    struct IovlItem
    {
        std::vector<std::uint16_t> canvasFillValue; ///< Four 16-bit words
        std::uint32_t outputWidth;
        std::uint32_t outputHeight;
        std::vector<std::uint32_t> itemIds;
        struct Offset
        {
            std::int32_t horizontal;
            std::int32_t vertical;
        };
        std::vector<Offset> offsets;
    };

    /// Data of item type Grid
    struct GridItem
    {
        std::uint8_t rowsMinusOne;
        std::uint8_t columnsMinusOne;
        std::uint32_t outputWidth;
        std::uint32_t outputHeight;
        std::vector<std::uint32_t> itemIds;
    };

    // Convenience types
    typedef std::uint32_t Timestamp;
    typedef std::uint32_t ItemId;
    typedef std::vector<std::pair<ItemId, Timestamp>> DecodingOrderVector;
    typedef std::vector<std::uint32_t> IdVector;
    typedef std::vector<std::uint8_t> DataVector;
    typedef std::vector<Timestamp> TimestampVector;
    typedef std::map<Timestamp, ItemId> TimestampMap;
    typedef std::map<std::string, IdVector> TypeToIdsMap;
    typedef std::map<std::string, std::vector<IdVector>> GroupingMap;
    typedef std::map<std::string, DataVector> ParameterSetMap;
    typedef std::vector<ItemPropertyInfo> PropertyTypeVector;

    class FileFeature
    {
    public:
        /** Enumerated list of File Features
            @bug Naming convention does not follow style guide (enums should be style e.g. HAS_SINGLE_IMAGE) though it
                 feels a bit intruding considering long enum names here. */
        enum Feature
        {
            HasSingleImage,
            HasImageCollection,
            HasImageSequence,
            HasCoverImage,
            HasOtherTimedMedia,
            HasRootLevelMetaBox,
            HasMoovLevelMetaBox,
            HasAlternateTracks
        };
        typedef std::set<Feature> FileFeatureSet;

        bool hasFeature(Feature feature) const { return mFileFeatureSet.count(feature); }
        void setFeature(Feature feature) { mFileFeatureSet.insert(feature); }
        std::uint32_t getContextId() const { return mContextId; }
        void setContextId(std::uint32_t id) { mContextId = id; }

    private:
        std::uint32_t mContextId;
        FileFeatureSet mFileFeatureSet;
    };

    class TrackFeature
    {
    public:
        /** @brief Media Track or timed image sequence features flag enumeration.
         *
         * A HEVC file may have several media tracks with different feature sets. */
        enum Feature
        {
            HasTrackLevelMetaBox,     ///< @todo Implement
            IsMasterImageSequence,    ///< Sample entry box has 'pict', and is not referencing any another track ('thmb')
            IsThumbnailImageSequence, ///< Is referencing an another track 'thmb' tref
            IsAuxiliaryImageSequence, ///< Is referencing an another track 'auxl' tref
            IsTimedMetadataTrack,     ///< @todo Implement (subtitles for example)
            IsEnabled,                ///< The track is enabled
            IsInMovie,                ///< The track is used in the presentation
            IsInPreview,              ///< The track is used when previewing the presentation
            HasAlternatives,          ///< The track has alternative track or tracks
            HasCodingConstraints,     ///< From Coding Constraints Box in HevcSampleEntry
            HasCoverImage,            ///< @todo Implement
            HasSampleGroups,          ///< The track has SampleToGroupBoxes
            HasLinkedAuxiliaryImageSequence, ///< There is a 'auxl' track reference pointing to this track.
            HasLinkedThumbnailImageSequence, ///< There is a 'thmb' track reference pointing to this track.
            HasLinkedTimedMetaDataTrack,     ///< @todo Implement
            HasSampletoItemGrouping,         ///< SampleToItemGroupingBox
            HasLinkedContentDescription, ///< Track reference of type 'cdsc' to reference image sequence's metadata as a timed metadata track
            HasExifSampleEntry, ///< From SampleEntryBox, not implemented yet
            HasXmlSampleEntry,  ///< From SampleEntryBox, not implemented yet
            HasEditList,        ///< Track has an edit list
            HasInfiniteLoopPlayback ///< Infinite looping has been set on in the edit list
        };
        typedef std::set<Feature> TrackFeatureSet;

        bool hasFeature(Feature feature) const { return mTrackFeatureSet.count(feature); }
        void setFeature(Feature feature) { mTrackFeatureSet.insert(feature); }

    private:
        TrackFeatureSet mTrackFeatureSet;
    };

    /** @brief MOOV level features flag enumeration. There may be a MOOv level Metabox */
    class MoovFeature
    {
    public:
        /// Enumerated list of Moov Features
        enum Feature
        {
            HasMoovLevelMetaBox,
            HasCoverImage
        };
        typedef std::set<Feature> MoovFeatureSet;

        bool hasFeature(Feature feature) const { return mMoovFeatureSet.count(feature); }
        void setFeature(Feature feature) { mMoovFeatureSet.insert(feature); }

    private:
        MoovFeatureSet mMoovFeatureSet;
    };

    /** @brief Represents the features of entities in Meta Box at file, moov or track level. */
    class MetaBoxFeature
    {
    public:
        /// Enumerated list of Meta box Features
        enum Feature
        {
            IsSingleImage,
            IsImageCollection,
            HasMasterImages,
            HasThumbnails,
            HasCoverImage,
            HasAuxiliaryImages,
            HasDerivedImages,
            HasPreComputedDerivedImages,
            HasHiddenImages,
            HasGroupLists,
            HasDataInMetaBox ///< i.e. contains 'idat' box.
        };
        typedef std::set<Feature> MetaBoxFeatureSet;

        bool hasFeature(Feature feature) const { return mMetaBoxFeatureSet.count(feature); }
        void setFeature(Feature feature) { mMetaBoxFeatureSet.insert(feature); }

    private:
        MetaBoxFeatureSet mMetaBoxFeatureSet;
    };

    /** @brief Each image in a MetaBox has different features */
    class ImageFeature
    {
    public:
        /// Enumerated list of image features
        enum Feature
        {
            IsMasterImage,
            IsThumbnailImage,
            IsAuxiliaryImage,
            IsPrimaryImage,
            IsDerivedImage,
            IsPreComputedDerivedImage,
            IsHiddenImage,
            IsCoverImage,
            IsProtected, ///< There is a Protection Scheme Info Box mapped for this image. The image is not accessible.
            HasLinkedThumbnails,
            HasLinkedAuxiliaryImage,
            HasLinkedDerivedImage,
            HasLinkedPreComputedDerivedImage,
            HasLinkedTiles,
            HasLinkedMetadata ///< 'cdsc' reference
        };
        typedef std::set<Feature> ImageFeatureSet;

        bool hasFeature(Feature feature) const { return mImageFeatureSet.count(feature); }
        void setFeature(Feature feature) { mImageFeatureSet.insert(feature); }

    private:
        ImageFeatureSet mImageFeatureSet;
    };

    /** @brief Each item in a MetaBox has different features */
    class ItemFeature
    {
    public:
        enum Feature
        {
            IsTileImageItem,
            IsMetadataItem,
            IsExifItem,
            IsMimeItem,
            IsProtected ///< There is a Protection Scheme Info Box mapped for this item. The item is not accessible.
        };
        typedef std::set<Feature> ItemFeatureSet;

        bool hasFeature(Feature feature) const { return mItemFeatureSet.count(feature); }
        void setFeature(Feature feature) { mItemFeatureSet.insert(feature); }

    private:
        ItemFeatureSet mItemFeatureSet;
    };

    class MetaDataFeature
    {
    public:
        enum Feature
        {
            IsExif
        };
        typedef std::set<Feature> MetaDataFeatureSet;

        bool hasFeature(Feature feature) const { return mMetaDataFeatureSet.count(feature); }
        void setFeature(Feature feature) { mMetaDataFeatureSet.insert(feature); }

    private:
        MetaDataFeatureSet mMetaDataFeatureSet;
    };

    class CodingConstraints
    {
    public:
        enum Feature
        {
            IsAllReferencePicturesIntra,
            IsIntraPredictionUsed
        };
        typedef std::set<Feature> CodingConstraintsSet;

        bool hasFeature(Feature feature) const { return mCodingConstraintsSet.count(feature); }
        void setFeature(Feature feature) { mCodingConstraintsSet.insert(feature); }

    private:
        CodingConstraintsSet mCodingConstraintsSet;
    };

    // Forward declarations
    struct TrackProperties;
    struct SampleProperties;

    // In the type definitions, the first element of each map represents the file-given ID of the relevant data structure.
    // In he below mentioned type definitions, key value is the ID of the entity.
    typedef std::map<std::uint32_t, ImageFeature> ImageFeaturesMap;
    typedef std::map<std::uint32_t, ItemFeature> ItemFeaturesMap;
    typedef std::map<std::uint32_t, TrackProperties> TrackPropertiesMap;   ///< <track_id/context_id, TrackProperties>
    typedef std::map<std::uint32_t, SampleProperties> SamplePropertiesMap;

    /** @brief MetaBox Property definition that contains image and item features */
    struct MetaBoxProperties
    {
        std::uint32_t contextId;
        MetaBoxFeature metaBoxFeature;
        ImageFeaturesMap imageFeaturesMap;
        ItemFeaturesMap itemFeaturesMap;
        GroupingMap entityToGroupMaps; ///< <grouping_type, vector of entity ID vectors (item ID or track ID>
    };

    /** @brief Moov Property definition that may contain a MetaBox */
    struct MoovProperties
    {
        std::uint32_t moovId;
        MoovFeature moovFeature;
        MetaBoxProperties metaBoxProperties;
    };

    /** @brief SampleType enumeration to indicate the type of the frame. */
    enum SampleType
    {
        OUTPUT_NON_REFERENCE_FRAME,
        OUTPUT_REFERENCE_FRAME,
        NON_OUTPUT_REFERENCE_FRAME
    };

    /** @brief Sample Property definition */
    struct SampleProperties
    {
        std::uint32_t sampleId; ///< based on the sample's entry order in the sample table
        std::string sampleEntryType; ///< coming from SampleDescriptionBox (codingname)
        std::uint32_t sampleDescriptionIndex;  ///< coming from SampleDescriptionBox index (sample_description_index)

        std::map<std::string, std::uint32_t> sampleGroupsMap; ///< <sample_group_type, sample group description index id> ... coming from sample group description and SampleGroupBox

        SampleType sampleType; ///< coming from sample groupings
        CodingConstraints codingConstraints;
        bool hasClap; ///< CleanApertureBox is present in the sample entry
        /// @todo Add support for tiles and sub-samples here.
    };

    /** @brief Track Property definition which contain sample properties.
     *
     * In the samplePropertiesMap, samples of the track are listed in the same order they appear
     * in the sample size or sample to chunk boxes.
     * Each sample is given an ID, which is used as the key of the map.
     */
    struct TrackProperties
    {
        std::uint32_t trackId; ///< trackId is also context id
        std::uint32_t alternateGroupId;
        TrackFeature trackFeature;
        MetaBoxProperties trackLevelMetaBoxProperties;
        SamplePropertiesMap sampleProperties;
        IdVector alternateTrackIds; ///< other tracks IDs with the same alternate_group id.
        TypeToIdsMap referenceTrackIds; ///< <reference_type, reference track ID> (coming from 'tref')
        TypeToIdsMap trackGroupIds; ///< <group_type, track IDs> ... coming from Track Group Box 'trgr'
        TypeToIdsMap sampleGroupIds; ///< grouping_type as string and samples that belong to this group.
        /// @todo Edit Lists will be included
    };

    /** @brief Overall File Property definition which contains file's properties.*/
    struct FileProperties
    {
        FileFeature fileFeature;
        MoovProperties moovFeatures;
        TrackPropertiesMap trackProperties;
        MetaBoxProperties rootLevelMetaBoxProperties;
    };

    /*---------- Interface methods are defined as follows:--------------------- */

    /** Open file for reading and reads the file header information.
     *  @param [in] fileName File to open.
     *  @throws FileReaderException, StatusCode=[FILE_OPEN_ERROR, FILE_READ_ERROR, FILE_HEADER_ERROR] */
    virtual void initialize(const std::string& fileName) = 0;

    /** Open an input stream for reading and reads the header information.
     *  @param input stream to open.
     *  @throws FileReaderException, StatusCode=[FILE_HEADER_ERROR, FILE_READ_ERROR] */
    virtual void initialize(std::istream& input) = 0;

    /** Placeholder to do any necessary clean up. */
    virtual void close() = 0;

    /** @return Major brand from the File Type Box
     *  @pre initialize() has been called successfully.
     *  @throws FileReaderException, StatusCode=[UNINITIALIZED] */
    virtual std::string getMajorBrand() const = 0;

    /** @return Minor version from the File Type Box
     *  @pre initialize() has been called successfully.
     *  @throws FileReaderException, StatusCode=[UNINITIALIZED] */
    virtual std::uint32_t getMinorVersion() const = 0;

    /** @return Compatible brands list from the File Type Box
     *  @pre initialize() has been called successfully.
     *  @throws FileReaderException, StatusCode=[UNINITIALIZED] */
    virtual std::vector<std::string> getCompatibleBrands() const = 0;

    /** Get file properties.
     *  These properties can be used to further initialize the presentation of the data in the file.
     *  Properties also give hints about the way and means to request data from the file.
     *  @return Properties of the file.
     *  @pre initialize() has been called successfully.
     *  @throws FileReaderException, StatusCode=[UNINITIALIZED] */
    virtual const FileProperties& getFileProperties() const { return mFileProperties; }

    /** Get maximum display width from track headers.
     *  @param [in] contextId Context ID of a track.
     *  @return Maximum display width in pixels.
     *  @pre getFileProperties() has been called successfully.
     *  @throws FileReaderException, StatusCode=[UNINITIALIZED, INVALID_CONTEXT_ID] */
    virtual std::uint32_t getDisplayWidth(std::uint32_t contextId) const = 0;

    /** Get maximum display height from track headers.
     *  @param [in] contextId Context ID of a track.
     *  @return Maximum display height in pixels.
     *  @pre getFileProperties() has been called successfully.
     *  @throws FileReaderException, StatusCode=[UNINITIALIZED, INVALID_CONTEXT_ID] */
    virtual std::uint32_t getDisplayHeight(std::uint32_t contextId) const = 0;

    /** Get width of item or entity.
     *  @param [in] contextId A track or metabox context id.
     *  @param [in] itemId    itemId can be any image item/sample in the metabox/track designated by the contextId
     *  @return Width in pixels.
     *  @pre getFileProperties() has been called successfully.
     *  @throws FileReaderException, StatusCode=[UNINITIALIZED, INVALID_CONTEXT_ID, INVALID_ITEM_ID] */
    virtual std::uint32_t getWidth(std::uint32_t contextId, std::uint32_t itemId) const = 0;

    /** Get height of item or entity.
     *  @param [in] contextId A track or metabox context id.
     *  @param [in] itemId    Item can be any image item/sample in the metabox/track designated by the contextId
     *  @return Height in pixels.
     *  @pre getFileProperties() has been called successfully.
     *  @throws FileReaderException, StatusCode=[UNINITIALIZED, INVALID_CONTEXT_ID, INVALID_ITEM_ID] */
    virtual std::uint32_t getHeight(std::uint32_t contextId, std::uint32_t itemId) const = 0;

    /** Get transformation matrix for the video, from the Movie Header Box of the file (if present).
     *  @return The transformation matrix, consisting of 9 32-bit integers. An empty vector if a Movie Header Box
     *          is not present in the file.
     *  @pre initialize() has been called successfully.
     *  @throws FileReaderException, StatusCode=[UNINITIALIZED] */
    virtual std::vector<std::int32_t> getMatrix() const = 0;

    /** Get playback duration of image sequence or media track in seconds.
     *  This considers also edit lists.
     *  @param [in] contextId ID of a timed media track such as an image sequence track.
     *  @return The playback duration of image sequence or media track in seconds.
     *          Returns (# of displayable master images)/(forced framerate), if a forced sequential playback is
     *          signaled by calling SetForcedTimedPlayback() for a metabox.
     *          Returns 0 for an image collection or other items.
     *  @pre getFileProperties() has been called successfully.
     *  @throws FileReaderException, StatusCode=[UNINITIALIZED, INVALID_CONTEXT_ID] */
    virtual float getPlaybackDurationInSecs(std::uint32_t contextId) const = 0;

    /** Set the forced timed playback mode for an image sequence in metabox.
     *  If the resource pointed by contextId is a media track, then the values are ignored.
     *  If the resource is a metabox, a forced generation of timestamps can be done by defining a forced FPS.
     *  @param [in] contextId   Metabox context id.
     *  @param [in] forcedFps   Frames per second
     *  @pre getFileProperties() has been called successfully.
     *  @throws FileReaderException, StatusCode=[UNINITIALIZED, INVALID_CONTEXT_ID] */
    virtual void setForcedTimedPlayback(std::uint32_t contextId, float forcedFps) = 0;

    /** Set the forced loop playback mode for an image sequence/samples in metabox/track.
     *  @param [in] contextId         Context ID of a media track
     *  @param [in] forceLoopPlayback True = set forced loop playback on, false = set forced loop playback off.
     *  @pre getFileProperties() has been called successfully.
     *  @throws FileReaderException, StatusCode=[UNINITIALIZED, INVALID_CONTEXT_ID] */
    virtual void setForcedLoopPlayback(std::uint32_t contextId, bool forceLoopPlayback = true) = 0;

    /** Get list of items in the container with the ID contextId having the requested itemType.
     *  @param [in]  contextId Track or metabox context id.
     *  @param [in]  itemType  If contextId refers to a MetaBox then itemType can be the following:
     *                         'master' , 'hidden', 'pre-computed', 'hvc1', 'iovl', 'grid', 'Exif', 'mime', 'hvt1',
     *                         'iden', 'lhv1'
     *                         ('master' is ('hvc1' - iref('thmb') or iref('auxl')))
     *                         If the contextId refers to a media track; then itemType can be the following:
     *                         'out_ref' : output reference frames
     *                         'out_non_ref' : output non-reference frames
     *                         'non_out_ref' : non-output reference frame
     *                         'display' : all frame samples in the track which are displayed and in display order
     *                         'samples' : all samples in the track in track's entry order
     *  @param [out] itemIds   Found items. The order of the itemIds are as present in the file.
     *                         An empty vector if no items are found.
     *  @pre initialize() has been called successfully.
     *  @throws FileReaderException, StatusCode=[UNINITIALIZED, INVALID_CONTEXT_ID, INVALID_ITEM_ID] */
    virtual void getItemListByType(std::uint32_t contextId, const std::string& itemType, IdVector& itemIds) const = 0;

    /** Get itemType of the item pointed by {contextId,itemId} pair
     *  The order of the itemIds are as present in the file
     *  @param [in] contextId Track or metabox context id.
     *  @param [in] itemId    Track or metabox sample/item id.
     *  @return The itemType of the item pointed by {contextId,itemId} pair.
     *          If the context is a metabox, can be the following:
     *          'master' , 'hidden', 'pre-computed', 'hvc1', 'iovl', 'grid', 'Exif', 'mime', 'hvt1', 'iden', 'lhv1'
     *          ('master' is ('hvc1' - iref('thmb') or iref('auxl')))'
     *          If the context is a media track sample description entry type is returned.
     *  @pre initialize() has been called successfully.
     *  @throws FileReaderException, StatusCode=[UNINITIALIZED, INVALID_CONTEXT_ID, INVALID_ITEM_ID] */
    virtual std::string getItemType(std::uint32_t contextId, std::uint32_t itemId) const = 0;

    /** Get list of referenced items for item with {contextId, itemId} pair having the requested referenceType.
     *  @param [in] contextId     Metabox context id.
     *  @param [in] fromItemId    Metabox item id.
     *  @param [in] referenceType Can be the following: 'thmb', 'cdcs', 'auxl', 'dimg', 'base'
     *                            ('master' is ('hvc1' - iref('thmb') or iref('auxl')))
     *  @param [out] itemIds      List of referenced items for item with {contextId, itemId} pair having the
     *                            requested referenceType.
     *                            The order of the itemIds are as present in the file.
     *                            An empty vector if no items are found.
     *  @pre initialize() has been called successfully.
     *  @throws FileReaderException, StatusCode=[UNINITIALIZED, INVALID_CONTEXT_ID, INVALID_ITEM_ID] */
    virtual void getReferencedFromItemListByType(std::uint32_t contextId, std::uint32_t fromItemId, const std::string& referenceType,
        IdVector& itemIds) const = 0;

    /** Get the list of referenced items for item with {contextId, itemId} pair having the requested referenceType.
     *  @param [in]  contextId     Metabox context id.
     *  @param [in]  toItemId      Metabox item id.
     *  @param [in]  referenceType Can be the following: 'thmb', 'cdcs', 'auxl', 'dimg', 'base'
     *                             ('master' is ('hvc1' - iref('thmb') or iref('auxl')))
     *  @param [out] itemIds       Found items. The order of the itemIds are as present in the file.
     *                             Empty vector if no items are found.
     *  @pre initialize() has been called successfully.
     *  @throws FileReaderException, StatusCode=[UNINITIALIZED, INVALID_CONTEXT_ID, INVALID_ITEM_ID] */
    virtual void getReferencedToItemListByType(std::uint32_t contextId, std::uint32_t toItemId, const std::string& referenceType,
        IdVector& itemIds) const = 0;

    /** Get the ID of the primary frame of the frame source. This is the default frame to display.
     *  @param [in] contextId  Track or metabox context id.
     *  @return Item ID of the primary frame of the frame source
     *  @pre initialize() has been called successfully.
     *  @throws FileReaderException, StatusCode=[UNINITIALIZED, INVALID_CONTEXT_ID] */
    virtual std::uint32_t getCoverImageItemId(std::uint32_t contextId) const = 0;

    /** Get item data for given {contextId, itemId} pair.
     *  Clear the itemData vector and fill it with the item data of the requested itemId.
     *  Item Data does not contain initialization or configuration data (i.e. decoder configuration records),
     *  it is pure item data except for samples and 'hvc1'/'avc1'/'lhv1' type items.
     *  For protected items pure data is returned always. Information how to handle such data is available from
     *  getItemProtectionScheme() which returns related 'sinf' box as whole. Note that getItemData() is the only reader
     *  API method which can be used for requesting data of such items.
     *  @param [in]  contextId  Track or metabox context id.
     *  @param [in]  itemId     Track or metabox item id.
     *  @param [out] itemData   Retrieved item data.
     *  @pre initialize() has been called successfully.
     *  @throws FileReaderException, StatusCode=[UNINITIALIZED, INVALID_CONTEXT_ID, INVALID_ITEM_ID] */
    virtual void getItemData(std::uint32_t contextId, std::uint32_t itemId, DataVector& itemData) = 0;

    /** Get data for item of type Image overlay for given {contextId, itemId} pair.
     *  @param [in]  contextId  Meta box context id
     *  @param [in]  itemId     Id of Image overlay item
     *  @return IovlItem struct with requested data.
     *  @pre initialize() has been called successfully.
     *  @throws FileReaderException, StatusCode=[UNINITIALIZED, INVALID_CONTEXT_ID, INVALID_ITEM_ID, PROTECTED_ITEM] */
    virtual IovlItem getItemIovl(std::uint32_t contextId, std::uint32_t itemId) const = 0;

    /** Get data for item of type Image grid for given {contextId, itemId} pair.
     *  @see getItemIovl() */
    virtual GridItem getItemGrid(std::uint32_t contextId, std::uint32_t itemId) const = 0;

    /** Get item property Image Mirror ('imir')
     *  @param [in]  contextId     Meta box context id.
     *  @param [in]  index Index of the property. This value is given by getItemProperties().
     *  @pre initialize() has been called successfully.
     *  @throws FileReaderException, StatusCode=[UNINITIALIZED, INVALID_CONTEXT_ID, INVALID_PROPERTY_INDEX] */
    virtual ImirProperty getPropertyImir(std::uint32_t contextId, std::uint32_t index) const = 0;

    /** Get item property Image Rotation ('irot')
     *  @param [in]  contextId     Meta box context id.
     *  @param [in]  index Index of the property. This value is given by getItemProperties().
     *  @pre initialize() has been called successfully.
     *  @throws FileReaderException, StatusCode=[UNINITIALIZED, INVALID_CONTEXT_ID, INVALID_PROPERTY_INDEX] */
    virtual IrotProperty getPropertyIrot(std::uint32_t contextId, std::uint32_t index) const = 0;

    /** Get image properties for auxiliary images ('auxC')
     *  @see getPropertyIrot() */
    virtual AuxProperty getPropertyAuxc(std::uint32_t contextId, std::uint32_t index) const = 0;

    /** Get item property Relative Location ('rloc')
     *  @see getPropertyIrot() */
    virtual RlocProperty getPropertyRloc(std::uint32_t contextId, std::uint32_t index) const = 0;

    /** Get data for Layer selector property ('lsel')
     *  @see getPropertyIrot() */
    virtual LselProperty getPropertyLsel(std::uint32_t contextId, std::uint32_t index) const = 0;

    /** Get data for Target output layer set property ('tols')
     *  @see getPropertyIrot() */
    virtual TolsProperty getPropertyTols(std::uint32_t contextId, std::uint32_t index) const = 0;

    /** Get property Clean aperture ('clap')
     *  @param [in] contextId Meta box or track context id.
     *  @param [in] index     Index of the property. For meta context this is an appropriate index given by
     *                        getItemProperties(). For tracks it is the sample description index.
     *  @pre initialize() has been called successfully.
     *  @throws FileReaderException, StatusCode=[UNINITIALIZED, INVALID_CONTEXT_ID, INVALID_PROPERTY_INDEX,
     *          INVALID_SAMPLE_DESCRIPTION_INDEX] */
    virtual ClapProperty getPropertyClap(std::uint32_t contextId, std::uint32_t index) const = 0;

    /** Get properties of an item
     *  @param [in]  contextId  Context ID to operate in. Must be a meta box.
     *  @param [in]  itemId     Item ID which properties to get.
     *  @pre initialize() has been called successfully.
     *  @throws FileReaderException, StatusCode=[UNINITIALIZED, INVALID_CONTEXT_ID, INVALID_ITEM_ID] */
    virtual PropertyTypeVector getItemProperties(std::uint32_t contextId, std::uint32_t itemId) const = 0;

    /** Create decodable item data for given {contextId, itemId} pair.
     *  This method can be used to feed the decoder with data.
     *  This method shall not be used if the item is not of 'hvc1', 'lhv1' or 'master' type. Use getItemData instead!
     *  @param [in] contextId                       Track or metabox context id.
     *  @param [in] itemId                          Track or metabox item id.
     *  @param [out] itemDataWithDecoderParameters  Decoder specific parameter initializations as part of the bitstream
     *  @pre initialize() has been called successfully.
     *  @throws FileReaderException, StatusCode=[UNINITIALIZED, INVALID_CONTEXT_ID, INVALID_ITEM_ID, PROTECTED_ITEM] */
    virtual void getItemDataWithDecoderParameters(std::uint32_t contextId, std::uint32_t itemId,
        DataVector& itemDataWithDecoderParameters) = 0;

    /** Get Protection Scheme Information Box for a protected item
     *  @param [in] contextId Metabox context id.
     *  @param [in] itemId    Metabox item id.
     *  @pre initialize() has been called successfully.
     *  @throws FileReaderException, StatusCode=[UNINITIALIZED, INVALID_CONTEXT_ID, INVALID_ITEM_ID, UNPROTECTED_ITEM]
     *  @return The 'sinf' box as a binary blob.
     */
    virtual DataVector getItemProtectionScheme(std::uint32_t contextId, std::uint32_t itemId) const = 0;

    /** Get display timestamp for each item.
     *  If the resource pointed by contextId is a media track, then timestamps are read from the track sample data.
     *  If the resource is a metabox, a forced generation of timestamps can be done by defining a forced FPS and
     *  a forced tickspersecond.
     *  @param [in]  contextId  Metabox or track context ID.
     *  @param [out] timestamps Items of type "master", or samples in the resource as <timestamp in milliseconds, itemId> pair
     *                          Non-output items/frames/samples are not listed here.
     *  @pre initialize() has been called successfully.
     *  @throws FileReaderException, StatusCode=[UNINITIALIZED, INVALID_CONTEXT_ID] */
    virtual void getItemTimestamps(std::uint32_t contextId, TimestampMap& timestamps) const = 0;

    /** Get display timestamps for an item. An item may be displayed many times based on the edit list.
     *  If the resource pointed by contextId is a media track, then timestamp is read from the track with ID contextId
     *  and sample with ID equal to itemId.
     *  If the resource is a metabox, a forced generation of timestamps can be done by defining a forced FPS and
     *  a forced tickspersecond.
     *  @param [in]  contextId  Track or metabox context id.
     *  @param [in]  itemId     Track or metabox item id.
     *  @param [out] timestamps Vector of timestamps. For non-output samples, an empty vector.
     *  @pre initialize() has been called successfully.
     *  @throws FileReaderException, StatusCode=[UNINITIALIZED, INVALID_CONTEXT_ID, INVALID_ITEM_ID] */
    virtual void getTimestampsOfItem(std::uint32_t contextId, std::uint32_t itemId, TimestampVector& timestamps) const = 0;

    /** Get items in decoding order.
     *  If the resource pointed by contextId is a media track, then timestamp is read from the track with ID contextId
     *  and sample with ID equal to itemId.
     *  If the resource is a metabox, a forced generation of timestamps can be done by defining a forced FPS.
     *  @param [in]  contextId         Track or metabox context id.
     *  @param [out] itemDecodingOrder Vector of <item ID, display timestamp> pairs.
     *                                 Also complete decoding dependencies are listed here. If an item ID is present
     *                                 as a decoding dependency for a succeeding frame, its timestamp is set to 0xffffffff.
     *  @pre initialize() has been called successfully.
     *  @throws FileReaderException, StatusCode=[UNINITIALIZED, INVALID_CONTEXT_ID] */
    virtual void getItemsInDecodingOrder(std::uint32_t contextId, DecodingOrderVector& itemDecodingOrder) const = 0;

    /** Retrieve item dependencies for given itemId in the order of entry that should be decoded before the item with itemId.
     *  This method should be used to retrieve referenced samples of a sample in a track.
     *  Decoder configuration dependencies are NOT returned by this method as dependent items.
     *  @param [in] contextId       Track (or metabox) context id.
     *  @param [in] itemId          Unique identifier of the input item/sample
     *  @param [out] dependencies   Vector including dependent item IDs:
     *                              For Metabox items, this method should return the same itemId.
     *                              For images that has no dependencies, this method returns the same itemId as the input itemId.
     *  @pre initialize() has been called successfully.
     *  @throws FileReaderException, StatusCode=[UNINITIALIZED, INVALID_CONTEXT_ID, INVALID_ITEM_ID] */
    virtual void getItemDecodeDependencies(std::uint32_t contextId, std::uint32_t itemId, IdVector& dependencies) const = 0;

    /** Get decoder code type for image/sample
     *  This method can be called by Player implementations that require a separate hardware decoder initialization
     *  before the first frame data is fed (e.g. Android)
     *  @param [in]  contextId     Track or metabox context id.
     *  @param [in]  itemId        Unique identifier of an image item/sample
     *  @return Decoder code type, e.g. "hvc1" or "avc1"
     *  @pre initialize() has been called successfully.
     *  @throws FileReaderException, StatusCode=[UNINITIALIZED, INVALID_CONTEXT_ID, INVALID_ITEM_ID] */
    virtual std::string getDecoderCodeType(const uint32_t contextId, const uint32_t itemId) const = 0;

    /** Get decoder configuration record parameter sets.
     *  This method can be called by Player implementations that require a separate hardware decoder initialization
     *  before the first frame data is fed (e.g. Android)
     *  @param [in]  contextId     Track or metabox context id.
     *  @param [in]  itemId        Unique identifier of an image item/sample
     *  @param [out] parameterSets Decoder configuration record parameter setsParameter sets in a map that contains
     *                             <parameter set type, bitstream> pairs.
     *                             Parameter set types are defined as "SPS", "PPS" and "VPS" for HEVC.
     *  @pre initialize() has been called successfully.
     *  @throws FileReaderException, StatusCode=[UNINITIALIZED, INVALID_CONTEXT_ID, INVALID_ITEM_ID] */
    virtual void getDecoderParameterSets(std::uint32_t contextId, std::uint32_t itemId,
       ParameterSetMap& parameterSets) const = 0;

protected:
    /** Set file properties.
     *  @see getFileProperties()
     *  @param [in] fileProperties New file properties.
     *  @pre initialize() has been called successfully.
     *  @throws FileReaderException, StatusCode=[UNINITIALIZED] */
    virtual void setFileProperties(const FileProperties& fileProperties) { mFileProperties = fileProperties; }

private:
    /** The File Properties object that contains all the properties enumerated using above-mentioned data
     *  structures and classes */
    FileProperties mFileProperties;
};

#endif /* IMAGEFILEREADERINTERFACE_HPP */

