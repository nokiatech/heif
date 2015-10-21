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
#include <algorithm>
#include <memory>

#define ANDROID_TO_STRING_HACK
#include "androidhacks.hpp"

using namespace std;

RootMetaImageWriter::RootMetaImageWriter(const vector<string>& handlerType, std::uint32_t contextId) :
    MetaWriter(handlerType.at(0), contextId)
{
    /// @todo Second parameter of type string vector is codec type, currently discarded
}

void RootMetaImageWriter::initWrite()
{
    mMetaItems.clear(); // Clear because of dual-pass
    MetaWriter::initWrite();
}

void RootMetaImageWriter::iinfWrite(MetaBox* metaBox) const
{
    for (const auto& item : mMetaItems)
    {
        metaBox->addItem(item.mId, item.mType, "HEVC image");
    }
}

void RootMetaImageWriter::ilocWrite(MetaBox* metaBox) const
{
    for (const auto& item : mMetaItems)
    {
        metaBox->addIloc(item.mId, item.mOffset, item.mLength, getBaseOffset());
    }
}

void RootMetaImageWriter::iprpWrite(MetaBox* metaBox, const unsigned int width, const unsigned int height) const
{
    std::vector<uint32_t> itemIds;
    for (const auto& item : mMetaItems)
    {
        itemIds.push_back(item.mId);
    }

    std::shared_ptr<HevcConfigurationBox> configBox(new HevcConfigurationBox);
    configBox->setConfiguration(mDecoderConfigRecord);
    metaBox->addProperty(configBox, itemIds, true);

    std::shared_ptr<ImageSpatialExtentsProperty> isep(new ImageSpatialExtentsProperty);
    isep->setDisplayWidth(width);
    isep->setDisplayHeight(height);
    metaBox->addProperty(isep, itemIds, true);
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

void RootMetaImageWriter::parseInputBitStream(const std::string& filename)
{
    H265Parser mediaParser;
    if (!mediaParser.openFile(filename.c_str()))
    {
        throw runtime_error("Not able to open H.265 bit stream file '" + filename + "'");
    }

    ParserInterface::AccessUnit* accessUnit = new ParserInterface::AccessUnit { };

    bool hasNalUnits = (accessUnit->mNalUnits.size() > 0) ? true : false;
    bool hasMoreImages = (mediaParser.parseNextAU(*accessUnit));
    bool hasSpsNalUnits = (accessUnit->mSpsNalUnits.size() > 0) ? true : false;
    bool hasPpsNalUnits = (accessUnit->mPpsNalUnits.size() > 0) ? true : false;
    bool isHevc = hasMoreImages || hasNalUnits || hasSpsNalUnits || hasPpsNalUnits;

    if (isHevc)
    {
        std::vector<uint8_t> vpsNals = accessUnit->mVpsNalUnits.front();
        std::vector<uint8_t> spsNals = accessUnit->mSpsNalUnits.front();
        std::vector<uint8_t> ppsNals = accessUnit->mPpsNalUnits.front();

        mDecoderConfigRecord.makeConfigFromSPS(spsNals, 0.0);
        mDecoderConfigRecord.addNalUnit(vpsNals, HevcNalUnitType::VPS, 0);
        mDecoderConfigRecord.addNalUnit(spsNals, HevcNalUnitType::SPS, 0);
        mDecoderConfigRecord.addNalUnit(ppsNals, HevcNalUnitType::PPS, 0);

        uint32_t itemOffset = 0;
        int imageIndex = 0;
        while (hasMoreImages)
        {
            if (accessUnit == nullptr)
            {
                accessUnit = new ParserInterface::AccessUnit { };
                hasMoreImages = mediaParser.parseNextAU(*accessUnit);
            }
            if (hasMoreImages)
            {
                MetaItem itemInfo;
                uint32_t itemLength = 0;
                for (auto nalU : accessUnit->mNalUnits)
                {
                    itemLength += nalU.size() + 4 - getNalStartCodeSize(nalU);
                }

                itemInfo.mType = HVC1_ITEM_TYPE;
                itemInfo.mId = IdSpace::getValue();
                itemInfo.mLength = itemLength;
                itemInfo.mOffset = itemOffset;
                mMetaItems.push_back(itemInfo);
                storeValue("item_indx", to_string(itemInfo.mId));
                itemOffset = itemOffset + itemLength; // Offset for next image
                ++imageIndex;
            }
            delete accessUnit;
            accessUnit = nullptr;
        }
    }
}
