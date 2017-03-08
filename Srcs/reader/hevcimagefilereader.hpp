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

#ifndef HEVCIMAGEFILEREADER_HPP
#define HEVCIMAGEFILEREADER_HPP

#include "decodepts.hpp"
#include "filetypebox.hpp"
#include "imagefilereaderinterface.hpp"
#include "imagespatialextentsproperty.hpp"
#include "metabox.hpp"
#include "moviebox.hpp"

#include <fstream>
#include <istream>
#include <memory>

class CleanAperture;
class AvcDecoderConfigurationRecord;
class HevcDecoderConfigurationRecord;
class LHevcDecoderConfigurationRecord;

/** @brief Interface for reading an Image File (e.g. HEIF Image File) from the filesystem. */
class HevcImageFileReader : public ImageFileReaderInterface
{
public:
    HevcImageFileReader();
    virtual ~HevcImageFileReader() = default;

    /// @see ImageFileReaderInterface::initialize()
    virtual void initialize(const std::string& fileName);

    /// @see ImageFileReaderInterface::initialize()
    virtual void initialize(std::istream& inputStream);

    /// @see ImageFileReaderInterface::close()
    virtual void close();

    /// @see ImageFileReaderInterface::getMajorBrand()
    virtual std::string getMajorBrand() const;

    /// @see ImageFileReaderInterface::getMinorBrand()
    virtual std::uint32_t getMinorVersion() const;

    /// @see ImageFileReaderInterface::getCompatibleBrands()
    virtual std::vector<std::string> getCompatibleBrands() const;

    /// @see ImageFileReaderInterface::getDisplayWidth()
    virtual std::uint32_t getDisplayWidth(std::uint32_t contextId) const;

    /// @see ImageFileReaderInterface::getDisplayHeight()
    virtual std::uint32_t getDisplayHeight(std::uint32_t contextId) const;

    /// @see ImageFileReaderInterface::getWidth()
    virtual std::uint32_t getWidth(std::uint32_t contextId, std::uint32_t itemId) const;

    /// @see ImageFileReaderInterface::getHeight()
    virtual std::uint32_t getHeight(std::uint32_t contextId, std::uint32_t itemId) const;

    /// @see ImageFileReaderInterface::getMatrix()
    virtual std::vector<std::int32_t> getMatrix() const;

    /// @see ImageFileReaderInterface::getPlaybackDurationInSecs()
    virtual float getPlaybackDurationInSecs(std::uint32_t contextId) const;

    /// @see ImageFileReaderInterface::setForcedTimedPlayback()
    virtual void setForcedTimedPlayback(std::uint32_t contextId, float forcedFps);

    /// @see ImageFileReaderInterface::setForcedLoopPlayback()
    virtual void setForcedLoopPlayback(std::uint32_t contextId, bool forceLoopPlayback = true);

    /// @see ImageFileReaderInterface::getItemListByType()
    virtual void getItemListByType(std::uint32_t contextId, const std::string& itemType, IdVector& itemIds) const;

    /// @see ImageFileReaderInterface::getItemType()
    virtual std::string getItemType(std::uint32_t contextId, std::uint32_t itemId) const;

    /// @see ImageFileReaderInterface::getReferencedFromItemListByType()
    virtual void getReferencedFromItemListByType(std::uint32_t contextId, std::uint32_t fromItemId, const std::string& referenceType,
        IdVector& itemIds) const;

    /// @see ImageFileReaderInterface::getReferencedToItemListByType()
    virtual void getReferencedToItemListByType(std::uint32_t contextId, std::uint32_t toItemId, const std::string& referenceType,
        IdVector& itemIds) const;

    /// @see ImageFileReaderInterface::getCoverImageItemId()
    virtual std::uint32_t getCoverImageItemId(std::uint32_t contextId) const;

    /// @see ImageFileReaderInterface::getItemData()
    virtual void getItemData(std::uint32_t contextId, std::uint32_t itemId, DataVector& itemData);

    /// @see ImageFileReaderInterface::getItemIovl()
    virtual IovlItem getItemIovl(std::uint32_t contextId, std::uint32_t itemId) const;

    /// @see ImageFileReaderInterface::getItemGrid()
    virtual GridItem getItemGrid(std::uint32_t contextId, std::uint32_t itemId) const;

    /// @see ImageFileReaderInterface::getPropertyAuxc()
    virtual AuxProperty getPropertyAuxc(std::uint32_t contextId, std::uint32_t index) const;

    /// @see ImageFileReaderInterface::getPropertyImir()
    virtual ImirProperty getPropertyImir(std::uint32_t contextId, std::uint32_t index) const;

    /// @see ImageFileReaderInterface::getPropertyIrot()
    virtual IrotProperty getPropertyIrot(std::uint32_t contextId, std::uint32_t index) const;

    /// @see ImageFileReaderInterface::getPropertyRloc()
    virtual RlocProperty getPropertyRloc(std::uint32_t contextId, std::uint32_t index) const;

    // @see ImageFileReaderInterface::getPropertyClap()
    virtual ClapProperty getPropertyClap(std::uint32_t contextId, std::uint32_t index) const;

    /// @see ImageFileReaderInterface::getPropertyLsel()
    virtual LselProperty getPropertyLsel(std::uint32_t contextId, std::uint32_t index) const;

    /// @see ImageFileReaderInterface::getPropertyTols()
    virtual TolsProperty getPropertyTols(std::uint32_t contextId, std::uint32_t index) const;

    /// @see ImageFileReaderInterface::getItemProperties()
    virtual PropertyTypeVector getItemProperties(std::uint32_t contextId, std::uint32_t itemId) const;

    /// @see ImageFileReaderInterface::getItemDataWithDecoderParameters()
    virtual void getItemDataWithDecoderParameters(std::uint32_t contextId, std::uint32_t itemId,
        DataVector& itemDataWithDecoderParameters);

    /// @see ImageFileReaderInterface::getItemProtectionData()
    virtual DataVector getItemProtectionScheme(std::uint32_t contextId, std::uint32_t itemId) const;

    /// @see ImageFileReaderInterface::getItemTimestamps()
    virtual void getItemTimestamps(std::uint32_t contextId, TimestampMap& timestamps) const;

    /// @see ImageFileReaderInterface::getTimestampsOfItem()
    virtual void getTimestampsOfItem(std::uint32_t contextId, std::uint32_t itemId, TimestampVector& timestamps) const;

    /// @see ImageFileReaderInterface::getItemsInDecodingOrder()
    virtual void getItemsInDecodingOrder(std::uint32_t contextId, DecodingOrderVector& itemDecodingOrder) const;

    /// @see ImageFileReaderInterface::getItemDecodeDependencies()
    virtual void getItemDecodeDependencies(std::uint32_t contextId, std::uint32_t itemId, IdVector& dependencies) const;

    /// @see ImageFileReaderInterface::getDecoderCodeType()
    virtual std::string getDecoderCodeType(const uint32_t contextId, const uint32_t itemId) const;

    /// @see ImageFileReaderInterface::getDecoderParameterSets()
    virtual void getDecoderParameterSets(std::uint32_t contextId, std::uint32_t itemId,
       ParameterSetMap& parameterSets) const;

private:
    std::ifstream mInputFileStream;
    std::istream* mInputStream;

    /// @see ImageFileReaderInterface::FileProperties
    FileProperties mFileProperties;

    /* ********************************************************************** */
    /* ****************** Common for meta and track content ***************** */
    /* ********************************************************************** */

    enum class State
    {
        UNINITIALIZED, ///< State before starting to read file and after closing it
        INITIALIZING,  ///< State during parsing the file
        READY          ///< State after the file has been parsed and information extracted
    };
    State mState; ///< Running state of the reader API implementation

    typedef std::uint32_t ContextId;                          ///< Context (= meta box and track) identifiers
    typedef std::uint32_t ItemId;                             ///< Sample/image/item identifiers
    typedef std::pair<ContextId, ItemId> Id;                  ///< Convenience type combining context and item IDs
    typedef std::pair<ItemId, Timestamp> ItemIdTimestampPair; ///< Pair of Item/sample ID and timestamp

    std::map<Id, std::string> mDecoderCodeTypeMap;  ///< Extracted decoder code types for each sample and image
    std::map<Id, ParameterSetMap> mParameterSetMap; ///< Extracted decoder parameter sets
    std::map<Id, Id> mImageToParameterSetMap;       ///< Map from every sample and image item to parameter set map entry

    /// Context type classification
    enum class ContextType
    {
        META,
        TRACK,
        FILE,
        NOT_SET
    };

    /// General information about each root meta and track context
    struct ContextInfo
    {
        ContextType contextType = ContextType::NOT_SET;
        bool isCoverImageSet = false;
        ItemId coverImageId = 0;
        bool isForcedLoopPlaybackEnabled = false;
    };
    std::map<ContextId, ContextInfo> mContextInfoMap;

    FileTypeBox mFtyp; ///< File Type Box for later information retrieval

    /** @throws FileReaderException StatusCode=[UNINITIALIZED] if input file has not been read yet */
    void isInitialized() const;

    /**
     * Return compatibility version of the file by trying to locate 'mdat' which has data beginning with
     * bytes 'NHW_'.
     * @return Compatibility version number if it was found, otherwise zero.
     */
    int readCompatibilityVersion();

    /**
     * Identify context by context id.
     * @param [in] id Context ID
     * @throws FileReaderException, StatusCode=[UNINITIALIZED, INVALID_CONTEXT_ID]
     * @return Context type based on mContextInfoMap. */
    ContextType getContextType(ContextId id) const;

    /** Parse input stream, fill mFileProperties and implementation internal data structures. */
    void readStream();

    FileFeature getFileFeatures() const;
    void readBox(BitStream& bitstream, std::string& boxType);

    /** Get dimensions for item
     * @param [in]  contextId Track or meta context id
     * @param [in]  itemId    Item ID from track or meta context
     * @param [out] width     Width if the item in pixels
     * @param [out] height    Height if the item in pixels */
    void getImageDimensions(ContextId contextId, ItemId itemId, std::uint32_t& width, std::uint32_t& height) const;

    /** Move decoder configuration parameter data to ParamSetMap
     * @param record AVC decoder configuration
     * @return Decoder parameters */
    ParameterSetMap makeDecoderParameterSetMap(const AvcDecoderConfigurationRecord& record) const;

    /** Move decoder configuration parameter data to ParamSetMap
     * @param record HEVC decoder configuration
     * @return Decoder parameters */
    ParameterSetMap makeDecoderParameterSetMap(const HevcDecoderConfigurationRecord& record) const;

    /** Move decoder configuration parameter data to ParamSetMap
     * @param record LHEVC decoder configuration
     * @return Decoder parameters */
    ParameterSetMap makeDecoderParameterSetMap(const LHevcDecoderConfigurationRecord& record) const;

    /** @return Ids of all items of a context */
    IdVector getContextItems(ContextId contextId) const;

    /**
     * @brief Check item protection status from ItemInfoEntry item_protection_index
     * @param contextId Meta context ID
     * @param itemId    ID of the item
     * @return True if the item is protected. Returns always false for other than meta contexts.
     * @todo Add support for protected track content if needed. */
    bool isProtected(std::uint32_t contextId, std::uint32_t itemId) const;

    /** Get item data from AVC bitstream
     *  @param [in]  rawItemData Raw AVC bitstream data
     *  @param [out] itemData    Retrieved item data.
     *  @pre initialize() has been called successfully. */
    void getAvcItemData(const DataVector& rawItemData, DataVector& itemData);

    /** Get item data from HEVC bitstream
     *  @param [in]  rawItemData Raw HEVC bitstream data
     *  @param [out] itemData    Retrieved item data.
     *  @pre initialize() has been called successfully. */
    void getHevcItemData(const DataVector& rawItemData, DataVector& itemData);

    /* ********************************************************************** */
    /* *********************** Meta-specific section  *********************** */
    /* ********************************************************************** */

    std::map<ContextId, MetaBox> mMetaBoxMap; ///< Map of read MetaBoxes

    typedef std::map<ItemId, PropertyTypeVector> Properties; ///< Convenience type for mapping item properties

    /// Reader internal information about each image item
    struct ImageInfo
    {
        std::string type = "invalid"; ///< Image item type, should be "master", "hidden", "pre-computed", "hvc1" or "lhv1"
        std::uint32_t width = 0;      ///< Width of the image from ispe property
        std::uint32_t height = 0;     ///< Height of the image from ispe property
        double displayTime = 0;       ///< Display timestamp generated by the reader
    };
    typedef std::map<ItemId, ImageInfo> ImageInfoMap;

    /// For non-image items (Exif or XML metadata)
    struct ItemInfo
    {
        std::string type; ///< Item type from Item Information box
    };
    typedef std::map<ItemId, ItemInfo> ItemInfoMap;

    /// Reader internal information about each MetaBox
    struct MetaBoxInfo
    {
        unsigned int displayableMasterImages = 0; ///< Number of master images
        bool isForcedLoopPlaybackEnabled = false; ///< True if looping has been manually enabled
        bool isForcedFpsSet = false;              ///< True if FPS for MetaBox has been manually set
        float forcedFps = 0.0;                    ///< Manually set FPS
        ImageInfoMap imageInfoMap;                ///< Information of all image items
        ItemInfoMap itemInfoMap;                  ///< Information of other items
        std::map<ItemId, GridItem> gridItems;     ///< Data of image grid items
        std::map<ItemId, IovlItem> iovlItems;     ///< Data of image overlay items
        Properties properties;                    ///< Property information of each item
    };
    std::map<ContextId, MetaBoxInfo> mMetaBoxInfo; ///< MetaBoxInfo struct for each MetaBox context

    /**
     * @brief Fill MetaBoxInfo imageInfoMap and displayableMasterImages for wanted context ID.
     * @pre Context ID is found from mMetaBoxMap, mFileProperties.rootLevelMetaBoxProperties.imageFeaturesMap has
     *      been filled.
     * @param contextId Context ID of the MetaBox */
    void fillImageInfoMap(ContextId contextId);

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
    MetaBoxFeature extractMetaBoxFeatures(const ImageFeaturesMap& imageFeatures, const GroupingMap& groupings) const;

    /**
     * @brief Create a GroupingMap struct for the reader interface
     * @param metaBox MetaBox to get information from
     * @return Filled GroupingMap */
    GroupingMap extractMetaBoxEntityToGroupMaps(const MetaBox& metaBox) const;

    /**
     * @brief Create a ImageFeaturesMap struct for the reader interface
     * @param metaBox MetaBox to get information from
     * @return Filled ImageFeaturesMap */
    ImageFeaturesMap extractMetaBoxImagePropertiesMap(const MetaBox& metaBox) const;

    /**
     * @brief Create a ItemFeaturesMap struct for the reader interface
     * @param metaBox MetaBox to get information from
     * @return Filled ItemFeaturesMap */
    ItemFeaturesMap extractMetaBoxItemPropertiesMap(const MetaBox& metaBox) const;

    /**
     * @brief Create ContextInfo for reader internal use
     * @param metaBoxProperties MetaBoxProperties to get information from
     * @return Filled ContextInfo */
    ContextInfo createContextInfo(const MetaBoxProperties& metaBoxProperties) const;

    /**
     * @brief Extract item properties for reader internal use
     * @param contextId Meta context ID
     * @pre Filled mMetaBoxMap
     * @return Filled Properties */
    Properties processItemProperties(ContextId contextId) const;

    /**
     * @brief Fill mImageToParameterSetMap and mParameterSetMap
     * @param contextId Meta context ID
     * @pre Filled mMetaBoxMap, filled mFileProperties.rootLevelMetaBoxProperties.imageFeaturesMap
     * @todo To support multiple MetaBoxes, ImageFeaturesMap should be a parameter (not use one from root meta). */
    void processDecoderConfigProperties(const ContextId contextId);

    /**
     * @brief Extract MetaBoxInfo struct for reader internal use
     * @param metaBox   MetaBox to get information from
     * @param contextId Meta context ID
     * @pre Filled mMetaBoxMap
     * @return Filled MetaBoxInfo */
    MetaBoxInfo extractItems(const MetaBox& metaBox, const std::uint32_t contextId) const;

    /**
     * @brief Load item data.
     * @param metaBox The MetaBox where the item is located
     * @param itemId  ID of the item
     * @pre mInputStream is good
     * @return Item data */
    std::vector<std::uint8_t> loadItemData(const MetaBox& metaBox, const ItemId itemId) const;

    /**
     * @brief Load item data.
     * @param metaBox The MetaBox where the item is located
     * @param itemId  ID of the item
     * @param [out] data Item Data
     * @pre mInputStream is good */
    void readItem(const MetaBox& metaBox, ItemId itemId, std::vector<std::uint8_t>& data) const;

    /* *********************************************************************** */
    /* *********************** Track-specific section  *********************** */
    /* *********************************************************************** */

    typedef std::uint32_t SampleDescriptionIndex;

    std::vector<std::int32_t> mMatrix; ///< Video transformation matrix from the Movie Header Box

    /// Reader internal information about each sample.
    struct SampleInfo
    {
        std::uint32_t decodingOrder  = 0;            ///< Decoding order/sequence number of the sample
        std::vector<std::uint32_t> compositionTimes; ///< Timestamps of the sample. Possible edit list is considered here.
        std::uint64_t dataOffset = 0;                ///< File offset of sample data in bytes
        std::uint32_t dataLength = 0;                ///< Length of same in bytes
        std::uint32_t width = 0;                     ///< Width of the frame
        std::uint32_t height = 0;                    ///< Height of the frame
        IdVector decodeDependencies;                 ///< Direct decoding dependencies
    };
    typedef std::vector<SampleInfo> SampleInfoVector;

    struct TrackInfo
    {
        SampleInfoVector samples; ///< Information about each sample in the TrackBox
        std::uint32_t width;      ///< display width in pixels, from 16.16 fixed point in TrackHeaderBox
        std::uint32_t height;     ///< display height in pixels, from 16.16 fixed point in TrackHeaderBox
        double duration;          ///< Track duration in seconds, from TrackHeaderBox
        DecodePts::PMap pMap;     ///< Display timestamps, from edit list
        std::map<SampleDescriptionIndex, ClapProperty> clapProperties; ///< Clean aperture data from sample description entries
    };
    std::map<ContextId, TrackInfo> mTrackInfo; ///< Reader internal information about each TrackBox

    /**
     * @brief Create a TrackPropertiesMap struct for the reader interface
     * @param [in] moovBox MovieBox to extract properties from
     * @return Filled TrackPropertiesMap */
    TrackPropertiesMap fillTrackProperties(MovieBox& moovBox);

    /**
     * @brief Create a MoovProperties struct for the reader interface
     * @param [in] moovBox MovieBox to extract properties from
     * @return Filled MoovProperties */
    MoovProperties extractMoovProperties(const MovieBox& moovBox) const;

    /**
     * @brief Fill mParameterSetMap and mTrackInfo clapProperties entries for a a TrackBox
     * @param [in] trackBox TrackBox to extract data from */
    void fillSampleEntryMap(TrackBox* trackBox);

    /**
     * @brief Find if there is certain type reference or references from tracks to a track.
     * @param trackPropertiesMap TrackPropertiesMap containing information about all tracks of the file
     * @param trackId            ID of the TrackBox to look references to
     * @param referenceType      4CC reference type to look for
     * @return True if a reference was found */
    bool isAnyLinkedToWithType(const TrackPropertiesMap& trackPropertiesMap, std::uint32_t trackId,
        const std::string& referenceType) const;

    /**
     * @brief Get direct decoding dependencies for a sample
     * @param itemId           Item ID of the sample
     * @param sgpd             SampleGroupDescriptionBox of the TrackBox
     * @param sampleToGroupBox SampleToGroupBox of the TrackBox
     * @return Item IDs of decoding dependencies */
    std::vector<ItemId> getSampleDirectDependencies(const ItemId itemId, const SampleGroupDescriptionBox* sgpd,
        const SampleToGroupBox& sampleToGroupBox) const;

    /**
     * @brief Create a TrackFeature struct for the reader interface
     * @param [in] trackBox TrackBox to extract data from
     * @return Filled TrackFeature */
    TrackFeature getTrackFeatures(TrackBox* trackBox) const;

    /**
     * @brief Extract information about reference tracks for the reader interface
     * @param [in] trackBox TrackBox to extract data from
     * @return Reference track information */
    TypeToIdsMap getReferenceTrackIds(TrackBox* trackBox) const;

    /**
     * @brief Extract information about sample group IDs for the reader interface
     * @param [in] trackBox TrackBox to extract data from
     * @return Sample grouping information */
    TypeToIdsMap getSampleGroupIds(TrackBox* trackBox) const;

    /**
     * @brief Extract information about alternate track IDs for a track for the reader interface
     * @param [in] trackBox TrackBox to look alternate track IDs for
     * @param [in] moovBox  MovieBox containing the TrackBox
     * @return IDs of alternate tracks. An empty vector if there was no alternates */
    IdVector getAlternateTrackIds(TrackBox* trackBox, MovieBox& moovBox) const;

    /**
     * @brief Extract reader internal TrackInfo structure from TrackBox
     * @param [in] trackBox TrackBox to extract data from
     * @param [in] moovBox  MovieBox containing the TrackBox
     * @return Filled TrackInfo struct */
    TrackInfo extractTrackInfo(TrackBox* trackBox, MovieBox& moovBox) const;

    /**
     * @brief Extract reader internal information about samples
     * @param trackBox [in] trackBox TrackBox to extract data from
     * @param pMap Presentation map for the track
     * @return SampleInfoVector containing information about every sample of the track */
    SampleInfoVector makeSampleInfoVector(TrackBox* trackBox, const DecodePts::PMap& pMap) const;

    /**
     * @brief Extract information about samples for the reader interface
     * @param trackBox [in] trackBox TrackBox to extract data from
     * @return Filled SamplePropertiesMap */
    SamplePropertiesMap makeSamplePropertiesMap(TrackBox* trackBox);

    /**
     * @brief Add sample decoding dependencies
     * @param contextId         ID of the track
     * @param itemDecodingOrder Items/samples to get decoding dependencies for
     * @return DecodingOrderVector with decoding dependencies */
    DecodingOrderVector addDecodingDependencies(uint32_t contextId, const DecodingOrderVector& itemDecodingOrder) const;

    /**
     * @brief Load frame data for a sample/item/frame from the input stream
     * @param frameIndex 0-based index of the sample (/item ID)
     * @param trackInfo  TrackInfo containing the sample
     * @return Data of the frame */
    DataVector getTrackFrameData(unsigned int frameIndex, const TrackInfo& trackInfo) const;

    /**
     * @brief Read bytes from stream to an integer value.
     * @param stream Stream to read bytes from.
     * @param count  Number of bytes to read (0-8).
     * @return Read value.
     */
    uint64_t readBytes(std::istream* stream, unsigned int count);
};

/* *********************************************************************** */
/* ************************* Helper functions **************************** */
/* *********************************************************************** */

/**
 * Helper function to get item type from  the Item Information Box inside a MetaBox
 * @param metaBox Pointer to the MetaBox
 * @param itemId ID of the item
 * @return 4CC type name */
std::string getRawItemType(const MetaBox& metaBox, uint32_t itemId);

/**
 * @brief Convert CleanAperture to ImageFileReaderInterface::ClapProperty
 * @param clapBox Pointer to input CleanAperture
 * @return A ClapProperty filled with data from CleanAperture */
ImageFileReaderInterface::ClapProperty makeClap(const CleanAperture* clapBox);

/**
 * @brief Recognize image item types ("hvc1", "lhv1", "grid", "iovl", "iden")
 * @param type 4CC Item type from Item Info Entry
 * @return True if the 4CC is an image type */
bool isImageItemType(const std::string& type);

/**
 * @brief Check if one or more references from an item exists.
 * @param metaBox       MetaBox containing the item and possible references
 * @param itemId        ID of the item to search references for
 * @param referenceType 4CC reference type
 * @return True if one or more referenceType references from the item exists. */
bool doReferencesFromItemIdExist(const MetaBox& metaBox, uint32_t itemId, const std::string& referenceType);

/**
 * @brief Check if one or more references to an item exists.
 * @param metaBox       MetaBox containing the item and possible references
 * @param itemId        ID of the item to search references for
 * @param referenceType 4CC reference type
 * @return True if one or more referenceType references to the item exists. */
bool doReferencesToItemIdExist(const MetaBox& metaBox, uint32_t itemId, const std::string& referenceType);

#endif /* HEVCIMAGEFILEREADER_HPP */

