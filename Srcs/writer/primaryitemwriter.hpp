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

#ifndef PRIMARYITEMWRITER_HPP
#define PRIMARYITEMWRITER_HPP

#include "metawriter.hpp"

/**
 * @brief MetaBox writer for setting item ID to the Primary Item Box.
 * @details The primary item is given either as reference uniq_bsid/index pair to the constructor, or in case
 * either of those is unavailable, as a context ID of a meta master context. In later case the first found item is used.
 * @todo The class could be refactored (implementation, constructor interface)
 */
class PrimaryItemWriter : public MetaWriter
{
public:
    /**
    * @brief PrimaryItemWriter constructor
    * @param uniqBsid        uniq_bsid of the primary item, 0xffffffff if not set
    * @param itemIndex       1-based index of the primary item, 0xffffffff if not set
    * @param masterContextId Fallback meta master context ID in case either or both uniqBsid and itemIndex is not set
    */
    PrimaryItemWriter(uint32_t uniqBsid, uint32_t itemIndex, uint32_t masterContextId);
    virtual ~PrimaryItemWriter() = default;

    /**
     * @brief Add data to a MetaBox based on configuration data given during writer construction.
     * @see MetaWriter::write()
     */
    virtual void write(MetaBox* metaBox);

private:
    uint32_t mPrimaryItemUniqBsid; ///< Primary item uniq_bsid context reference
    uint32_t mPrimaryItemIndex;    ///< Primary item 1-based index

    /// Context ID of a master context, to search a potential primary item from in case uniq_bsid/index item is not available
    uint32_t mMasterContextId;
};

#endif /* end of include guard: PRIMARYITEMWRITER_HPP */

