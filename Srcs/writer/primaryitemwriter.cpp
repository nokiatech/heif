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

#include "primaryitemwriter.hpp"
#include "datastore.hpp"
#include "idspace.hpp"
#include "log.hpp"
#include "metabox.hpp"
#include "services.hpp"

#define ANDROID_STOI_HACK
#include "androidhacks.hpp"

PrimaryItemWriter::PrimaryItemWriter(const uint32_t uniqBsid,
    const uint32_t itemIndex, const uint32_t masterContextId) :
    MetaWriter(),
    mPrimaryItemUniqBsid(uniqBsid),
    mPrimaryItemIndex(itemIndex),
    mMasterContextId(masterContextId)
{
}


void PrimaryItemWriter::write(MetaBox* metaBox)
{
    uint32_t itemId = 0;

    const std::vector<std::uint32_t> storeIds = DataServe::getStoreIds();

    // Use first item of master as default if primary item has not been set
    if (mPrimaryItemUniqBsid == 0xffffffff || mPrimaryItemIndex == 0xffffffff)
    {
        std::shared_ptr<DataStore> store = DataServe::getStore(mMasterContextId);
        const std::vector<std::string> itemIds = store->getValue("item_indx");
        if (itemIds.size() > 0)
        {
            itemId = std::stoi(itemIds.at(0));
        }
        else
        {
            throw std::runtime_error("No items in master context?");
        }
    }
    else
    {
        for (const auto storeId : storeIds)
        {
            std::shared_ptr<DataStore> store = DataServe::getStore(storeId);
            if (store->isValueSet("uniq_bsid"))
            {
                const std::vector<std::string> uniqBsids = store->getValue("uniq_bsid");
                /// @todo Refactor code to avoid this "multiple_uniq_bsids".
                if (not store->isValueSet("multiple_uniq_bsids"))
                {
                    // This is the normal case, for master and thumb writers, etc.
                    const uint32_t uniqBsid = std::stoi(uniqBsids.at(0));
                    if (uniqBsid == mPrimaryItemUniqBsid)
                    {
                        const std::vector<std::string> itemIds = store->getValue("item_indx");
                        if (itemIds.size() >= mPrimaryItemIndex)
                        {
                            itemId = std::stoi(itemIds.at(mPrimaryItemIndex - 1));
                            break;
                        }
                        else
                        {
                            throw std::runtime_error("Invalid Primary Item index.");
                        }
                    }
                }
                else
                {
                    // Several uniq_bsids means this is from a derived item writer. Access it differently.
                    /// @todo Refactor code to avoid this.
                    unsigned int index = 0;
                    for (const auto& bsidString : uniqBsids)
                    {
                        if (std::stoi(bsidString) == static_cast<int>(mPrimaryItemUniqBsid))
                        {
                            ++index;
                            if (index == mPrimaryItemIndex)
                            {
                                const std::vector<std::string> itemIds = store->getValue("item_indx");
                                if (itemIds.size() < index)
                                {
                                    throw std::runtime_error("Invalid Primary Item index.");
                                }
                                itemId = std::stoi(itemIds.at(index - 1));
                                break;
                            }
                        }
                    }
                }
            }
        }
    }

    if (itemId == 0)
    {
        logWarning() << "Unique bsid for primary item not found, not writing Primary Item." << std::endl;
    }

    metaBox->setPrimaryItem(itemId);
}
