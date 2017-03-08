/* Copyright (c) 2017, Nokia Technologies Ltd.
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

#ifndef LAYERIMAGEWRITER_HPP
#define LAYERIMAGEWRITER_HPP

#include "isomediafile.hpp"
#include "rootmetaimagewriter.hpp"
#include <map>
#include <memory>
#include <set>
#include <string>

class Metabox;

/**
 * @brief MetaBox writer for writing layered images.
 */
class LayerImageWriter : public RootMetaImageWriter
{
public:
    /**
    * @brief LayerImageWriter constructor
    * @param config    Configuration of this writer
    * @param contextId Context ID of this writer
    */
    LayerImageWriter(const IsoMediaFile::Layer& config, std::uint32_t contextId);
    virtual ~LayerImageWriter() = default;

    /**
     * @brief Add data to a MetaBox based on configuration data given during writer construction.
     * @see MetaWriter::write()
     */
    virtual void write(MetaBox* metaBox);

private:
    IsoMediaFile::Layer mConfig; ///< Input configuration of the writer
    MetaBox* mMetaBox; ///< MetaBox where this layer is written to.

    struct OinfEntry
    {
        std::shared_ptr<OperatingPointsInformation> oinf;
        std::vector<std::uint32_t> itemIds;           ///< Item IDs of images using this property
    };
    std::vector<OinfEntry> mOinfEntries;

    /**
     * @brief Add a new 'oinf' property.
     * @param vpsNalUnits VPS nal units to extract 'oinf' information from.
     * @return Generated OinfEntry index in mOinfEntries.
     */
    int addOinf(const std::vector<std::uint8_t>& vpsNalUnit);

    /**
     * Add item references to an external baselayer, if present.
     */
    void addExblReferences();

    /**
     * @brief Generate image item and property information based on input bitsream.
     * @param fileName File name of the input bitstream file.
     * @param tols The output layer set index used for decoding generated image items.
     * @param lsel The layer identifier used for decoding generated image items. Set to -1 in case
     *             no LayerSelectorProperty is needed.
     */
    void parseInputBitStream(const std::string& fileName, std::uint16_t tols, std::int32_t lsel);
};

#endif /* end of include guard: LAYERIMAGEWRITER_HPP */
