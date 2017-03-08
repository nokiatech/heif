/* Copyright (c) 2015-2016-2017, Nokia Technologies Ltd.
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

#include "avcconfigurationbox.hpp"
#include "hevcconfigurationbox.hpp"
#include "idspace.hpp"
#include "imagespatialextentsproperty.hpp"
#include "iteminfobox.hpp"
#include "itemlocationbox.hpp"
#include "layeredhevcconfigurationitemproperty.hpp"
#include "mediatypedefs.hpp"
#include "metabox.hpp"
#include "parserfactory.hpp"
#include "services.hpp"
#include "writerconstants.hpp"
#include <memory>
#include <stdexcept>

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
    mDecoderConfigs.clear();
    mIspes.clear();

    MetaWriter::initWrite();
}

void RootMetaImageWriter::iinfWrite(MetaBox* metaBox, const bool hidden) const
{
    for (const auto& item : mMetaItems)
    {
        metaBox->addItem(item.mId, item.mType, item.mName, hidden);
    }
}

void RootMetaImageWriter::ilocWrite(MetaBox* metaBox) const
{
    for (const auto& item : mMetaItems)
    {
        metaBox->addIloc(item.mId, item.mOffset, item.mLength, getBaseOffset());
    }
}


unsigned int RootMetaImageWriter::getIspeIndex(const unsigned int configIndex)
{
    std::uint32_t picWidth = 0;
    std::uint32_t picHeight = 0;

    const Configuration* config = mDecoderConfigs.at(configIndex).get();

    if (config->avcDecoderConfig != nullptr)
    {
        const AvcDecoderConfigurationRecord& configRecord = *(config->avcDecoderConfig);
        picWidth  = configRecord.getPicWidth();
        picHeight = configRecord.getPicHeight();
    }
    else if (config->hevcDecoderConfig != nullptr)
    {
        const HevcDecoderConfigurationRecord& configRecord = *(config->hevcDecoderConfig);
        picWidth  = configRecord.getPicWidth();
        picHeight = configRecord.getPicHeight();
    }
    else if (config->lhevcDecoderConfig != nullptr)
    {
        throw std::runtime_error("Error while generating 'ispe'. Calling RootMetaImageWriter::getIspeIndex for a LHEVC decoder config is not supported.");
    }
    else
    {
        throw std::runtime_error("Error while generating 'ispe'. Decoder configuration not present?");
    }

    return getIspeIndex(picWidth, picHeight);
}

unsigned int RootMetaImageWriter::getIspeIndex(const std::uint32_t width, const std::uint32_t height)
{
    // Check if there already is an 'ispe' with same dimensions.
    for (size_t i = 0; i < mIspes.size(); ++i)
    {
        ImageSpatialExtentsProperty* ispe = mIspes.at(i).ispe.get();
        if ((ispe->getDisplayWidth() == width) && (ispe->getDisplayHeight() == height))
        {
            return i;
        }
    }

    // Create a new ispe.
    Ispe newIspe;
    newIspe.ispe = std::make_shared<ImageSpatialExtentsProperty>(width, height);
    mIspes.push_back(newIspe);
    return mIspes.size() - 1;
}

void RootMetaImageWriter::addIspeForItem(const std::uint32_t itemId, const std::uint32_t width, const std::uint32_t height)
{
    unsigned int index = getIspeIndex(width, height);
    mIspes.at(index).itemIds.push_back(itemId);
}

void RootMetaImageWriter::iprpWrite(MetaBox* metaBox) const
{
    for (const auto& config : mDecoderConfigs)
    {
        if (config->avcDecoderConfig)
        {
            // Add "avcC" box
            auto configBox = std::make_shared<AvcConfigurationBox>();

            AvcDecoderConfigurationRecord& configRecord = *(config->avcDecoderConfig);
            configBox->setConfiguration(configRecord);
            metaBox->addProperty(configBox, config->itemIds, true);
        }
        else if (config->hevcDecoderConfig)
        {
            // Add "hvcC" box
            auto configBox = std::make_shared<HevcConfigurationBox>();

            HevcDecoderConfigurationRecord& configRecord = *(config->hevcDecoderConfig);
            configBox->setConfiguration(configRecord);
            metaBox->addProperty(configBox, config->itemIds, true);
        }
        else if (config->lhevcDecoderConfig)
        {
            // Add "lhvC" box
            auto configBox = make_shared<LayeredHevcConfigurationItemProperty>();

            LHevcDecoderConfigurationRecord& configRecord = *(config->lhevcDecoderConfig);
            configBox->setConfiguration(configRecord);
            metaBox->addProperty(configBox, config->itemIds, true);
        }
        else
        {
            throw std::runtime_error("Failed to create 'iprp' box, decoder configuration not set.");
        }
    }

    for (const auto& ispe : mIspes)
    {
        metaBox->addProperty(ispe.ispe, ispe.itemIds, false);
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

int RootMetaImageWriter::addDecoderConfiguration(const ParserInterface::AccessUnit& au,
                                                  MediaType mediaType, const std::vector<uint8_t>& usedLayers,
                                                  const uint8_t layer)
{
    std::unique_ptr<Configuration> config(new Configuration);

    switch (mediaType)
    {
    case MediaType::AVC:
    {
        AvcDecoderConfigurationRecord* configRecord = new AvcDecoderConfigurationRecord;
        configRecord->makeConfigFromSPS(au.mSpsNalUnits.front());
        configRecord->addNalUnit(au.mSpsNalUnits.front(), AvcNalUnitType::SPS, 0);
        configRecord->addNalUnit(au.mPpsNalUnits.front(), AvcNalUnitType::PPS, 0);

        config->avcDecoderConfig.reset(configRecord);
        break;
    }
    case MediaType::HEVC:
    {
        HevcDecoderConfigurationRecord* configRecord = new HevcDecoderConfigurationRecord;

        configRecord->makeConfigFromSPS(au.mSpsNalUnits.front(), 0.0);
        configRecord->addNalUnit(au.mVpsNalUnits.front(), HevcNalUnitType::VPS, 0);
        configRecord->addNalUnit(au.mSpsNalUnits.front(), HevcNalUnitType::SPS, 0);
        configRecord->addNalUnit(au.mPpsNalUnits.front(), HevcNalUnitType::PPS, 0);

        config->hevcDecoderConfig.reset(configRecord);
        break;
    }
    case MediaType::LHEVC:
    {
        LHevcDecoderConfigurationRecord* configRecord = new LHevcDecoderConfigurationRecord;
        configRecord->makeConfigFromSPS(getSps(au, layer), au.mVpsNalUnits.front());

        // Add SPS and PPS for all required layers.
        for (const auto usedLayer : usedLayers)
        {
            configRecord->addNalUnit(getSps(au, usedLayer), HevcNalUnitType::SPS, 0);
            configRecord->addNalUnit(getPps(au, usedLayer), HevcNalUnitType::PPS, 0);
        }
        configRecord->addNalUnit(au.mVpsNalUnits.front(), HevcNalUnitType::VPS, 0);

        config->lhevcDecoderConfig.reset(configRecord);
        break;
    }
    default:
    {
        throw std::runtime_error("Failed to add decoder configuration for unsupported media type.");
    }
    }

    mDecoderConfigs.push_back(std::move(config));
    return mDecoderConfigs.size()-1;
}

uint32_t RootMetaImageWriter::getItemLength(const std::vector<std::vector<std::uint8_t>>& nalUnits) const
{
    uint32_t length = 0;
    for (const auto& nalU : nalUnits)
    {
        length += nalU.size() + 4 - getNalStartCodeSize(nalU);
    }
    return length;
}

std::uint32_t RootMetaImageWriter::addItem(const std::string& itemType, const std::string& itemName,
                                           const uint32_t offset, const uint32_t length, const uint32_t configIndex)
{
    MetaItem itemInfo;
    itemInfo.mType = itemType;
    itemInfo.mName = itemName;
    itemInfo.mId = IdSpace::getValue();
    itemInfo.mLength = length;
    itemInfo.mOffset = offset;
    storeValue("item_indx", to_string(itemInfo.mId));
    mDecoderConfigs.at(configIndex)->itemIds.push_back(itemInfo.mId);
    mMetaItems.push_back(itemInfo);

    return itemInfo.mId;
}

void RootMetaImageWriter::parseInputBitStream(const std::string& fileName, const std::string& codeType)
{
    const MediaType mediaType = MediaTypeTool::getMediaTypeByCodeType(codeType, fileName);
    const ItemType itemTypeInfo = getItemType(mediaType);

    // Create bitstream parser for this code type
    std::unique_ptr<ParserInterface> mediaParser = ParserFactory::getParser(mediaType);
    if (!mediaParser->openFile(fileName))
    {
        throw std::runtime_error("Not able to open " + MediaTypeTool::getBitStreamTypeName(mediaType) +
                                 " bit stream file '" + fileName + "'");
    }

    ParserInterface::AccessUnit au;
    bool moreAccessUnits = true;
    int decoderConfigurationIndex = -1;
    int ispeIndex = -1;

    uint32_t offset = 0;

    while (moreAccessUnits)
    {
        moreAccessUnits = mediaParser->parseNextAU(au);

        if ((not au.mPpsNalUnits.empty()) || (not au.mSpsNalUnits.empty()) || (not au.mVpsNalUnits.empty()))
        {
            decoderConfigurationIndex = addDecoderConfiguration(au, mediaType);
            ispeIndex = getIspeIndex(decoderConfigurationIndex);
        }

        if (not au.mNalUnits.empty())
        {
            if (decoderConfigurationIndex < 0)
            {
                throw std::runtime_error("VCL NAL units in bitstream before decoder configuration ?");
            }

            const uint32_t length = getItemLength(au.mNalUnits);
            const uint32_t itemId = addItem(itemTypeInfo.mType, itemTypeInfo.mName, offset, length, decoderConfigurationIndex);
            mIspes.at(ispeIndex).itemIds.push_back(itemId);
            offset += length;
        }
    }
}

RootMetaImageWriter::ItemType RootMetaImageWriter::getItemType(const MediaType type) const
{
    const std::map<MediaType, ItemType> types =
    {
        { MediaType::AVC, {AVC1_ITEM_TYPE, AVC1_ITEM_NAME} },
        { MediaType::HEVC, {HVC1_ITEM_TYPE, HVC1_ITEM_NAME} },
        { MediaType::LHEVC, {LHV1_ITEM_TYPE, LHV1_ITEM_NAME} }
    };

    if (types.count(type) == 0)
    {
        throw std::runtime_error("Failed to define item type for unsupported media type");
    }

    return types.at(type);
}

/// @todo This method is a hack which must be refactored. It handles only the most simple cases.
const std::vector<uint8_t>& RootMetaImageWriter::getSps(const ParserInterface::AccessUnit& au, const uint8_t layerIndex) const
{
    const size_t spsSize = au.mSpsNalUnits.size();
    unsigned int index = layerIndex;
    if ((spsSize == 1) && (layerIndex == 1))
    {
        index = 0;
    }
    return au.mSpsNalUnits.at(index);
}

/// @todo This method is a hack which must be refactored. It handles only the most simple cases.
const std::vector<uint8_t>& RootMetaImageWriter::getPps(const ParserInterface::AccessUnit& au, const uint8_t layerIndex) const
{
    const size_t ppsSize = au.mPpsNalUnits.size();
    unsigned int index = layerIndex;
    if ((ppsSize == 1) && (layerIndex == 1))
    {
        index = 0;
    }
    return au.mPpsNalUnits.at(index);
}
