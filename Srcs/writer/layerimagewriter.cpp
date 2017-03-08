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

#include "layerimagewriter.hpp"
#include "datastore.hpp"
#include "layerselectorproperty.hpp"
#include "log.hpp"
#include "metabox.hpp"
#include "parserfactory.hpp"
#include "services.hpp"
#include "targetolsproperty.hpp"
#include "writerconstants.hpp"
#include <memory>

LayerImageWriter::LayerImageWriter(const IsoMediaFile::Layer& config, const std::uint32_t contextId) :
    RootMetaImageWriter({ "" }, contextId),
    mConfig(config),
    mMetaBox(nullptr)
{
}

void LayerImageWriter::write(MetaBox* metaBox)
{
    mMetaBox = metaBox;
    RootMetaImageWriter::initWrite();
    mOinfEntries.clear();

    storeValue("uniq_bsid", std::to_string(mConfig.uniq_bsid));
    storeValue("capsulation", META_ENCAPSULATION);

    // Having no input file_path set implies to using bitstream of the referenced context.
    std::string bitstreamFile = mConfig.file_path;
    if (bitstreamFile == "")
    {
        std::shared_ptr<DataStore> store = DataServe::getStore("uniq_bsid", std::to_string(mConfig.base_refr));
        bitstreamFile = store->getValue("bitstream").at(0);
    }

    parseInputBitStream(bitstreamFile, mConfig.target_outputlayer, mConfig.layer_selection);

    addExblReferences();

    RootMetaImageWriter::ilocWrite(metaBox);
    RootMetaImageWriter::iinfWrite(metaBox, mConfig.hidden);
    RootMetaImageWriter::iprpWrite(metaBox);
}

void LayerImageWriter::addExblReferences()
{
    // Do not write external base layer references if the writer was not assigned an own bitstream.
    if (mConfig.file_path == "")
    {
        return;
    }

    std::shared_ptr<DataStore> store = DataServe::getStore("uniq_bsid", std::to_string(mConfig.base_refr));
    const std::vector<std::string> baseLayerImageIds = store->getValue("item_indx");
    for (size_t i = 0; i < baseLayerImageIds.size(); ++i)
    {
        std::uint32_t layerImageId = mMetaItems.at(i).mId;
        mMetaBox->addItemReference("exbl", layerImageId, std::stoi(baseLayerImageIds.at(i)));
    }
}

void LayerImageWriter::parseInputBitStream(const std::string& fileName,
                                           const std::uint16_t tols,
                                           const std::int32_t lsel)
{
    // Create bitstream parser for this code type
    std::unique_ptr<ParserInterface> mediaParser = ParserFactory::getParser(MediaType::LHEVC);
    if (!mediaParser->openFile(fileName))
    {
        throw std::runtime_error("Not able to open L-HEVC bit stream file '" + fileName + "'");
    }

    ParserInterface::AccessUnit au;
    bool moreAccessUnits = true;
    int layeredDecoderConfigurationIndex = -1;
    int oinfIndex = -1;

    std::vector<std::uint32_t> itemIds;

    bool vpsExtensionFound = true; /// @todo This should default to false (after VPS extension parsing has been integrated).
    uint32_t nextItemOffset = 0;
    while (moreAccessUnits)
    {
        moreAccessUnits = mediaParser->parseNextAU(au);

        if ((not au.mPpsNalUnits.empty()) || (not au.mSpsNalUnits.empty()) || (not au.mVpsNalUnits.empty()))
        {
            if (not vpsExtensionFound)
            {
                logInfo() << "LayerImageWriter::parseInputBitStream - no vps extension found, continuing" << std::endl;
                continue;
            }
            logInfo() << "LayerImageWriter::parseInputBitStream - adding lhevc config" << std::endl;
            oinfIndex = addOinf(au.mVpsNalUnits.front());
            const std::vector<uint8_t> usedLayers = mOinfEntries.at(oinfIndex).oinf->getLayerIds(tols);

            /// @todo Using 'tols' value as the layer index like this is an exceptionally dirty hack. Fix it.
            layeredDecoderConfigurationIndex = addDecoderConfiguration(au, MediaType::LHEVC, usedLayers, tols);
        }

        if (not au.mNalUnits.empty())
        {
            if ((layeredDecoderConfigurationIndex < 0) || (oinfIndex < 0))
            {
                throw std::runtime_error("VCL NAL units in bitstream before decoder configuration ?");
            }

            const uint32_t length = getItemLength(au.mNalUnits);
            const uint32_t itemId = addItem(LHV1_ITEM_TYPE, LHV1_ITEM_NAME, nextItemOffset, length,
                           layeredDecoderConfigurationIndex);
            itemIds.push_back(itemId);
            OinfEntry& oinf = mOinfEntries.at(oinfIndex);
            oinf.itemIds.push_back(itemId);
            const auto extents = oinf.oinf->getMaxExtents(tols);
            addIspeForItem(itemId, extents.first, extents.second);
            nextItemOffset += length;
        }
    }

    for (auto oinf : mOinfEntries)
    {
        mMetaBox->addProperty(oinf.oinf, oinf.itemIds, true);
    }

    auto tolsProperty = std::make_shared<TargetOlsProperty>(tols);
    mMetaBox->addProperty(tolsProperty, itemIds, true);

    if (lsel != -1)
    {
        auto lselProperty = std::make_shared<LayerSelectorProperty>(lsel);
        mMetaBox->addProperty(lselProperty, itemIds, true);
    }
}

int LayerImageWriter::addOinf(const std::vector<std::uint8_t>& vpsNalUnit)
{
    auto oinfProperty = std::make_shared<OperatingPointsInformation>();
    oinfProperty->makeOinf(vpsNalUnit);
    mOinfEntries.push_back({ oinfProperty, std::vector<uint32_t>() });

    return mOinfEntries.size() - 1;
}
