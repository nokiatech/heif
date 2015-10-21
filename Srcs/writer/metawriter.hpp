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

#ifndef METAWRITER_HPP
#define METAWRITER_HPP

#include "context.hpp"
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

class DataStore;
class MetaBox;

/**
 * @brief Abstract base class for MetaBox writers
 * @details MetaWriter derived MetaBox writers are used add image item information, Properties, or other entries or
 * boxes to a MetaBox. Usually MetaWriter should add only related metadata, while a corresponding MediaWriter derived
 * class handles writing the data itself.
 * @todo Add a constructor without handlerType, or refactor handler type writing elsewhere. */
class MetaWriter
{
public:
    MetaWriter();

    /**
     * @brief Constructor of MetaWriter base class
     * @param [in] handlerType
     * @param [in] contextId Context ID of this meta writer.
     */
    MetaWriter(const std::string& handlerType, std::uint32_t contextId);
    virtual ~MetaWriter() = default;

    /**
     * Add data to metaBox. Functionality should depend on configuration provided to the derived MetaWriter.
     * @param [in,out] metaBox Pointer to the MetaBox where to add data.
     */
    virtual void write(MetaBox* metaBox) = 0;

    /**
     * @brief Set the base offset of of this MetaWriter. Usually this refers to offset of 'mdat' box in the file.
     * @details Base offset in file is possibly not known when MetaWriter is called for first time. Final values
     * of some fields might need to be re-written after final base offset is known.
     * @param [in] offset Offset value.
     */
    void setBaseOffset(std::uint32_t offset);

    /**
     * @brief Link DataStore of the master context
     * @param [in] masterContextId ID of the master context/writer.
     * @todo See if this should/could be refactored as most derived classes are not using this.
     */
    void linkMasterDataStore(ContextId masterContextId);

protected:
    /**
     * @brief Get base offset
     * @see setBaseOffset()
     */
    std::uint32_t getBaseOffset() const;

    /**
     * @brief Store a value to the DataStore of this ImageWriter
     * @param [in] key Key in the DataStore
     * @param [in] value Value string to add
     */
    void storeValue(const std::string& key, const std::string& value);

    /**
     * @brief Clear the internal DataStore of ImageWriter.
     */
    void initWrite();

    /**
     * @brief Write handler type to the HandlerBox
     * @param [in,out] metaBox Pointer to the MetaBox where to add data.
     */
    void hdlrWrite(MetaBox* metaBox);

    /**
     * @brief Retrieve values from the DataStore of master context
     * @param [in] key Key in the DataStore
     * @return Vector of string values
     * @throws std::runtime_error in case key is not found.
     */
    std::vector<std::string> getMasterStoreValue(const std::string& key) const;

private:
    std::shared_ptr<DataStore> mMasterStore; /**< Thumbs track will have a link to masters store */
    std::shared_ptr<DataStore> mInternStore; /**< Every track registers its internal store */
    std::uint32_t mBaseOffset = 0; ///< Base offset in bytes, @see setBaseOffset() for more detail.
    std::string mHandlerType;   /**< Handler type */
};

#endif /* end of include guard: METAWRITER_HPP */

