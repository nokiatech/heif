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

#ifndef HEIFREADERIMPL_HPP
#define HEIFREADERIMPL_HPP

#include "decodepts.hpp"
#include "extendedtypebox.hpp"
#include "filetypebox.hpp"
#include "heiffiledatatypesinternal.hpp"
#include "heifreader.h"
#include "heifreadersegment.hpp"
#include "heifstreamgeneric.hpp"
#include "heifstreaminternal.hpp"
#include "imagespatialextentsproperty.hpp"
#include "metabox.hpp"
#include "moviebox.hpp"
#include "moviefragmentbox.hpp"
#include "segmentindexbox.hpp"

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
        ~HeifReaderImpl() override = default;

        /// @see Reader::initialize()
        ErrorCode initialize(const char* fileName) override;

        /// @see Reader::initialize()
        ErrorCode initialize(StreamInterface* stream) override;

        /// @see Reader::close()
        void close() override;

        /// @see Reader::getMajorBrand()
        ErrorCode getMajorBrand(FourCC& majorBrand) const override;

        /// @see Reader::getMinorVersion()
        ErrorCode getMinorVersion(uint32_t& minorVersion) const override;

        /// @see Reader::getCompatibleBrands()
        ErrorCode getCompatibleBrands(Array<FourCC>& compatibleBrands) const override;

        /// @see Reader::getCompatibleBrandCombinations()
        ErrorCode getCompatibleBrandCombinations(Array<Array<FourCC>>& compatibleBrandCombinations) const override;

        /// @see Reader::getFileInformation()
        ErrorCode getFileInformation(FileInformation& fileinfo) const override;

        /// @see Reader::getDisplayWidth()
        ErrorCode getDisplayWidth(const SequenceId& sequenceId, uint32_t& displayWidth) const override;

        /// @see Reader::getDisplayHeight()
        ErrorCode getDisplayHeight(const SequenceId& sequenceId, uint32_t& displayHeight) const override;

        /// @see Reader::getWidth()
        ErrorCode getWidth(const ImageId& itemId, uint32_t& width) const override;
        ErrorCode getWidth(const SequenceId& sequenceId, const SequenceImageId& itemId, uint32_t& width) const override;

        /// @see Reader::getHeight()
        ErrorCode getHeight(const ImageId& itemId, uint32_t& height) const override;
        ErrorCode getHeight(const SequenceId& sequenceId,
                            const SequenceImageId& itemId,
                            uint32_t& height) const override;

        /// @see Reader::getMatrix()
        ErrorCode getMatrix(Array<std::int32_t>& matrix) const override;

        /// @see Reader::getMatrix()
        ErrorCode getMatrix(const SequenceId& sequenceId, Array<int32_t>& matrix) const override;

        /// @see Reader::getPlaybackDurationInSecs()
        ErrorCode getPlaybackDurationInSecs(const SequenceId& sequenceId, double& durationInSecs) const override;

        /// @see Reader::getMasterImages()
        ErrorCode getMasterImages(Array<ImageId>& itemIds) const override;
        ErrorCode getMasterImages(const SequenceId& sequenceId, Array<SequenceImageId>& itemIds) const override;

        /// @see Reader::getItemListByType()
        ErrorCode getItemListByType(const FourCC& itemType, Array<ImageId>& itemIds) const override;

        /// @see Reader::getItemListByType()
        ErrorCode getItemListByType(const SequenceId& sequenceId,
                                    const TrackSampleType& sampleType,
                                    Array<SequenceImageId>& sampleIdsApi) const override;

        /// @see Reader::getItemType()
        ErrorCode getItemType(const ImageId& itemId, FourCC& type) const override;

        /// @see Reader::getItemType()
        ErrorCode getItemType(const SequenceId& sequenceId,
                              const SequenceImageId& sequenceImageId,
                              FourCC& type) const override;

        /// @see Reader::getReferencedFromItemListByType()
        ErrorCode getReferencedFromItemListByType(const ImageId& id,
                                                  const FourCC& referenceType,
                                                  Array<ImageId>& itemIds) const override;

        /// @see Reader::getReferencedToItemListByType()
        ErrorCode getReferencedToItemListByType(const ImageId& toItemId,
                                                const FourCC& referenceType,
                                                Array<ImageId>& itemIds) const override;

        /// @see Reader::getPrimaryItem()
        ErrorCode getPrimaryItem(ImageId& itemId) const override;

        /// @see Reader::getItemData()
        ErrorCode getItemData(const ImageId& itemId,
                              uint8_t* memoryBuffer,
                              uint64_t& memoryBufferSize,
                              bool bytestreamHeaders = true) const override;

        /// @see Reader::getItemData()
        ErrorCode getItemData(const SequenceId& sequenceId,
                              const SequenceImageId& itemId,
                              uint8_t* memoryBuffer,
                              uint64_t& memoryBufferSize,
                              bool bytestreamHeaders = true) override;

        /// @see Reader::getItem()
        ErrorCode getItem(const ImageId& itemId, Overlay& iovlItem) const override;

        /// @see Reader::getItem()
        ErrorCode getItem(const ImageId& itemId, Grid& gridItem) const override;

        /// @see Reader::getProperty()
        ErrorCode getProperty(const PropertyId& index, RequiredReferenceTypes& rref) const override;

        /// @see Reader::getProperty()
        ErrorCode getProperty(const PropertyId& index, UserDescription& udes) const override;

        /// @see Reader::getProperty()
        ErrorCode getProperty(const PropertyId& index, Mirror& imir) const override;

        /// @see Reader::getProperty()
        ErrorCode getProperty(const PropertyId& index, Rotate& irot) const override;

        /// @see Reader::getProperty()
        ErrorCode getProperty(const PropertyId& index, Scale& iscl) const override;

        /// @see Reader::getProperty()
        ErrorCode getProperty(const PropertyId& index, AuxiliaryType& auxC) const override;

        /// @see Reader::getProperty()
        ErrorCode getProperty(const SequenceId& sequenceId,
                              const std::uint32_t index,
                              AuxiliaryType& auxC) const override;

        /// @see Reader::getProperty()
        ErrorCode getProperty(const PropertyId& index, RelativeLocation& rloc) const override;

        /// @see Reader::getProperty()
        ErrorCode getProperty(const PropertyId& index, PixelInformation& pixi) const override;

        /// @see Reader::getProperty()
        ErrorCode getProperty(const PropertyId& index, PixelAspectRatio& pasp) const override;

        /// @see Reader::getProperty()
        ErrorCode getProperty(const PropertyId& index, ColourInformation& colr) const override;

        /// @see Reader::getProperty()
        ErrorCode getProperty(const PropertyId& index, CleanAperture& clap) const override;

        /// @see Reader::getProperty()
        ErrorCode getProperty(const PropertyId& index, CreationTimeInformation& crtt) const override;

        /// @see Reader::getProperty()
        ErrorCode getProperty(const PropertyId& index, ModificationTimeInformation& mdft) const override;

        /// @see Reader::getProperty()
        ErrorCode getProperty(const PropertyId& index, AccessibilityText& altt) const override;

        /// @see Reader::getProperty()
        ErrorCode getProperty(const SequenceId& sequenceId,
                              const std::uint32_t index,
                              CleanAperture& clap) const override;

        /// @see Reader::getItemProperties()
        ErrorCode getItemProperties(const ImageId& itemId, Array<ItemPropertyInfo>& propertyTypes) const override;

        /// @see Reader::getGroupProperties()
        ErrorCode getItemProperties(const GroupId& groupId, Array<ItemPropertyInfo>& propertyTypes) const override;

        /// @see Reader::getItemProperties()
        ErrorCode getProperty(const PropertyId& index, RawProperty& property) const override;

        /// @see Reader::getItemDataWithDecoderParameters()
        ErrorCode getItemDataWithDecoderParameters(const ImageId& itemId,
                                                   uint8_t* memoryBuffer,
                                                   uint64_t& memoryBufferSize) const override;

        /// @see Reader::getItemDataWithDecoderParameters()
        ErrorCode getItemDataWithDecoderParameters(const SequenceId& sequenceId,
                                                   const SequenceImageId& itemId,
                                                   uint8_t* memoryBuffer,
                                                   uint64_t& memoryBufferSize) override;

        /// @see Reader::getItemProtectionScheme()
        ErrorCode getItemProtectionScheme(const ImageId& itemId,
                                          uint8_t* memoryBuffer,
                                          uint64_t& memoryBufferSize) const override;

        /// @see Reader::getItemTimestamps()
        ErrorCode getItemTimestamps(const SequenceId& sequenceId, Array<TimestampIDPair>& timestamps) const override;

        /// @see Reader::getTimestampsOfItem()
        ErrorCode getTimestampsOfItem(const SequenceId& sequenceId,
                                      const SequenceImageId& itemId,
                                      Array<int64_t>& timestamps) const override;

        /// @see Reader::getItemsInDecodingOrder()
        ErrorCode getItemsInDecodingOrder(const SequenceId& sequenceId,
                                          Array<TimestampIDPair>& decodingOrder) const override;

        /// @see Reader::getDecodeDependencies()
        ErrorCode getDecodeDependencies(const SequenceId& sequenceId,
                                        const SequenceImageId& itemId,
                                        Array<SequenceImageId>& dependencies) const override;

        /// @see Reader::getDecodeDependencies()
        ErrorCode getDecodeDependencies(const ImageId& imageId, Array<ImageId>& dependencies) const override;

        /// @see Reader::getDecoderCodeType()
        ErrorCode getDecoderCodeType(const ImageId& itemId, FourCC& type) const override;

        /// @see Reader::getDecoderCodeType()
        ErrorCode getDecoderCodeType(const SequenceId& trackId,
                                     const SequenceImageId& sampleId,
                                     FourCC& type) const override;

        /// @see Reader::getDecoderParameterSets()
        ErrorCode getDecoderParameterSets(const ImageId& itemId, DecoderConfiguration& decoderInfos) const override;

        /// @see Reader::getDecoderParameterSets()
        ErrorCode getDecoderParameterSets(const SequenceId& sequenceId,
                                          const SequenceImageId& itemId,
                                          DecoderConfiguration& decoderInfos) const override;

        /// @see Reader::getTrackInformations()
        ErrorCode getTrackInformations(Array<TrackInformation>& trackInfos) const override;

        // segment handling
    public:
        ErrorCode parseInitializationSegment(StreamInterface* streamInterface) override;
        ErrorCode parseSegment(StreamInterface* streamInterface,
                               SegmentId segmentId,
                               uint64_t earliestPTSinTS = UINT64_MAX) override;
        ErrorCode invalidateSegment(SegmentId segmentId) override;
        ErrorCode getSegmentIndex(Array<SegmentInformation>& segmentIndex) override;
        ErrorCode parseSegmentIndex(StreamInterface* streamInterface, Array<SegmentInformation>& segmentIndex) override;

    private:
        enum class State
        {
            UNINITIALIZED,  ///< State before starting to read file and after closing it
            INITIALIZING,   ///< State during parsing the file
            READY           ///< State after the file has been parsed and information extracted
        };
        State mState;  ///< Running state of the reader API implementation

        StreamIO mFileStream;  ///< File IO stream

        /// The File Properties object contains all information extracted from the read file.
        FileInformationInternal mFileProperties;

        /* ********************************************************************** */
        /* ************************ Segment handling **************************** */
        /* ********************************************************************** */

        friend class Segments;
        friend class ConstSegments;

        Segments segmentsBySequence();
        ConstSegments segmentsBySequence() const;

        ErrorCode handleInitSegmentMoof(StreamIO& io, SegmentId segmentId);
        ErrorCode handleSegmentMoof(StreamIO& io,
                                    SegmentId segmentId,
                                    bool& earliestPTSRead,
                                    Map<SequenceId, DecodePts::PresentationTimeTS>& earliestPTSTS,
                                    uint64_t& earliestPTSinTS);


        /* ********************************************************************** */
        /* ****************** Common for meta and track content ***************** */
        /* ********************************************************************** */

        bool mIsPrimaryItemSet;  ///< True if Primary Item Box is present.
        ImageId mPrimaryItemId;  ///< ID of the primary item.

        FileTypeBox mFtyp;      ///< File Type Box for later information retrieval
        ExtendedTypeBox mEtyp;  ///< Extended Type Box (optional) for later information retrieval

        /** @returns ErrorCode=[UNINITIALIZED] if input file has not been read yet */
        ErrorCode isInitialized() const;

        /**
         * Create file information struct for the API.
         * @param internalFileInfo Reader internal information about the file.
         * @return FileInformation struct for the public API.
         */
        FileInformation makeFileInformation(const FileInformationInternal& internalFileInfo) const;

        /** Reset reader internal state */
        void reset();

        /** Parse input stream, fill mFileProperties and implementation internal data structures. */
        ErrorCode readStream();

        FileFeature getFileFeatures() const;

        FileInformation mFileInformation;  ///< File information extracted during initialize().

        static ErrorCode readBoxParameters(StreamIO& io, String& boxType, std::int64_t& boxSize);
        ErrorCode readBox(StreamIO& io, BitStream& bitstream);
        ErrorCode skipBox(StreamIO& io);

        ErrorCode handleFtyp(StreamIO& io);
        ErrorCode handleEtyp(StreamIO& io);
        ErrorCode handleMeta(StreamIO& io);
        ErrorCode handleMoov(StreamIO& io);


        ErrorCode getItemLength(const MetaBox& metaBox,
                                const ImageId& itemId,
                                std::uint64_t& itemLength,
                                List<ImageId>& pastReferences) const;

        /**
         * @brief Read bytes from stream to an integer value.
         * @param io           Segment IO stream to read from.
         * @param count        Number of bytes to read (0-8).
         * @param [out] result Read value.
         * @return Error code OK or FILE_READ_ERROR
         */
        static ErrorCode readBytes(StreamIO& io, unsigned int count, std::int64_t& result);

        /**
         * @brief Seek to the desired offset in the input stream (counting from the beginning).
         *        Eliminates redundant seeks (and buffer discarding).
         * @param pos The offset to seek to.
         */
        static void seekInput(StreamIO& io, std::int64_t pos);

        /** Move decoder configuration parameter data to ParamSetMap
         * @param record decoder configuration
         * @return Decoder parameters */
        static ParameterSetMap makeDecoderParameterSetMap(const DecoderConfigurationRecord& record);

        /**
         * Get ids of all items of a image collection.
         * @param [out] items Ids of all items in the image collection.
         * @return NO_ERROR. */
        void getCollectionItems(Vector<ImageId>& items) const;

        /**
         * Get ids of all items of an image sequence.
         * @param [out] images Ids of all items of the image sequence.
         * @return INVALID_SEQUENCE_ID or NO_ERROR. */
        ErrorCode getSamples(SequenceId sequenceId, Vector<SequenceImageId>& images) const;

        /**
         * @brief Check metabox item protection status from ItemInfoEntry item_protection_index
         * @param [in]  itemId      ID of the item
         * @param [out] isProtected True if the item is protected. False otherwise.
         * @return ErrorCode: OK, INVALID_ITEM_ID */
        ErrorCode getProtection(ImageId itemId, bool& isProtected) const;

        /** Process item data from AVC bitstream
         *  @param [in] data  char pointer to Raw AVC bitstream data.
         *  @param [in] size  size of data to be modified.
         *  @pre initialize() has been called successfully.
         *  @return ErrorCode: OK, FILE_READ_ERROR */
        static ErrorCode processAvcItemData(uint8_t* memoryBuffer, uint64_t& memoryBufferSize);

        /** Process item data from HEVC bitstream
         *  @param [in] data  char pointer to Raw HEVC bitstream data.
         *  @param [in] size  size of data to be modified.
         *  @pre initialize() has been called successfully.
         *  @return ErrorCode: OK, FILE_READ_ERROR */
        static ErrorCode processHevcItemData(uint8_t* memoryBuffer, uint64_t& memoryBufferSize);

        /* ********************************************************************** */
        /* *********************** Meta-specific section  *********************** */
        /* ********************************************************************** */

        Map<ImageId, FourCCInt> mImageItemCodeTypeMap;  ///< Extracted decoder code types for each image item
        Map<DecoderConfigId, ParameterSetMap> mImageItemParameterSetMap;  ///< Extracted decoder parameter sets
        Map<ImageId, DecoderConfigId> mImageToParameterSetMap;  ///< Map from image item to parameter set map entry

        MetaBox mMetaBox;  ///< Root-level MetaBox for later information retrieval
        bool mMetaBoxLoaded;

        typedef Map<uint32_t, PropertyTypeVector> Properties;  ///< Convenience type for mapping properties

        /// For non-image items (Exif or XML metadata)
        struct ItemInfo
        {
            FourCCInt type;  ///< Item type from Item Information box
            String name;
            String contentType;
            String contentEncoding;

            // Further information for image items:
            std::uint32_t width  = 0;  ///< Width of the image from ispe property
            std::uint32_t height = 0;  ///< Height of the image from ispe property
            uint64_t displayTime = 0;  ///< Display timestamp generated by the reader
        };
        typedef Map<ImageId, ItemInfo> ItemInfoMap;

        /**
         * @brief Recognize image item types ( "avc1", "hvc1", "grid", "iovl", "iden", "jpeg")
         *        as well as mime content type "image/jpeg"
         * @param ItemInfo for the item
         * @return True if the 4CC is an image type or if it's a mime type and content type "image/jpeg" */
        static bool isImageItem(const ItemInfo& itemInfo);

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
        ErrorCode isValidItem(const ImageId& imageId) const;

        /**
         * @param groupId Entity Group Id to check.
         * @return OK if groupId is a valid entity group id in the root-level meta box, INVALID_GROUP_ID if not.
         *         UNINITIALIZED if initialize() had not been called.
         */
        ErrorCode isValidEntityGroup(const GroupId& groupId) const;

        /**
         * @brief Construct an ItemInfo out of ItemInfoEntry. Not a constructor as in not to
         *        affect the other c++ mechanisms. The image-specific data is not filled in.
         * @param ItemInfoEntry for the item
         * @return ItemInfo for the item */
        static ItemInfo makeItemInfo(const ItemInfoEntry& itemInfo);

        /// Reader internal information about each MetaBox
        struct MetaBoxInfo
        {
            unsigned int displayableMasterImages = 0;  ///< Number of master images
            ItemInfoMap itemInfoMap;                   ///< Information of other items
            Map<ImageId, Grid> gridItems;              ///< Data of image grid items
            Map<ImageId, Overlay> iovlItems;           ///< Data of image overlay items
            Properties properties;                     ///< Property information of each item
        };
        MetaBoxInfo mMetaBoxInfo;  ///< MetaBoxInfo struct for root-level MetaBox

        /**
         * @brief Extract information about non-image items of a MetaBox.
         * @param metaBox The MetaBox items of which to handle.
         * @return ItemInfoMap including information about non image type items. */
        static ItemInfoMap extractItemInfoMap(const MetaBox& metaBox);

        /**
         * @brief Create a MetaBoxProperties struct for the reader interface
         * @param metaBox MetaBox to get information from
         * @return Filled MetaBoxProperties */
        static MetaBoxProperties extractMetaBoxProperties(const MetaBox& metaBox);

        /**
         * @brief Create a MetaBoxFeature struct for the reader interface
         * @param imageFeatures Previously extracted ImageFeaturesMap
         * @param groupings     Previously extracted groupings
         * @return Filled MetaBoxProperties */
        static MetaBoxFeature extractMetaBoxFeatures(const ItemFeaturesMap& imageFeatures, const Groupings& groupings);

        /**
         * @brief Create a GroupingMap struct for the reader interface
         * @param metaBox MetaBox to get information from
         * @return Filled GroupingMap */
        static Groupings extractMetaBoxEntityToGroupMaps(const MetaBox& metaBox);

        /**
         * @brief Create a ItemFeaturesMap struct for the reader interface
         * @param metaBox MetaBox to get information from
         * @return Filled ItemFeaturesMap */
        static ItemFeaturesMap extractMetaBoxItemPropertiesMap(const MetaBox& metaBox);

        /**
         * @brief Extract item properties for reader internal use
         * @pre Filled mMetaBoxMap
         * @return Filled Properties */
        Properties processItemProperties() const;

        /**
         * @brief Fill imageItemParameterSetMap, imageToParameterSetMap and imageItemCodeTypeMap from metabox */
        static void processDecoderConfigProperties(const ItemPropertiesBox& iprp,
                                                   const ItemFeaturesMap& itemFeaturesMap,
                                                   Map<DecoderConfigId, ParameterSetMap>& imageItemParameterSetMap,
                                                   Map<ImageId, DecoderConfigId>& imageToParameterSetMap,
                                                   Map<ImageId, FourCCInt>& imageItemCodeTypeMap);

        /**
         * @brief Extract MetaBoxInfo struct for reader internal use
         * @param metaBox   MetaBox to get information from
         * @pre Filled mMetaBoxMap
         * @return Filled MetaBoxInfo */
        MetaBoxInfo extractItems(const MetaBox& metaBox) const;

        /**
         * @brief Load item data.
         * @param metaBox The MetaBox where the item is located
         * @param itemId  ID of the item
         * @param [out] data Item Data
         * @pre mInputStream is good
         * @return ErrorCode: OK, INVALID_ITEM_ID, FILE_READ_ERROR */
        ErrorCode loadItemData(const MetaBox& metaBox, ImageId itemId, DataVector& data) const;

        /**
         * @brief Load item data.
         * @param metaBox The MetaBox where the item is located
         * @param itemId  ID of the item
         * @param [out] data Item Data
         * @pre mInputStream is good
         * @return ErrorCode: OK, INVALID_ITEM_ID, FILE_READ_ERROR */
        ErrorCode readItem(const MetaBox& metaBox, ImageId itemId, uint8_t* memorybuffer, uint64_t maxSize) const;

        /**
         * @brief Convert information extracted from the MetaBox to fixed-sized arrays for public API.
         * @return Filled MetaBoxInformation struct.
         */
        MetaBoxInformation convertRootMetaBoxInformation(const MetaBoxProperties& metaboxProperties) const;

        /* *********************************************************************** */
        /* *********************** Track-specific section  *********************** */
        /* *********************************************************************** */

        /**
         * Get sample data, possibly from a segment.
         */
        ErrorCode getTrackSampleData(const SequenceId& trackId,
                                     const SequenceImageId& itemIdApi,
                                     uint8_t* memoryBuffer,
                                     uint64_t& memoryBufferSize);

        /** Given an init segment id and an item id find the segment id */
        ErrorCode segmentIdOf(SequenceId sequenceId, SequenceImageId itemId, SegmentId& segmentId) const;

        /// Next Sequence number replacing  for surviving through non-linearities in
        /// MovieFragmentHeader.FragmentSequenceNumber
        Sequence mNextSequence = {};

        typedef Map<SequenceId, DecodePts::PresentationTimeTS> SequenceIdPresentationTimeTSMap;

        InitTrackInfo& getInitTrackInfo(SequenceId initSegTrackId);
        const InitTrackInfo& getInitTrackInfo(SequenceId initSegTrackId) const;

        const TrackInfoInSegment& getTrackInfo(SegmentTrackId segTrackId) const;
        HEIF::ErrorCode getSampleInfo(SequenceId sequenceId,
                                      SequenceImageId sequenceImageId,
                                      SampleProperties& sampleInfo) const;
        const SamplePropertyVector& getSampleInfo(SegmentTrackId segTrackId, SequenceImageId& sampleBase) const;

        /**
         * @brief Update mDecoderCodeTypeMap to include the data from the samples
         * @param [in] SampleInfoVector sampleInfo Samples to traverse
         */
        static void updateDecoderCodeTypeMap(const SamplePropertyVector& sampleInfo,
                                             WriteOnceMap<SequenceImageId, FourCCInt>& decoderCodeTypeMap,
                                             std::size_t prevSampleInfoSize = 0);

        /**
         * @brief Update given SampleToParameterSetMap
         * @param [out] sampleToParameterSetMap Map where parameters are added
         * @param [in] sequenceId              Sequence/track ID
         * @param [in] sampleToParameterSetMap Samples to traverse
         * @param [in] size_t prevSampleInfoSize Index to start interating on (to avoid duplicate work on consecutive
         * moofs)
         */
        static void updateSampleToParametersSetMap(SampleToParameterSetMap& sampleToParameterSetMap,
                                                   SequenceId initSegTrackId,
                                                   const SamplePropertyVector& sampleInfo,
                                                   size_t prevSampleInfoSize = 0);

        /**
         * @brief Add samples to a addToTrackInfo for the reader interface
         * @param [in/out] trackInfo TrackInfo reference to add to.
         * @param [in] timeScale  std::uint32_t timescale for the samples.
         * @param [in] baseDataOffset  std::uint64_t baseDataOffset for the samples.
         * @param [in] ItemId itemIdBase sample id for the first sample in the current segment
         * @param [in] ItemId trackrunItemIdBase sample id for the first sample the current track run
         * @param [in] uint32_t sampleDescriptionIndex for samples in this run.
         * @param [in] trackRunBox TrackRunBox* to extract sample properties from */
        static void addSamplesToTrackInfo(TrackInfoInSegment& trackInfo,
                                          const FileInformationInternal& fileInformation,
                                          const InitTrackInfo& initTrackInfo,
                                          std::uint64_t baseDataOffset,
                                          uint32_t sampleDescriptionIndex,
                                          SequenceImageId itemIdBase,
                                          SequenceImageId trackrunItemIdBase,
                                          const TrackRunBox* trackRunBox);

        /**
         * @brief Add a mapping between segment number and a sequence.
         * If the mapping already exists, nothing changes. */
        void addSegmentSequence(SegmentId segmentId, Sequence sequence);

        /**
         * @brief Add to a TrackPropertiesMap struct for the reader interface
         * @param [in] segmentId Segment id of the track.
         * @param [in] moofBox MovieFragmentBox to extract properties from */
        void addToTrackProperties(SegmentId segmentId,
                                  MovieFragmentBox& moofBox,
                                  const SequenceIdPresentationTimeTSMap& earliestPTSTS);

        /**
         * For a given segment and track, find the first following (non-overlapping) ItemId
         */
        SequenceImageId getFollowingSequenceImageId(SegmentTrackId segTrackId) const;

        /**
         * For a given segment and track, find the last (non-overlapping) ItemId + 1 (or 0 if at the beginning)
         */
        SequenceImageId getPrecedingSequenceImageId(SegmentTrackId segTrackId) const;

        /**
         * @brief Find the preceding segment of given segment by using segment sequence numbers
         * @return Returns true if such a segment was found, in which case the segment id is placed
         *         to the return parameter precedingSegmentId
         */
        bool getPrecedingSegment(SegmentId curSegmentId, SegmentId& precedingSegmentId) const;


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
         * @brief Create a InitTrackInfoMap struct for the reader interface internal usage.
         * @param [in] segmentId Segment id.
         * @param [in] moovBox MovieBox to extract properties from
         * @return Filled TrackPropertiesMap */
        static InitTrackInfoMap extractInitTrackInfos(SegmentId segmentId,
                                                      const MovieBox& moovBox,
                                                      const SegmentPropertiesMap& segmentPropertiesMap);

        /**
         * @brief Create a TrackInfoInSegment map struct for the reader interface internal usage.
         * @param [in] segmentId Segment id.
         * @param [in] moovBox MovieBox to extract properties from
         * @return Filled TrackPropertiesMap */
        static void fillSegmentPropertiesMap(SegmentId segmentId,
                                             const MovieBox& moovBox,
                                             SegmentPropertiesMap& segmentPropertiesMap);

        /**
         * @brief Create a MoovProperties struct for the reader interface
         * @param [in] moovBox MovieBox to extract properties from
         * @return Filled MoovProperties */
        static MoovProperties extractMoovProperties(const MovieBox& moovBox);

        /**
         * @brief Fill mParameterSetMap and mTrackInfo clapProperties entries for a a TrackBox
         * @param [in] stsdBox SampleDescriptionBox to extract data from
         * @param [out] initTrackInfo Struct to fill extracted information to. */
        static void fillSampleEntryMap(const SampleDescriptionBox& stsdBox, InitTrackInfo& initTrackInfo);

        /**
         * @brief Find if there is certain type reference or references from tracks to a track.
         * @param trackPropertiesMap TrackPropertiesMap containing information about all tracks of the file
         * @param trackId            ID of the TrackBox to look references to
         * @param referenceType      4CC reference type to look for
         * @return True if a reference was found */
        static bool isAnyLinkedToWithType(const InitTrackInfoMap& trackPropertiesMap,
                                          SequenceId trackId,
                                          FourCCInt referenceType);

        /**
         * @brief Get direct decoding dependencies for a sample
         * @param itemId           Item ID of the sample
         * @param sgpd             SampleGroupDescriptionBox of the TrackBox
         * @param sampleToGroupBox SampleToGroupBox of the TrackBox
         * @return Item IDs of decoding dependencies */
        static Vector<SequenceImageId> getSampleDirectDependencies(SequenceImageId itemId,
                                                                   const SampleGroupDescriptionBox* sgpd,
                                                                   const SampleToGroupBox& sampleToGroupBox);

        /**
         * @brief Create a TrackFeature struct for the reader interface
         * @param [in] trackBox TrackBox to extract data from
         * @return Filled TrackFeature */
        static TrackFeature getTrackFeatures(const TrackBox* trackBox);

        /**
         * @brief Extract information about reference tracks for the reader interface
         * @param [in] trackBox TrackBox to extract data from
         * @return Reference track information */
        static TypeToIdsMap getReferenceTrackIds(const TrackBox* trackBox);

        /**
         * @brief Extract information about track groups for the reader interface
         * @param [in] trackBox TrackBox to extract data from
         * @return Track group information */
        static TrackGroupInfoMap getTrackGroupInfoMap(const TrackBox* trackBox);

        /**
         * @brief Extract information about reference tracks for the reader interface
         * @param [in] trackBox TrackBox to extract data from
         * @return Reference track information */
        static EditList getEditList(const TrackBox* trackBox, double repetitions);

        /**
         * @brief Extract information about sample groups for the reader interface
         * @param [in] trackBox TrackBox to extract data from
         * @return Sample grouping information */
        static Array<SampleGrouping> getSampleGroupings(const TrackBox* trackBox);

        /**
         * @brief Extract information about sample VisualEquivalenceEntry() groups ('eqiv') for the reader interface.
         * @param [in] trackBox TrackBox to extract data from.
         * @return Sample equivalency information. */
        static Array<SampleVisualEquivalence> getEquivalenceGroups(const TrackBox* trackBox);

        /**
         * @brief Extract information about sample SampleToMetadataItemEntry() groups ('stmi') for the reader interface.
         * @param [in] trackBox TrackBox to extract data from.
         * @return Sample to Metadata information. */
        static Array<SampleToMetadataItem> getSampleToMetadataItemGroups(const TrackBox* trackBox);

        /**
         * @brief Extract information about sample DirectReferenceSamplesList() groups ('refs') for the reader
         * interface.
         * @param [in] trackBox TrackBox to extract data from.
         * @return Sample to Metadata information. */
        static Array<DirectReferenceSamples> getDirectReferenceSamplesGroups(const TrackBox* trackBox);

        /**
         * @brief Extract information about alternate track IDs for a track for the reader interface
         * @param [in] trackBox TrackBox to look alternate track IDs for
         * @param [in] moovBox  MovieBox containing the TrackBox
         * @return IDs of alternate tracks. An empty vector if there was no alternates */
        static Vector<SequenceId> getAlternateTrackIds(const TrackBox* trackBox, const MovieBox& moovBox);

        /**
         * @brief Extract reader internal InitTrackInfo structure from TrackBox
         * @param [in] trackBox        TrackBox to extract data from
         * @return Filled InitTrackInfo structs */
        static InitTrackInfo extractInitTrackInfo(const TrackBox* trackBox);

        /**
         * @brief Extract reader internal TrackInfoInSegment structure from TrackBox
         * @param [in] trackBox        TrackBox to extract data from
         * @param [in] movieTimescale  Time scale of the MovieBox containing the TrackBox
         * @return Filled TrackInfoInSegment struct */
        static TrackInfoInSegment createTrackInfoInSegment(const TrackBox* trackBox, uint32_t movieTimescale);

        /**
         * @brief Extract reader internal information about samples
         * @param trackBox [in] trackBox TrackBox to extract data from
         * @param maxSampleSize max size of samples for track.
         * @return SamplePropertyVector containing information about every sample of the track */
        static SamplePropertyVector makeSamplePropertyVector(const TrackBox* trackBox, std::uint64_t& maxSampleSize);

        /**
         * @brief Add sample decoding dependencies
         * @param sequenceId        ID of the track
         * @param itemDecodingOrder Items/samples to get decoding dependencies for
         * @return DecodingOrderVector with decoding dependencies */
        Vector<TimestampIDPair> addDecodingDependencies(SequenceId sequenceId,
                                                        const Vector<TimestampIDPair>& itemDecodingOrder) const;

        /**
         * @brief convertTrackInformation Convert information extracted from the MovieBox to fixed-size arrays for the
         * public API.
         * @param trackPropertiesMap Data extracted from the MovieBox during initialization.
         * @return TrackInformation structs for track in the file.
         */
        Array<TrackInformation> convertTrackInformation(const InitTrackInfoMap& trackPropertiesMap) const;

        /**
         * @brief Make SegmentIndexBox array from SegmentIndexBox information
         * @param [in] sidxBox SegmentIndexBox  to get info from.
         * @param [out] segmentIndex SegmentIndex to store info into
         * @param [in] dataOffsetAnchor
         */
        static void makeSegmentIndex(const SegmentIndexBox& sidxBox,
                                     SegmentIndex& segmentIndex,
                                     int64_t dataOffsetAnchor);

        /**
         * @brief Set the segment's sidx fallback value based on the time information available on
         *        preceding segment (per segment sequence number) if it exists.
         */
        void setupSegmentSidxFallback(SegmentTrackId segTrackId);

        /**
         * Updates segment's sampleInfo's composition times per the trackInfo's pMap
         */
        void updateCompositionTimes(SegmentId segmentId);

        /**
         * @brief Find the preceding track info of given segment/track by using segment sequence numbers.
         *        This search skips segments that don't have the corresponding track.
         * @return Return a pointer to the found TrackInfo if it exists, otherwise nullptr.
         */
        const TrackInfoInSegment* getPrecedingTrackInfo(SegmentTrackId segTrackId) const;

        /** @return True if there exists track info for wanted track in the segment in question, false otherwise. */
        bool hasTrackInfo(SegmentTrackId segTrackId) const;

        /**
         * @brief Get parameters for the sequence image/sample.
         * @return Pointer to parameter set, nullptr if not found. */
        const ParameterSetMap* getParameterSetMap(SequenceId sequenceId, SequenceImageId sampleId) const;

        class FileReaderException : public ISOBMFF::Exception
        {
        public:
            FileReaderException(ErrorCode status, const char* message = nullptr)
                : Exception(message)
                , mErrorCode(status)
            {
            }

            const char* what() const noexcept override
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
    ErrorCode getRawItemType(const MetaBox& metaBox, ImageId itemId, FourCCInt& type);

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
     * @brief Check if one or more references from an item exists.
     * @param metaBox       MetaBox containing the item and possible references
     * @param itemId        ID of the item to search references for
     * @param referenceType 4CC reference type
     * @return True if one or more referenceType references from the item exists. */
    bool doReferencesFromItemIdExist(const MetaBox& metaBox, ImageId itemId, const FourCCInt& referenceType);

    /**
     * @brief Check if one or more references to an item exists.
     * @param metaBox       MetaBox containing the item and possible references
     * @param itemId        ID of the item to search references for
     * @param referenceType 4CC reference type
     * @return True if one or more referenceType references to the item exists. */
    bool doReferencesToItemIdExist(const MetaBox& metaBox, ImageId itemId, const FourCCInt& referenceType);

    /**
     * Template for making Array for the public API from a container.
     */
    template <typename T, typename Container>
    Array<T> makeArray(const Container& container);
}  // namespace HEIF

#endif /* HEIFREADERIMPL_HPP */
