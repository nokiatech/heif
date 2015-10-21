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

#ifndef DATASTORE_HPP
#define DATASTORE_HPP

#include <map>
#include <string>
#include <vector>

/** @brief A class for storing key and value vector pairs.
 *  @todo Code should be refactored so DataStore::setValue() special behavior for UINT32_MAX_STRING would not be needed. */
class DataStore
{
public:
    /** Remove all elements from DataStore. */
    void clear();

    /** @brief Set a new value to DataStore.
     *  @param [in] key Key to set. If key already exists, the value will be appended to the value vector.
     *  @param [in] value New value to append. If value equals to std::to_string(std::numeric_limits<std::uint32_t>::max())
     *                    it will be not added. */
    void setValue(const std::string& key, const std::string& value);

    /** Check if key and value(s) exists in the DataStore
     * @return True if key exists in store, false otherwise */
    bool isValueSet(const std::string& key) const;

    /** Get value(s) mapped to the key.
     *  @param [in] key Key of values to retrieve
     *  @return Vector of one or more element.
     *  @throws std::runtime_error if the key is not found. */
    std::vector<std::string> getValue(const std::string& key) const;

private:
    typedef std::map<std::string, std::vector<std::string>> ValueMap;
    ValueMap mValueMap; ///< Storage for values
};

#endif /* end of include guard: DATASTORE_HPP */
