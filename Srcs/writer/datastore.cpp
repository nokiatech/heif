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

#include "datastore.hpp"
#include <limits>
#include <stdexcept>

#define ANDROID_TO_STRING_HACK
#include "androidhacks.hpp"

void DataStore::clear()
{
    mValueMap.clear();
}

void DataStore::setValue(const std::string& key, const std::string& value)
{
    static const std::string UINT32_MAX_STRING = std::to_string(std::numeric_limits<std::uint32_t>::max());
    if (value == UINT32_MAX_STRING)
    {
        // Value is invalid/not set, so do not store it.
        return;
    }

    mValueMap[key].push_back(value);
}

std::vector<std::string> DataStore::getValue(const std::string& key) const
{
    if (not mValueMap.count(key))
    {
        throw std::runtime_error("DataStore::getValue() access error: key '" + key + "' not found.");
    }
    return mValueMap.at(key);
}

bool DataStore::isValueSet(const std::string& key) const
{
    return mValueMap.count(key);
}
