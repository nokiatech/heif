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

#ifndef HEIFREADERIMPL_HPP
#define HEIFREADERIMPL_HPP

#include "decodepts.hpp"
#include "filetypebox.hpp"
#include "heiffiledatatypesinternal.hpp"
#include "heifreader.h"
#include "heifstreamgeneric.hpp"
#include "heifstreaminternal.hpp"
#include "imagespatialextentsproperty.hpp"
#include "metabox.hpp"
#include "moviebox.hpp"

class CleanApertureBox;
class AuxiliaryTypeInfoBox;
class DecoderConfigurationRecord;

namespace HEIF
{
    /** @brief Implementation for reading an HEIF image file from the filesystem. */
    class HeifReaderImpl : public Reader
    {
    public:
        HeifReaderImpl();
        virtual ~HeifReaderImpl() = default;

        /// @see Reader::initialize()
        virtual ErrorCode initialize(const char* fileName);

        /// @see Reader::initialize()
        virtual ErrorCode initialize(StreamInterface* input);

        /// @see Reader::close()
        virtual void close();

        /// @see Reader::getMajorBrand()
        virtual ErrorCode getMajorBrand(FourCC& majorBrand) const;

        /// @see Reader::getMinorVersion()
        virtual ErrorCode getMinorVersion(uint32_t& minorVersion) const;

        /// @see Reader::getCompatibleBrands()
        virtual ErrorCode getCompatibleBrands(Array<FourCC>& compatibleBrands) const;

        /// @see Reader::getFileInformation()
        virtual ErrorCode getFileInformation(FileInformation& fileinfo) const;

        /// @see Reader::getDisplayWidth()
        virtual ErrorCode getDisplayWidth(const SequenceId& sequenceId, uint32_t& displayWidth) const;

        /// @see Reader::getDisplayHeight()
        virtual ErrorCode getDisplayHeight(const SequenceId& sequenceId, uint32_t& displayHeight) const;

        /// @see Reader::getWidth()
        virtual ErrorCode getWidth(const ImageId& itemId, uint32_t& width) const;
        virtual ErrorCode getWidth(const SequenceId& sequenceId, const SequenceImageId& itemId, uint32_t& width) const;

        /// @see Reader::getHeight()
        virtual ErrorCode getHeight(const ImageId& itemId, uint32_t& width) const;
        virtual ErrorCode getHeight(const SequenceId& sequenceId, const SequenceImageId& itemId, uint32_t& width) const;

        /// @see Reader::getMatrix()
        virtual ErrorCode getMatrix(Array<std::int32_t>& matrix) const;

        /// @see Reader::getMatrix()
        virtual ErrorCode getMatrix(const SequenceId& sequenceId, Array<int32_t>& matrix) const;

        /// @see Reader::getPlaybackDurationInSecs()
        virtual ErrorCode getPlaybackDurationInSecs(const SequenceId& sequenceId, double& durationInSecs) const;

        /// @see Reader::getMasterImages()
        virtual ErrorCode getMasterImages(Array<ImageId>& itemIds) const;
        virtual ErrorCode getMasterImages(const SequenceId& contextId, Array<SequenceImageId>& itemIds) const;

        /// @see Reader::getItemListByType()
        virtual ErrorCode getItemListByType(const FourCC& itemType, Array<ImageId>& itemIds) const;

        /// @see Reader::getItemListByType()
        virtual ErrorCode getItemListByType(const SequenceId& sequenceId,
                                            const TrackSampleType& itemType,
                                            Array<SequenceImageId>& itemIds) const;

        /// @see Reader::getItemType()
        virtual ErrorCode getItemType(const ImageId& itemId, FourCC& type) const;

        /// @see Reader::getItemType()
        virtual ErrorCode getItemType(const SequenceId& sequenceId, const SequenceImageId& itemId, FourCC& type) const;

        /// @see Reader::getReferencedFromItemListByType()
        virtual ErrorCode getReferencedFromItemListByType(const ImageId& id,
                                                          const FourCC& referenceType,
                                                          Array<ImageId>& itemIds) const;

        /// @see Reader::getReferencedToItemListByType()
        virtual ErrorCode getReferencedToItemListByType(const ImageId& toItemId,
                                                        const FourCC& referenceType,
                                                        Array<ImageId>& itemIds) const;

        /// @see Reader::getPrimaryItem()
        virtual ErrorCode getPrimaryItem(ImageId& itemId) const;

        /// @see Reader::getItemData()
        virtual ErrorCode getItemData(const ImageId& itemId,
                                      uint8_t* memoryBuffer,
                                      uint64_t& memoryBufferSize,
                                      bool bytestreamHeaders = true) const;

        /// @see Reader::getItemData()
        virtual ErrorCode getItemData(const SequenceId& sequenceId,
                                      const SequenceImageId& itemId,
                                      uint8_t* memoryBuffer,
                                      uint64_t& memoryBufferSize,
                                      bool bytestreamHeaders = true) const;

        /// @see Reader::getItem()
        virtual ErrorCode getItem(const ImageId& itemId, Overlay& iovlItem) const;

        /// @see Reader::getItem()
        virtual ErrorCode getItem(const ImageId& itemId, Grid& gridItem) const;

        /// @see Reader::getProperty()
        virtual ErrorCode getProperty(const PropertyId& index, Mirror& imir) const;

        /// @see Reader::getProperty()
        virtual ErrorCode getProperty(const PropertyId& index, Rotate& irot) const;

        /// @see Reader::getProperty()
        virtual ErrorCode getProperty(const PropertyId& index, AuxiliaryType& auxC) const;

        /// @see Reader::getProperty()
        virtual ErrorCode getProperty(const SequenceId& trackId, const std::uint32_t index, AuxiliaryType& auxC) const;

        /// @see Reader::getProperty()
        virtual ErrorCode getProperty(const PropertyId& index, RelativeLocation& rloc) const;

        /// @see Reader::getProperty()
        virtual ErrorCode getProperty(const PropertyId& index, PixelInformation& pixi) const;

        /// @see Reader::getProperty()
        virtual ErrorCode getProperty(const PropertyId& index, PixelAspectRatio& pasp) const;

        /// @see Reader::getProperty()
        virtual ErrorCode getProperty(const PropertyId& index, ColourInformation& colr) const;

        /// @see Reader::getProperty()
        virtual ErrorCode getProperty(const PropertyId& index, CleanAperture& clap) const;

        /// @see Reader::getProperty()
        virtual ErrorCode getProperty(const SequenceId& trackId, const std::uint32_t index, CleanAperture& clap) const;

        /// @see Reader::getItemProperties()
        virtual ErrorCode getItemProperties(const ImageId& itemId, Array<ItemPropertyInfo>& propertyTypes) const;

        /// @see Reader::getItemProperties()
        virtual ErrorCode getProperty(const PropertyId& index, RawProperty& property) const;

        /// @see Reader::getItemDataWithDecoderParameters()
        virtual ErrorCode getItemDataWithDecoderParameters(const ImageId& itemId,
                                                           uint8_t* memoryBuffer,
                                                           uint64_t& memoryBufferSize) const;

        /// @see Reader::getItemDataWithDecoderParameters()
        virtual ErrorCode getItemDataWithDecoderParameters(const SequenceId& sequenceId,
                                                           const SequenceImageId& itemId,
                                                           uint8_t* memoryBuffer,
                                                           uint64_t& memoryBufferSize) const;

        /// @see Reader::getItemProtectionScheme()
        virtual ErrorCode getItemProtectionScheme(const ImageId& itemId,
                                                  uint8_t* memoryBuffer,
                                                  uint64_t& memoryBufferSize) const;

        /// @see Reader::getItemTimestamps()
        virtual ErrorCode getItemTimestamps(const SequenceId& sequenceId, Array<TimestampIDPair>& timestamps) const;

        /// @see Reader::getTimestampsOfItem()
        virtual ErrorCode getTimestampsOfItem(const SequenceId& sequenceId,
                                              const SequenceImageId& itemId,
                                              Array<int64_t>& timestamps) const;

        /// @see Reader::getItemsInDecodingOrder()
        virtual ErrorCode getItemsInDecodingOrder(const SequenceId& sequenceId,
                                                  Array<TimestampIDPair>& decodingOrder) const;

        /// @see Reader::getDecodeDependencies()
        virtual ErrorCode getDecodeDependencies(const SequenceId& sequenceId,
                                                const SequenceImageId& itemId,
                                                Array<SequenceImageId>& dependencies) const;

        /// @see Reader::getDecoderCodeType()
        virtual ErrorCode getDecoderCodeType(const ImageId& itemId, FourCC& type) const;

        /// @see Reader::getDecoderCodeType()
        virtual ErrorCode getDecoderCodeType(const SequenceId& sequenceId,
                                             const SequenceImageId& itemId,
                                             FourCC& type) const;

        /// @see Reader::getDecoderParameterSets()
        virtual ErrorCode getDecoderParameterSets(const ImageId& itemId, DecoderConfiguration& decoderInfos) const;

        /// @see Reader::getDecoderParameterSets()
        virtual ErrorCode getDecoderParameterSets(const SequenceId& sequenceId,
                                                  const SequenceImageId& itemId,
                                                  DecoderConfiguration& decoderInfos) const;

    private:
        enum class State
        {
            UNINITIALIZED,  ///< State before starting to read file and after closing it
            INITIALIZING,   ///< State during parsing the file
            READY           ///< State after the file has been parsed and information extracted
        };
        State mState;  ///< Running state of the reader API implementation

        struct StreamIO
        {
            UniquePtr<InternalStream> stream;
            UniquePtr<StreamInterface> fileStream;  // only used when reading from files
            std::int64_t size = 0;
        };
        StreamIO mIo;


        /// The File Properties object contains all information extracted from the read file.
        FileInformationInternal mFileProperties;

        /* ********************************************************************** */
        /* ****************** Common for meta and track content ***************** */
        /* ********************************************************************** */


        Map<Id, FourCCInt> mDecoderCodeTypeMap;     ///< Extracted decoder code types for each sample and image
        Map<Id, ParameterSetMap> mParameterSetMap;  ///< Extracted decoder parameter sets
        Map<Id, Id> mImageToParameterSetMap;        ///< Map from every sample and image item to parameter set map entry

        /// Context type classification
        enum class ContextType
        {
            META,
            TRACK,
            FILE,
            INVALID
        };

        bool mIsPrimaryItemSet;  ///< True if Primary Item Box is present.
        ImageId mPrimaryItemId;  ///< ID of the primary item.

        FileTypeBox mFtyp;  ///< File Type Box for later information retrieval

        /** @returns ErrorCode=[UNINITIALIZED] if input file has not been read yet */
        ErrorCode isInitialized() const;

        /** Reset reader internal state */
        void reset();

        /** Parse input stream, fill mFileProperties and implementation internal data structures. */
        ErrorCode readStream();

        FileFeature getFileFeatures() const;

        FileInformation mFileInformation;  ///< File information extracted during initialize().

        ErrorCode readBoxParameters(String& boxType, std::int64_t& boxSize);
        ErrorCode readBox(BitStream& bitstream);
        ErrorCode skipBox();

        ErrorCode getItemLength(const MetaBox& metaBox,
                                const ItemId& itemId,
                                std::uint64_t& itemLength,
                                List<ItemId>& pastReferences) const;


        /**
         * @brief Read bytes from stream to an integer value.
         * @param count  Number of bytes to read (0-8).
         * @param [out] result Read value.
         * @return Error code OK or FILE_READ_ERROR
         */
        ErrorCode readBytes(unsigned int count, std::int64_t& result);

        /**
         * @brief Seek to the desired offset in the input stream (counting from the beginning).
         *        Eliminates redundant seeks (and buffer discarding).
         * @param pos The offset to seek to.
         */
        void seekInput(std::int64_t pos) const;

        /** Move decoder configuration parameter data to ParamSetMap
         * @param record decoder configuration
         * @return Decoder parameters */
        ParameterSetMap makeDecoderParameterSetMap(const DecoderConfigurationRecord& record) const;

        /**
         * Get ids of all items of a image collection.
         * @param [out] items Ids of all items in the image collection.
         * @return NO_ERROR. */
        void getCollectionItems(IdVector& items) const;

        /**
         * Get ids of all items of an image sequence.
         * @param [out] items Ids of all items of the image sequence.
         * @return INVALID_SEQUENCE_ID or NO_ERROR. */
        ErrorCode getSequenceItems(SequenceId sequenceId, IdVector& items) const;

        /**
         * @brief Check metabox item protection status from ItemInfoEntry item_protection_index
         * @param [in]  itemId      ID of the item
         * @param [out] isProtected True if the item is protected. False otherwise.
         * @return ErrorCode: OK, INVALID_ITEM_ID */
        ErrorCode getProtection(std::uint32_t itemId, bool& isProtected) const;

        /** Process item data from AVC bitstream
         *  @param [in] data  char pointer to Raw AVC bitstream data.
         *  @param [in] size  size of data to be modified.
         *  @pre initialize() has been called successfully.
         *  @return ErrorCode: OK, FILE_READ_ERROR */
        ErrorCode processAvcItemData(uint8_t* data, uint64_t& memoryBufferSize) const;

        /** Process item data from HEVC bitstream
         *  @param [in] data  char pointer to Raw HEVC bitstream data.
         *  @param [in] size  size of data to be modified.
         *  @pre initialize() has been called successfully.
         *  @return ErrorCode: OK, FILE_READ_ERROR */
        ErrorCode processHevcItemData(uint8_t* data, uint64_t& memoryBufferSize) const;

        /* ********************************************************************** */
        /* *********************** Meta-specific section  *********************** */
        /* ********************************************************************** */

        Map<ContextId, MetaBox> mMetaBoxMap;  ///< Map of read MetaBoxes

        typedef Map<ItemId, PropertyTypeVector> Properties;  ///< Convenience type for mapping item properties

        /// For non-image items (Exif or XML metadata)
        struct ItemInfo
        {
            FourCCInt type;  ///< Item type from Item Information box
            String name;
            String contentType;
            String contentEncoding;

            // in case of image items, some further information:
            std::uint32_t width  = 0;  ///< Width of the image from ispe property
            std::uint32_t height = 0;  ///< Height of the image from ispe property
            uint64_t displayTime = 0;  ///< Display timestamp generated by the reader
        };
        typedef Map<ItemId, ItemInfo> ItemInfoMap;

        /// Reader internal information about each MetaBox
        struct MetaBoxInfo
        {
            unsigned int displayableMasterImages = 0;  ///< Number of master images
            ItemInfoMap itemInfoMap;                   ///< Information of other items
            Map<ItemId, Grid> gridItems;               ///< Data of image grid items
            Map<ItemId, Overlay> iovlItems;            ///< Data of image overlay items
            Properties properties;                     ///< Property information of each item
        };
        Map<ContextId, MetaBoxInfo> mMetaBoxInfo;  ///< MetaBoxInfo struct for each MetaBox context

        /**
         * @brief Extract information about non-image items of a MetaBox.
         * @param metaBox The MetaBox items of which to handle.
         * @return ItemInfoMap including information about non image type items. */
        ItemInfoMap extractItemInfoMap(const MetaBox& metaBox) const;

        /**
         * @brief Create a MetaBoxProperties struct for the reader interface
         * @param metaBox MetaBox to get information from
         * @return Filled MetaBoxProperties */
        MetaBoxProperties extractMetaBoxProperties(const MetaBox& metaBox) const;

        /**
         * @brief Create a MetaBoxFeature struct for the reader interface
         * @param imageFeatures Previously extracted ImageFeaturesMap
         * @param groupings     Previously extracted groupings
         * @return Filled MetaBoxProperties */
        MetaBoxFeature extractMetaBoxFeatures(const ItemFeaturesMap& imageFeatures, const Groupings& groupings) const;

        /**
         * @brief Create a GroupingMap struct for the reader interface
         * @param metaBox MetaBox to get information from
         * @return Filled GroupingMap */
        Groupings extractMetaBoxEntityToGroupMaps(const MetaBox& metaBox) const;

        /**
         * @brief Create a ItemFeaturesMap struct for the reader interface
         * @param metaBox MetaBox to get information from
         * @return Filled ItemFeaturesMap */
        ItemFeaturesMap extractMetaBoxItemPropertiesMap(const MetaBox& metaBox) const;

        /**
         * @brief Extract item properties for reader internal use
         * @param contextId Meta context ID
         * @pre Filled mMetaBoxMap
         * @return Filled Properties */
        Properties processItemProperties(ContextId contextId) const;

        /**
         * @brief Fill mImageToParameterSetMap and mParameterSetMap
         * @param contextId Meta context ID
         * @pre Filled mMetaBoxMap, filled mFileProperties.rootLevelMetaBoxProperties.imageFeaturesMap. */
        void processDecoderConfigProperties(ContextId contextId);

        /**
         * @brief Extract MetaBoxInfo struct for reader internal use
         * @param metaBox   MetaBox to get information from
         * @param contextId Meta context ID
         * @pre Filled mMetaBoxMap
         * @return Filled MetaBoxInfo */
        MetaBoxInfo extractItems(const MetaBox& metaBox, std::uint32_t contextId) const;

        /**
         * @brief Load item data.
         * @param metaBox The MetaBox where the item is located
         * @param itemId  ID of the item
         * @param [out] data Item Data
         * @pre mInputStream is good
         * @return ErrorCode: OK, INVALID_ITEM_ID, FILE_READ_ERROR */
        ErrorCode loadItemData(const MetaBox& metaBox, const ItemId itemId, DataVector& data) const;

        /**
         * @brief Load item data.
         * @param metaBox The MetaBox where the item is located
         * @param itemId  ID of the item
         * @param [out] data Item Data
         * @pre mInputStream is good
         * @return ErrorCode: OK, INVALID_ITEM_ID, FILE_READ_ERROR */
        ErrorCode readItem(const MetaBox& metaBox, const ItemId itemId, uint8_t* memorybuffer, uint64_t maxSize) const;

        /**
         * @brief Convert information extracted from the MetaBox to fixed-sized arrays for public API.
         * @return Filled MetaBoxInformation struct.
         */
        MetaBoxInformation convertRootMetaBoxInformation(const MetaBoxProperties& metaboxProperties) const;

        /* *********************************************************************** */
        /* *********************** Track-specific section  *********************** */
        /* *********************************************************************** */

        typedef std::uint32_t SampleDescriptionIndex;

        Vector<std::int32_t> mMatrix;  ///< Video transformation matrix from the Movie Header Box

        /// Reader internal information about each sample.
        struct SampleInfo
        {
            std::uint32_t decodingOrder = 0;  ///< Decoding order/sequence number of the sample
            Vector<std::int64_t>
                compositionTimes;          ///< Timestamps of the sample. Possible edit list is considered here.
            std::uint64_t dataOffset = 0;  ///< File offset of sample data in bytes
            std::uint32_t dataLength = 0;  ///< Length of same in bytes
            std::uint32_t width      = 0;  ///< Width of the frame
            std::uint32_t height     = 0;  ///< Height of the frame
            IdVector decodeDependencies;   ///< Direct decoding dependencies
        };
        typedef Vector<SampleInfo> SampleInfoVector;

        struct TrackInfo
        {
            SampleInfoVector samples;  ///< Information about each sample in the TrackBox
            std::uint32_t width;       ///< display width in pixels, from 16.16 fixed point in TrackHeaderBox
            std::uint32_t height;      ///< display height in pixels, from 16.16 fixed point in TrackHeaderBox
            Vector<int32_t> matrix;    ///< transformation matrix of the track (from track header box)
            double duration;           ///< Track duration in seconds, from TrackHeaderBox
            DecodePts::PMap pMap;      ///< Display timestamps, from edit list
            Map<SampleDescriptionIndex, CleanAperture>
                clapProperties;  ///< Clean aperture data from sample description entries
            Map<SampleDescriptionIndex, AuxiliaryType>
                auxiProperties;  ///< Clean aperture data from sample description entries
            double repetitions;
        };
        Map<SequenceId, TrackInfo> mTrackInfo;  ///< Reader internal information about each TrackBox

        /**
         * @param sequenceId Track ID to  check.
         * @return OK if sequenceId is a valid track ID in this file, INVALID_SEQUENCE_ID if not.
         *         UNINITIALIZED if initialize() had not been called.
         */
        ErrorCode isValidTrack(const SequenceId& sequenceId) const;

        /**
         * @param sequenceId      ID of the track containing the sample.
         * @param sequenceImageId ID of the sample/image.
         * @return OK if the image/sample is a valid sample on the sequenceId given.  INVALID_SEQUENCE_IMAGE_ID or
         * INVALID_SEQUENCE_ID otherwise. UNINITIALIZED if initialize() had not been called.
         */
        ErrorCode isValidSample(const SequenceId& sequenceId, const SequenceImageId& sequenceImageId) const;

        /**
         * @param imageId ImageId to check.
         * @return OK if itemId is a valid image item in the root-level meta box, INVALID_ITEM_ID if not.
         *         UNINITIALIZED if initialize() had not been called.
         */
        ErrorCode isValidImageItem(const ImageId& imageId) const;

        /**
         * @param itemId ImageId to check.
         * @return OK if itemId is a valid item in the root-level meta box, INVALID_ITEM_ID if not.
         *         UNINITIALIZED if initialize() had not been called.
         */
        ErrorCode isValidItem(const ImageId& itemId) const;

        /**
         * @brief Create a TrackPropertiesMap struct for the reader interface
         * @param [in] moovBox MovieBox to extract properties from
         * @return Filled TrackPropertiesMap */
        TrackPropertiesMap fillTrackProperties(const MovieBox& moovBox);

        /**
         * @brief Create a MoovProperties struct for the reader interface
         * @param [in] moovBox MovieBox to extract properties from
         * @return Filled MoovProperties */
        MoovProperties extractMoovProperties(const MovieBox& moovBox) const;  // TODO: Not-Implemented

        /**
         * @brief Fill mParameterSetMap and mTrackInfo clapProperties entries for a a TrackBox
         * @param [in] trackBox TrackBox to extract data from */
        void fillSampleEntryMap(const TrackBox* trackBox);

        /**
         * @brief Find if there is certain type reference or references from tracks to a track.
         * @param trackPropertiesMap TrackPropertiesMap containing information about all tracks of the file
         * @param trackId            ID of the TrackBox to look references to
         * @param referenceType      4CC reference type to look for
         * @return True if a reference was found */
        bool isAnyLinkedToWithType(const TrackPropertiesMap& trackPropertiesMap,
                                   SequenceId trackId,
                                   FourCCInt referenceType) const;

        /**
         * @brief Get direct decoding dependencies for a sample
         * @param itemId           Item ID of the sample
         * @param sgpd             SampleGroupDescriptionBox of the TrackBox
         * @param sampleToGroupBox SampleToGroupBox of the TrackBox
         * @return Item IDs of decoding dependencies */
        Vector<ItemId> getSampleDirectDependencies(ItemId itemId,
                                                   const SampleGroupDescriptionBox* sgpd,
                                                   const SampleToGroupBox& sampleToGroupBox) const;

        /**
         * @brief Create a TrackFeature struct for the reader interface
         * @param [in] trackBox TrackBox to extract data from
         * @return Filled TrackFeature */
        TrackFeature getTrackFeatures(const TrackBox* trackBox) const;

        /**
         * @brief Extract information about reference tracks for the reader interface
         * @param [in] trackBox TrackBox to extract data from
         * @return Reference track information */
        TypeToIdsMap getReferenceTrackIds(const TrackBox* trackBox) const;

        /**
         * @brief Extract information about reference tracks for the reader interface
         * @param [in] trackBox TrackBox to extract data from
         * @return Reference track information */
        EditList getEditList(const TrackBox* trackBox, const double repetitions) const;

        /**
         * @brief Extract information about sample groups for the reader interface
         * @param [in] trackBox TrackBox to extract data from
         * @return Sample grouping information */
        Array<SampleGrouping> getSampleGroupings(const TrackBox* trackBox) const;

        /**
         * @brief Extract information about sample VisualEquivalenceEntry() groups ('eqiv') for the reader interface.
         * @param [in] trackBox TrackBox to extract data from.
         * @return Sample equivalency information. */
        Array<SampleVisualEquivalence> getEquivalenceGroups(const TrackBox* trackBox) const;

        /**
         * @brief Extract information about sample SampleToMetadataItemEntry() groups ('stmi') for the reader interface.
         * @param [in] trackBox TrackBox to extract data from.
         * @return Sample to Metadata information. */
        Array<SampleToMetadataItem> getSampleToMetadataItemGroups(const TrackBox* trackBox) const;

        /**
         * @brief Extract information about sample DirectReferenceSamplesList() groups ('refs') for the reader
         * interface.
         * @param [in] trackBox TrackBox to extract data from.
         * @return Sample to Metadata information. */
        Array<DirectReferenceSamples> getDirectReferenceSamplesGroups(const TrackBox* trackBox) const;

        /**
         * @brief Extract information about alternate track IDs for a track for the reader interface
         * @param [in] trackBox TrackBox to look alternate track IDs for
         * @param [in] moovBox  MovieBox containing the TrackBox
         * @return IDs of alternate tracks. An empty vector if there was no alternates */
        IdVector getAlternateTrackIds(const TrackBox* trackBox, const MovieBox& moovBox) const;

        /**
         * @brief Extract reader internal TrackInfo structure from TrackBox
         * @param [in] trackBox TrackBox to extract data from
         * @param [in] moovBox  MovieBox containing the TrackBox
         * @return Filled TrackInfo struct */
        TrackInfo extractTrackInfo(const TrackBox* trackBox, const MovieBox& moovBox) const;

        /**
         * @brief Extract reader internal information about samples
         * @param trackBox [in] trackBox TrackBox to extract data from
         * @param pMap Presentation map for the track
         * @param maxSampleSize max size of samples for track.
         * @return SampleInfoVector containing information about every sample of the track */
        SampleInfoVector makeSampleInfoVector(const TrackBox* trackBox,
                                              const DecodePts::PMap& pMap,
                                              std::uint64_t& maxSampleSize) const;

        /**
         * @brief Extract information about samples for the reader interface
         * @param trackBox [in] trackBox TrackBox to extract data from
         * @return Filled SamplePropertiesMap */
        SamplePropertiesMap makeSamplePropertiesMap(const TrackBox* trackBox);

        /**
         * @brief Add sample decoding dependencies
         * @param sequenceId        ID of the track
         * @param itemDecodingOrder Items/samples to get decoding dependencies for
         * @return DecodingOrderVector with decoding dependencies */
        Vector<TimestampIDPair> addDecodingDependencies(SequenceId sequenceId,
                                                        const Vector<TimestampIDPair>& itemDecodingOrder) const;

        /**
         * @brief Load frame data for a sample/item/frame from the input stream
         * @param [in]  frameIndex 0-based index of the sample (/item ID)
         * @param [in]  trackInfo  TrackInfo containing the sample
         * @param [in]  memorybuffer  memory buffer pointer to write data to
         * @param [in/out]  memorybuffersize   memory buffer size with written data (or required size if too small).
         * @return ErrorCode: OK, FILE_READ_ERROR, INVALID_ITEM_ID */
        ErrorCode getTrackFrameData(unsigned int frameIndex,
                                    const TrackInfo& trackInfo,
                                    uint8_t* memorybuffer,
                                    uint64_t& memorybuffersize) const;

        /**
         * @brief convertTrackInformation Convert information extracted from the MovieBox to fixed-size arrays for the
         * public API.
         * @param trackPropertiesMap Data extracted from the MovieBox during initialization.
         * @return TrackInformation structs for track in the file.
         */
        Array<TrackInformation> convertTrackInformation(const TrackPropertiesMap& trackPropertiesMap) const;

        class FileReaderException : public Exception
        {
        public:
            FileReaderException(ErrorCode status, const char* message = nullptr)
                : Exception(message)
                , mErrorCode(status)
            {
            }

            const char* what() const throw()
            {
                const char* msgptr = Exception::what();
                if (msgptr != nullptr)
                {
                    return msgptr;
                }

                switch (mErrorCode)
                {
                case ErrorCode::FILE_OPEN_ERROR:
                    return "Unable to open input file.";
                case ErrorCode::FILE_HEADER_ERROR:
                    return "Error while reading file header.";
                case ErrorCode::FILE_READ_ERROR:
                    return "Error while reading file.";
                case ErrorCode::UNSUPPORTED_CODE_TYPE:
                    return "Unsupported code type.";
                case ErrorCode::INVALID_FUNCTION_PARAMETER:
                    return "Invalid API method parameter.";
                case ErrorCode::INVALID_CONTEXT_ID:
                    return "Invalid context ID parameter.";
                case ErrorCode::INVALID_ITEM_ID:
                    return "Invalid item ID parameter.";
                case ErrorCode::INVALID_PROPERTY_INDEX:
                    return "Invalid item property index.";
                case ErrorCode::INVALID_SAMPLE_DESCRIPTION_INDEX:
                    return "Invalid sample description index.";
                case ErrorCode::PROTECTED_ITEM:
                    return "The item is unaccessible because it is protected.";
                case ErrorCode::UNPROTECTED_ITEM:
                    return "The item is not protected.";
                case ErrorCode::UNINITIALIZED:
                    return "Reader not initialized. Call initialize() first.";
                case ErrorCode::NOT_APPLICABLE:
                default:
                    return "Unspecified error.";
                }
            }

            ErrorCode getErrorCode() const
            {
                return mErrorCode;
            }

        private:
            ErrorCode mErrorCode;
        };
    };

    /* *********************************************************************** */
    /* ************************* Helper functions **************************** */
    /* *********************************************************************** */

    /**
     * Helper function to get item type from  the Item Information Box inside a MetaBox
     * @param [in]  metaBox Pointer to the MetaBox
     * @param [in]  itemId  ID of the item
     * @param [out] type    type name
     * @return ErrorCode: OK, INVALID_ITEM_ID */
    ErrorCode getRawItemType(const MetaBox& metaBox, uint32_t itemId, FourCCInt& type);

    /**
     * @brief Convert CleanAperture to Reader::CleanAperture
     * @param clapBox Pointer to input CleanAperture
     * @return A CleanAperture filled with data from CleanAperture */
    CleanAperture makeClap(const CleanApertureBox* clapBox);

    /**
     * @brief Convert AuxiliaryTypeInfoBox to Reader::AuxProperty
     * @param auxiBox Pointer to input AuxiliaryTypeInfoBox
     * @return A AuxProperty filled with data from AuxiliaryTypeInfoBox */
    AuxiliaryType makeAuxi(const AuxiliaryTypeInfoBox* auxiBox);

    /**
     * @brief Recognize image item types ( "avc1", "hvc1", "grid", "iovl", "iden", "jpeg").
     * @param type 4CC Item type from Item Info Entry
     * @return True if the 4CC is an image type */
    bool isImageItemType(const FourCCInt& type);

    /**
     * @brief Check if one or more references from an item exists.
     * @param metaBox       MetaBox containing the item and possible references
     * @param itemId        ID of the item to search references for
     * @param referenceType 4CC reference type
     * @return True if one or more referenceType references from the item exists. */
    bool doReferencesFromItemIdExist(const MetaBox& metaBox, uint32_t itemId, const FourCCInt& referenceType);

    /**
     * @brief Check if one or more references to an item exists.
     * @param metaBox       MetaBox containing the item and possible references
     * @param itemId        ID of the item to search references for
     * @param referenceType 4CC reference type
     * @return True if one or more referenceType references to the item exists. */
    bool doReferencesToItemIdExist(const MetaBox& metaBox, uint32_t itemId, const FourCCInt& referenceType);

    /**
     * Template for making Array for the public API from a container.
     */
    template <typename T, typename Container>
    Array<T> makeArray(const Container& container);
}  // namespace HEIF

#endif /* HEIFREADERIMPL_HPP */
