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

#include "services.hpp"
#include "datastore.hpp"

DataServe::ServeMap DataServe::mServeMap;

std::shared_ptr<DataStore> DataServe::getStore(std::uint32_t storeId)
{
    return mServeMap.at(storeId);
}

void DataServe::regStore(std::uint32_t storeId, std::shared_ptr<DataStore> dataStore)
{
    mServeMap.insert(make_pair(storeId, dataStore));
}

std::vector<std::uint32_t> DataServe::getStoreIds()
{
    std::vector<std::uint32_t> storeIds;

    for (auto entry : mServeMap)
    {
        storeIds.push_back(entry.first);
    }

    return storeIds;
}

void DataServe::clear()
{
    mServeMap.clear();
}
