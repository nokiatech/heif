/* This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2018 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its subsidiaries. All rights are reserved.
 *
 * Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior written consent of Nokia.
 */

#ifndef ITEMPROPERTYASSOCIATION_HPP
#define ITEMPROPERTYASSOCIATION_HPP

#include "customallocator.hpp"
#include "fullbox.hpp"


/** Item Property Association class.
 * @details 'ipma' box implementation.
 * Version 0 and 1 are are supported. */
class ItemPropertyAssociation : public FullBox
{
public:
    ItemPropertyAssociation();
    virtual ~ItemPropertyAssociation() = default;

    /** A property association entry */
    struct Entry
    {
        bool essential;       ///< True indicates that this property is essential to the item, otherwise it is non-essential.
        std::uint16_t index;  ///< 1-based index of the property.
    };
    typedef Vector<Entry> AssociationEntries;

    /** Get association information related to an item id.
     * @param [in] itemId The item id to request property indexes for.
     * @return Requested association entries. An empty vector if the itemId is not present in ItemPropertyAssociation.
     */
    const AssociationEntries& getAssociationEntries(std::uint32_t itemId) const;

    /** Add a property association for an item.
     * @param [in] itemId Item ID of the item.
     * @param [in] index 1-based index of the property to associate from the Item Property Container Box. Value 0 indicates no property is associated.
     * @param [in] essential True if the property is essential for the item (reader is required to process it).
     */
    void addEntry(std::uint32_t itemId, std::uint16_t index, bool essential);

    /** Write box to ISOBMFF::BitStream.
     *  @see Box::writeBox() */
    virtual void writeBox(ISOBMFF::BitStream& bitstream) const;

    /** Read box from ISOBMFF::BitStream.
     *  @see Box::parseBox() */
    virtual void parseBox(ISOBMFF::BitStream& bitstream);

private:
    typedef std::uint32_t ItemId;
    Map<ItemId, AssociationEntries> mAssociations;
};

#endif
