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

#include "auxiliaryimagewriter.hpp"
#include "auxiliarytypeproperty.hpp"
#include "datastore.hpp"
#include "hevcconfigurationbox.hpp"
#include "imagespatialextentsproperty.hpp"
#include "itempropertiesbox.hpp"
#include "metabox.hpp"
#include "services.hpp"
#include "writerconstants.hpp"
#include <memory>

#define ANDROID_STOI_HACK
#define ANDROID_TO_STRING_HACK
#include "androidhacks.hpp"

AuxiliaryImageWriter::AuxiliaryImageWriter(const IsoMediaFile::Auxiliary& config, const std::uint32_t contextId) :
    RootMetaImageWriter({""}, contextId),
    mConfig(config)
{
}

void AuxiliaryImageWriter::write(MetaBox* metaBox)
{
    RootMetaImageWriter::initWrite();
    storeValue("uniq_bsid", std::to_string(mConfig.uniq_bsid));
    storeValue("capsulation", META_ENCAPSULATION);
    RootMetaImageWriter::parseInputBitStream(mConfig.file_path);
    ItemSet itemIds = addItemReferences(metaBox);

    ilocWrite(metaBox, itemIds);
    iinfWrite(metaBox, itemIds);
    iprpWrite(metaBox, itemIds);
}

void AuxiliaryImageWriter::ilocWrite(MetaBox* metaBox, const ItemSet& itemIds) const
{
    for (const auto& item : mMetaItems)
    {
        if (itemIds.count(item.mId))
        {
            metaBox->addIloc(item.mId, item.mOffset, item.mLength, getBaseOffset());
        }
    }
}

void AuxiliaryImageWriter::iinfWrite(MetaBox* metaBox, const ItemSet& itemIds) const
{
    for (const auto& item : mMetaItems)
    {
        if (itemIds.count(item.mId))
        {
            metaBox->addItem(item.mId, item.mType, "HEVC Image", mConfig.hidden);
        }
    }
}

void AuxiliaryImageWriter::iprpWrite(MetaBox* metaBox, const ItemSet& itemIds) const
{
    std::vector<std::uint32_t> itemIdVector;
    for (const auto id : itemIds)
    {
        itemIdVector.push_back(id);
    }

    std::shared_ptr<HevcConfigurationBox> configBox(new HevcConfigurationBox);
    const HevcDecoderConfigurationRecord decoderConfig = RootMetaImageWriter::getFirstDecoderConfiguration();

    configBox->setConfiguration(decoderConfig);
    metaBox->addProperty(configBox, itemIdVector, true);

    auto auxBox = std::make_shared<AuxiliaryTypeProperty>();
    auxBox->setAuxType(mConfig.urn);
    metaBox->addProperty(auxBox, itemIdVector, true);

    auto ispe = std::make_shared<ImageSpatialExtentsProperty>();
    ispe->setDisplayWidth(decoderConfig.getPicWidth());
    ispe->setDisplayHeight(decoderConfig.getPicHeight());
    metaBox->addProperty(ispe, itemIdVector, true);
}

AuxiliaryImageWriter::ItemSet AuxiliaryImageWriter::addItemReferences(MetaBox* metaBox) const
{
    ItemId fromId = mMetaItems.at(0).mId;
    ItemSet itemIds;

    if (mConfig.idxs_list.size() == 0)
    {
        // No idxs_list was given, so assume 1:1 mapping for referenced image items and auxiliary image items, or
        // map an auxiliary image to every master if there is only one.
        const std::vector<std::string> masterImageIds = getMasterStoreValue("item_indx");
        for (size_t i = 0; i < masterImageIds.size(); ++i)
        {
            if (mMetaItems.size() > 1)
            {
                fromId = mMetaItems.at(i).mId;
            }
            metaBox->addItemReference("auxl", fromId, std::stoi(masterImageIds.at(i)));
            itemIds.insert(fromId);
        }
    }
    else
    {
        // idxs_list was given, so set the first auxiliary image for all images listed.
        const ReferenceToItemIdMap referenceMap = createReferenceToItemIdMap();
        for (size_t i = 0; i < mConfig.refs_list.size(); ++i)
        {
            const UniqBsid masterUniqBsid =  mConfig.refs_list.at(i);
            const ItemIdVector& indexList = mConfig.idxs_list.at(i);
            for (const auto index : indexList)
            {
                const ItemId masterImageId = referenceMap.at(masterUniqBsid).at(index - 1);
                metaBox->addItemReference("auxl", fromId, masterImageId);
                itemIds.insert(fromId);
            }
        }
    }

    return itemIds;
}

/// @todo This is common with derived item writer and should be refactored.
AuxiliaryImageWriter::ReferenceToItemIdMap AuxiliaryImageWriter::createReferenceToItemIdMap() const
{
    ReferenceToItemIdMap referenceToItemIdMap;

    // Add meta master contexts to the map
    const std::vector<std::uint32_t> storeIds = DataServe::getStoreIds();
    for (const auto storeId : storeIds)
    {
        const std::shared_ptr<DataStore> dataStore = DataServe::getStore(storeId);

        if (dataStore->isValueSet("uniq_bsid") &&
            dataStore->isValueSet("capsulation") &&
            dataStore->getValue("capsulation").at(0) == META_ENCAPSULATION)
        {
            const UniqBsid uniqBsid = std::stoi(dataStore->getValue("uniq_bsid").at(0));
            const std::vector<std::string> itemIdStrings = dataStore->getValue("item_indx");
            ItemIdVector itemIds;
            for (const auto& itemId : itemIdStrings)
            {
                itemIds.push_back(std::stoi(itemId));
            }
            referenceToItemIdMap.insert( { uniqBsid, itemIds });
        }
    }

    return referenceToItemIdMap;
}

