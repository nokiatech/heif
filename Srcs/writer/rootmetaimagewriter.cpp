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

#include "rootmetaimagewriter.hpp"
#include "h265parser.hpp"
#include "hevcconfigurationbox.hpp"
#include "idspace.hpp"
#include "imagespatialextentsproperty.hpp"
#include "iteminfobox.hpp"
#include "itemlocationbox.hpp"
#include "metabox.hpp"
#include "services.hpp"
#include "writerconstants.hpp"
#include <memory>
#include <stdexcept>

#define ANDROID_TO_STRING_HACK
#include "androidhacks.hpp"

using namespace std;

RootMetaImageWriter::RootMetaImageWriter(const vector<string>& handlerType, std::uint32_t contextId) :
    MetaWriter(handlerType.at(0), contextId),
    mNextItemOffset(0)
{
    /// @todo Second parameter of type string vector is codec type, currently discarded
}

void RootMetaImageWriter::initWrite()
{
    mMetaItems.clear(); // Clear because of dual-pass
    mConfigs.clear();
    mNextItemOffset = 0;

    MetaWriter::initWrite();
}

void RootMetaImageWriter::iinfWrite(MetaBox* metaBox, const bool hidden) const
{
    for (const auto& item : mMetaItems)
    {
        metaBox->addItem(item.mId, item.mType, "HEVC Image", hidden);
    }
}

void RootMetaImageWriter::ilocWrite(MetaBox* metaBox) const
{
    for (const auto& item : mMetaItems)
    {
        metaBox->addIloc(item.mId, item.mOffset, item.mLength, getBaseOffset());
    }
}

void RootMetaImageWriter::iprpWrite(MetaBox* metaBox) const
{
    for (const auto& config : mConfigs)
    {
        std::shared_ptr<HevcConfigurationBox> configBox(new HevcConfigurationBox);
        configBox->setConfiguration(config.decoderConfig);
        metaBox->addProperty(configBox, config.itemIds, true);

        std::shared_ptr<ImageSpatialExtentsProperty> isep(new ImageSpatialExtentsProperty);
        isep->setDisplayWidth(config.decoderConfig.getPicWidth());
        isep->setDisplayHeight(config.decoderConfig.getPicHeight());
        metaBox->addProperty(isep, config.itemIds, true);
    }
}

unsigned int RootMetaImageWriter::getNalStartCodeSize(const vector<uint8_t>& nalU) const
{
    /// @todo Check if this method is correct (looks a bit unusual/non-robust)
    unsigned int size = 0;
    for (const auto byte : nalU)
    {
        if (byte == 0)
        {
            ++size;
        }
        else if (byte == 1)
        {
            ++size;
            break;
        }
    }
    return size;
}

void RootMetaImageWriter::addDecoderConfiguration(const ParserInterface::AccessUnit& au)
{
    Configuration config;
    config.decoderConfig.makeConfigFromSPS(au.mSpsNalUnits.front(), 0.0);
    config.decoderConfig.addNalUnit(au.mVpsNalUnits.front(), HevcNalUnitType::VPS, 0);
    config.decoderConfig.addNalUnit(au.mSpsNalUnits.front(), HevcNalUnitType::SPS, 0);
    config.decoderConfig.addNalUnit(au.mPpsNalUnits.front(), HevcNalUnitType::PPS, 0);
    mConfigs.push_back(config);
}

uint32_t RootMetaImageWriter::getItemLength(const std::list<std::vector<std::uint8_t>>& nalUnits) const
{
    uint32_t length = 0;
    for (const auto& nalU : nalUnits)
    {
        length += nalU.size() + 4 - getNalStartCodeSize(nalU);
    }
    return length;
}

void RootMetaImageWriter::addItem(const uint32_t lenght)
{
    MetaItem itemInfo;
    itemInfo.mType = HVC1_ITEM_TYPE;
    itemInfo.mId = IdSpace::getValue();
    itemInfo.mLength = lenght;
    itemInfo.mOffset = mNextItemOffset;
    storeValue("item_indx", to_string(itemInfo.mId));
    mConfigs.back().itemIds.push_back(itemInfo.mId);
    mMetaItems.push_back(itemInfo);
    mNextItemOffset += lenght;
}

void RootMetaImageWriter::parseInputBitStream(const std::string& filename)
{
    H265Parser mediaParser;
    ParserInterface::AccessUnit au;

    if (!mediaParser.openFile(filename.c_str()))
    {
        throw runtime_error("Not able to open H.265 bit stream file '" + filename + "'");
    }

    bool moreAccessUnits = true;
    while (moreAccessUnits)
    {
        moreAccessUnits = mediaParser.parseNextAU(au);

        if ((not au.mPpsNalUnits.empty()) || (not au.mSpsNalUnits.empty()) || (not au.mVpsNalUnits.empty()))
        {
            addDecoderConfiguration(au);
        }

        if (not au.mNalUnits.empty())
        {
            const uint32_t length = getItemLength(au.mNalUnits);
            addItem(length);
        }
    }
}

HevcDecoderConfigurationRecord RootMetaImageWriter::getFirstDecoderConfiguration() const
{
    return mConfigs.at(0).decoderConfig;
}
