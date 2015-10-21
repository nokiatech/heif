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

#include "entitygroupwriter.hpp"
#include "datastore.hpp"
#include "entitytogroupbox.hpp"
#include "metabox.hpp"
#include "services.hpp"
#include "writerconstants.hpp"

#include <string>

#define ANDROID_STOI_HACK
#include "androidhacks.hpp"

EntityGroupWriter::EntityGroupWriter(const IsoMediaFile::AltrIndexPairVector& config) :
    MetaWriter(),
    mConfig(config)
{
}

void EntityGroupWriter::write(MetaBox* metaBox)
{
    const IdPairToEntityIdMap idPairToEntityIdMap = makeEntityMap();

    std::vector<EntityId> entityIds;
    for (const auto& item : mConfig)
    {
        entityIds.push_back(idPairToEntityIdMap.at(std::make_pair(item.uniq_bsid, item.item_indx)));
    }
    metaBox->addAlternateGroup(entityIds);
}

EntityGroupWriter::IdPairToEntityIdMap EntityGroupWriter::makeEntityMap() const
{
    const std::vector<std::uint32_t> storeIds = DataServe::getStoreIds();

    IdPairToEntityIdMap idPairToEntityIdMap;
    for (auto contextId : storeIds)
    {
        DataStore* store = DataServe::getStore(contextId).get();
        UniqBsid uniqBsid = 0;

        if (not store->isValueSet("uniq_bsid"))
        {
            continue;
        }

        uniqBsid = std::stoi(store->getValue("uniq_bsid").at(0));
        const std::string type = store->getValue("capsulation").at(0);

        if (type == TRAK_ENCAPSULATION)
        {
            // For tracks, add mapping to track itself
            idPairToEntityIdMap[std::pair<uint32_t, uint32_t>(uniqBsid, 0)] = contextId;
        }
        else if (type == META_ENCAPSULATION)
        {
            // For meta, add mapping to image items inside it
            const std::vector<std::string> itemIdStrings = store->getValue("item_indx");
            ItemIndex imageIndex = 1;
            for (const auto& itemIdString : itemIdStrings)
            {
                const uint32_t itemId = std::stoi(itemIdString);
                idPairToEntityIdMap[std::make_pair(uniqBsid, imageIndex)] = itemId;
                ++imageIndex;
            }
        }
    }

    return idPairToEntityIdMap;
}
