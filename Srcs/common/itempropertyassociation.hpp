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

#ifndef ITEMPROPERTYASSOCIATION_HPP
#define ITEMPROPERTYASSOCIATION_HPP

#include "fullbox.hpp"

#include <map>
#include <vector>

/** Item Property Association class.
 * @details 'ipma' box implementation as specified in the HEIF specification.
 * Version 0 and 1 are are supported. */
class ItemPropertyAssociation : public FullBox
{
public:
    ItemPropertyAssociation();
    virtual ~ItemPropertyAssociation() = default;

    /** A property association entry */
    struct Entry
    {
        bool essential; ///< True indicates that this property is essential to the item, otherwise it is non-essential.
        std::uint16_t index; ///< Index of the property. Based on flags either 7 or 15 bits are used.
    };
    typedef std::vector<Entry> AssociationEntries;

    /** Get association information related to an item id.
     * @param [in] itemId The item id to request property indexes for.
     * @return Requested association entries. An empty vector if the itemId is not present in ItemPropertyAssociation.
     */
    AssociationEntries getAssociationEntries(std::uint32_t itemId) const;

    /** Add a property association for an item.
     * @param [in] itemId Item ID of the item.
     * @param [in] index 1-based index of the property to associate.
     * @param [in] essential True if the property is essential for the item (reader is required to process it).
     */
    void addEntry(std::uint32_t itemId, std::uint32_t index, bool essential);

    /** Write box to BitStream.
     *  @see Box::writeBox() */
    virtual void writeBox(BitStream& bitstream);

    /** Read box from BitStream.
     *  @see Box::parseBox() */
    virtual void parseBox(BitStream& bitstream);

private:
    typedef std::uint32_t ItemId;
    std::map<ItemId, AssociationEntries> mAssociations;
};

#endif
