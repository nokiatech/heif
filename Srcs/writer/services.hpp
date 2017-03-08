/* Copyright (c) 2015-2017, Nokia Technologies Ltd.
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

#ifndef SERVICES_HPP
#define SERVICES_HPP

#include <cstdint>
#include <map>
#include <memory>
#include <vector>

class DataStore;


/**
 * @brief DataServe class implementation.
 * @details This class handles data source registrations and Id spaces used by the file writer.
 */
class DataServe
{
public:
    DataServe() = default;
    virtual ~DataServe() = default;

    /// Clear all static data
    static void clear();

    /**
     * @brief Get a Data Store with a particular registered store Id.
     * @details Provides access to the DataStore registered under a particular Id.
     * @param [in] storeId Store ID value.
     * @returns pointer to the requested DataStore
     */
    static std::shared_ptr<DataStore> getStore(std::uint32_t storeId);

    /**
     * @brief Get a DataStore where is a key "key" with value "value".
     * @details If several DataStores match to search condition, only the first found will be returned.
     * @param [in] key Searched key in the store.
     * @param [in] value The value search from the key.
     * @returns Pointer to the requested DataStore. A nullptr in case the value was not found in any DataStore.
     */
    static std::shared_ptr<DataStore> getStore(const std::string& key, const std::string& value);

    /**
     * @brief Register a Data Store with a particular store Id.
     * @param [in] storeId Store ID value.
     * @param [in] dataStore Data Store object to be registered.
     */
    static void regStore(std::uint32_t storeId, std::shared_ptr<DataStore> dataStore);

    /**
     * @brief Get the list of registered store Ids.
     * @returns vector of store Ids
     */
    static std::vector<std::uint32_t> getStoreIds();

private:
    typedef std::map<std::uint32_t, std::shared_ptr<DataStore>> ServeMap; ///< Internal list of Data store - Id mappings
    static ServeMap mServeMap; ///< Static ServeMap object
};

#endif /* end of include guard: SERVICES_HPP */

