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

#ifndef ROOTMETAIMAGEWRITER_HPP
#define ROOTMETAIMAGEWRITER_HPP

#include "avcdecoderconfigrecord.hpp"
#include "hevcdecoderconfigrecord.hpp"
#include "imagespatialextentsproperty.hpp"
#include "layerselectorproperty.hpp"
#include "lhevcdecoderconfigrecord.hpp"
#include "mediatypedefs.hpp"
#include "metawriter.hpp"
#include "operatingpointsinformation.hpp"
#include "parserinterface.hpp"
#include "targetolsproperty.hpp"

#include <map>
#include <memory>
#include <string>

class MetaBox;

/**
 * @brief Abstract base class for MetaBox writers for writing HEVC images.
 */
class RootMetaImageWriter : public MetaWriter
{
public:
    /**
    * @brief RootMetaImageWriter constructor
    * @param handlerType Handler type
    * @param contextId   Context ID of this writer
    */
    RootMetaImageWriter(const std::vector<std::string>& handlerType, uint32_t contextId);
    virtual ~RootMetaImageWriter() = default;

    /**
     * @brief Add data to a MetaBox based on configuration data given during writer construction.
     * @see MetaWriter::write()
     */
    void write(MetaBox* metaBox) = 0;

protected:
    /** Clear internal data. This is needed because of dual-pass writing */
    void initWrite();

    /** Item information, parsed from input bitstream */
    struct MetaItem
    {
        std::string mType;     /**< Identifier string   */
        std::string mName;     /**< Item name           */
        std::uint32_t mId;     /**< Index of item       */
        std::uint32_t mOffset; /**< Offset in bitstream */
        std::uint32_t mLength; /**< Length in bytes     */
    };
    std::vector<MetaItem> mMetaItems;

    /**
     * @brief Add entries to the Item Information Box
     * @param [in,out] metaBox Pointer to the MetaBox where to add data.
     * @param [in] hidden True if items should be marked as hidden.
     */
    void iinfWrite(MetaBox* metaBox, bool hidden = false) const;

    /**
     * @brief Add entries to the Item Location Box
     * @param [in,out] metaBox Pointer to the MetaBox where to add data.
     */
    void ilocWrite(MetaBox* metaBox) const;

    /**
     * @brief Add 'ispe' and 'hvcC' properties to the Item Properties Box
     * @param [in,out] metaBox Pointer to the MetaBox where to add data.
     */
    void iprpWrite(MetaBox* metaBox) const;

    /**
     * @brief Parse bit stream file and fill mMetaItems with image information.
     * @details Item IDs are added to DataStore of this track, to key "item_indx".
     * @param fileName Name of the bitstream file
     * @param codeType Configured code type
     */
    void parseInputBitStream(const std::string& fileName, const std::string& codeType);

    /**
     * Get NAL start code size
     * @param nalU NAL unit data
     * @return Size of the start code
     */
    unsigned int getNalStartCodeSize(const std::vector<std::uint8_t>& nalU) const;

    /**
     * Return index to 'ispe' with given dimensions. A new ImageSpatialExtentsProperty is created on demand.
     * @param width Width of the image.
     * @param height Height of the image.
     * @return Index to an 'ispe' property with wanted dimensions, specific for this image writer.
     */
    unsigned int getIspeIndex(std::uint32_t width, std::uint32_t height);

    /**
     * Add ImageSpatialExtentsProperty 'ispe' for an image item. A new 'ispe' is generated if one with matching
     * dimensions does not already exist.
     * @param itemId Item ID of the image item.
     * @param width Width of the image.
     * @param height Height of the image.
     */
    void addIspeForItem(std::uint32_t itemId, std::uint32_t width, std::uint32_t height);

    /**
     * @brief Calculate image item length in bytes from NAL units.
     * @param nalUnits NAL unit vector containing the item
     * @return Item length in bytes
     */
    uint32_t getItemLength(const std::vector<std::vector<std::uint8_t>>& nalUnits) const;

    /**
     * @brief Create an decoder configuration from AccessUnit and store it.
     * @param accessUnit AccessUnit containing (VPS,) SPS and PPS NAL units
     * @param mediaType Media type
     * @param usedLayers For L-HEVC: include SPS and PPS for these layers
     * @param layer For L-HEVC: the layer this decoder configuration is made for
     * @return Index of the added configuration.
     */
    int addDecoderConfiguration(const ParserInterface::AccessUnit& au,
                                 MediaType mediaType, const std::vector<uint8_t>& usedLayers = std::vector<uint8_t>(),
                                 const uint8_t layer = 0);

    /**
     * @brief Create and add a new item info structure
     * @param itemType Item type (e.g. "hvc1")
     * @param itemName Item name (e.g. "HEVC Image")
     * @param offset todo
     * @param length Length of the item in bytes
     * @param configIndex
     * @return Item ID of the item.
     */
    std::uint32_t addItem(const std::string& itemType, const std::string& itemName, uint32_t offset, uint32_t length,
                          uint32_t configIndex);

private:
    /** Decoder configurations for this bitstream */
    struct Configuration
    {
        // Only one DecoderConfig is valid for each configuration
        std::unique_ptr<AvcDecoderConfigurationRecord> avcDecoderConfig;
        std::unique_ptr<HevcDecoderConfigurationRecord> hevcDecoderConfig;
        std::unique_ptr<LHevcDecoderConfigurationRecord> lhevcDecoderConfig;
        std::vector<std::uint32_t> itemIds;           ///< Item IDs of images using this configuration
    };
    std::vector<std::unique_ptr<Configuration>> mDecoderConfigs; ///< Decoder configurations of this bitstream

    struct Ispe
    {
        std::shared_ptr<ImageSpatialExtentsProperty> ispe;
        std::vector<std::uint32_t> itemIds;           ///< Item IDs of images using this property
    };
    std::vector<Ispe> mIspes;

    /** Bitstream type specific item type and name (for example "hvc1" and "HEVC Image"). */
    struct ItemType
    {
        std::string mType;
        std::string mName;
    };

    ItemType getItemType(MediaType type) const;

    /**
     * Get index of a contained 'ispe' property which has dimensions defined in the decoder configuration record
     * at configIndex. A new 'ispe' will be created if one with suitable dimensions does not already exist.
     * @param configIndex Index of the decoder configuration record (mDecoderConfigs).
     * @return Index of the contained 'ispe'.
     */
    unsigned int getIspeIndex(unsigned int configIndex);

    /**
     * Get a SPS NAL unit based on layer index.
     * @todo This method is a hack which handles only most simple cases. This *must* be refactored.
     * @param au Access data unit from the parser which includes the NAL unit.
     * @param layerIndex Index of the layer.
     * @return SPS data.
     */
    const std::vector<uint8_t>& getSps(const ParserInterface::AccessUnit& au, const uint8_t layerIndex) const;

    /**
     * Get a PPS NAL unit based on layer index.
     * @todo This method is a hack which handles only most simple cases. This *must* be refactored.
     * @param au Access data unit from the parser which includes the NAL unit.
     * @param layerIndex Index of the layer.
     * @return PPS data.
     */
    const std::vector<uint8_t>& getPps(const ParserInterface::AccessUnit& au, const uint8_t layerIndex) const;
};

#endif /* end of include guard: ROOTMETAIMAGEWRITER_HPP */

