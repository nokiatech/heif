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

#ifndef ROOTMETAIMAGEWRITER_HPP
#define ROOTMETAIMAGEWRITER_HPP

#include "hevcdecoderconfigrecord.hpp"
#include "metawriter.hpp"
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
        std::uint32_t mId;     /**< Index of item       */
        std::uint32_t mOffset; /**< Offset in bitstream */
        std::uint32_t mLength; /**< Length in bytes     */
    };
    std::vector<MetaItem> mMetaItems;

    /** HEVC decoder configuration for this context / this bitstream */
    HevcDecoderConfigurationRecord mDecoderConfigRecord;

    /**
     * @brief Add entries to the Item Information Box
     * @param [in,out] metaBox Pointer to the MetaBox where to add data.
     */
    void iinfWrite(MetaBox* metaBox) const;

    /**
     * @brief Add entries to the Item Location Box
     * @param [in,out] metaBox Pointer to the MetaBox where to add data.
     */
    void ilocWrite(MetaBox* metaBox) const;

    /**
     * @brief Add 'ispe' and 'hvcC' properties to the Item Properties Box
     * @param [in,out] metaBox Pointer to the MetaBox where to add data.
     * @param width, height Dimensions of the image.
     */
    void iprpWrite(MetaBox* metaBox, unsigned int width, unsigned int height) const;

    /**
     * @brief Parse H.265 bit stream and fill mMetaItems with image information.
     * @details Item IDs are added to DataStore of this track, to key "item_indx".
     * @param filename Name of the bitstream file
     */
    void parseInputBitStream(const std::string& filename);

    /**
     * Get NAL start code size
     * @param nalU NAL unit data
     * @return Size of the start code
     */
    unsigned int getNalStartCodeSize(const std::vector<std::uint8_t>& nalU) const;
};

#endif /* end of include guard: ROOTMETAIMAGEWRITER_HPP */

