/* This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2018 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its
 * subsidiaries. All rights are reserved.
 *
 * Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior
 * written consent of Nokia.
 */

#include "metabox.hpp"

MetaBox::MetaBox()
    : FullBox("meta", 0, 0)
    , mHandlerBox()
    , mPrimaryItemBox()
    , mDataInformationBox()
    , mItemLocationBox()
    , mItemProtectionBox()
    , mItemInfoBox()
    , mItemReferenceBox()
    , mItemDataBox()
    , mItemPropertiesBox()
    , mGroupsListBox()
{
}

FourCCInt MetaBox::getHandlerType() const
{
    return mHandlerBox.getHandlerType();
}

void MetaBox::setHandlerType(FourCCInt handler)
{
    mHandlerBox.setHandlerType(handler);
}

const PrimaryItemBox& MetaBox::getPrimaryItemBox() const
{
    return mPrimaryItemBox;
}

void MetaBox::setPrimaryItem(const std::uint32_t itemId)
{
    mPrimaryItemBox.setItemId(itemId);
}

const ItemInfoBox& MetaBox::getItemInfoBox() const
{
    return mItemInfoBox;
}

ItemInfoBox& MetaBox::getItemInfoBox()
{
    return mItemInfoBox;
}

void MetaBox::setItemInfoBox(const ItemInfoBox& itemInfoBox)
{
    mItemInfoBox = itemInfoBox;
}

const ItemLocationBox& MetaBox::getItemLocationBox() const
{
    return mItemLocationBox;
}

const ItemReferenceBox& MetaBox::getItemReferenceBox() const
{
    return mItemReferenceBox;
}

const ItemPropertiesBox& MetaBox::getItemPropertiesBox() const
{
    return mItemPropertiesBox;
}

std::uint16_t MetaBox::addProperty(std::shared_ptr<Box> box, const Vector<std::uint32_t>& itemIds, const bool essential)
{
    return mItemPropertiesBox.addProperty(box, itemIds, essential);
}

void MetaBox::addProperty(const std::uint16_t index, const Vector<std::uint32_t>& itemIds, const bool essential)
{
    mItemPropertiesBox.associateProperty(index, itemIds, essential);
}

const GroupsListBox& MetaBox::getGroupsListBox() const
{
    return mGroupsListBox;
}

void MetaBox::addEntityGrouping(const FourCCInt type, const std::uint32_t groupId, const Vector<std::uint32_t>& itemIds)
{
    EntityToGroupBox entityToGroupBox(type);
    entityToGroupBox.setEntityIds(itemIds);
    entityToGroupBox.setGroupId(groupId);
    mGroupsListBox.addEntityToGroupBox(entityToGroupBox);
}

const DataInformationBox& MetaBox::getDataInformationBox() const
{
    return mDataInformationBox;
}

void MetaBox::addItemReference(const FourCCInt type, const std::uint32_t fromId, const std::uint32_t toId)
{
    mItemReferenceBox.add(type, fromId, toId);
}

void MetaBox::addIloc(const std::uint32_t itemId,
                      const std::uint64_t offset,
                      const std::uint64_t length,
                      const std::uint64_t baseOffset)
{
    ItemLocationExtent locationExtent;
    locationExtent.mExtentOffset = offset;
    locationExtent.mExtentLength = length;

    ItemLocation itemLocation;
    itemLocation.setItemID(itemId);
    itemLocation.setBaseOffset(baseOffset);
    itemLocation.addExtent(locationExtent);

    mItemLocationBox.addLocation(itemLocation);
}

void MetaBox::addItem(const std::uint32_t itemId, FourCCInt type, const String& name, const bool hidden)
{
    ItemInfoEntry infeBox;
    infeBox.setVersion(2);
    infeBox.setItemType(type);
    infeBox.setItemID(itemId);
    infeBox.setItemName(name);

    if (hidden)
    {
        // Set (flags & 1) == 1 to indicate the item is hidden.
        infeBox.setFlags(1);
    }

    mItemInfoBox.addItemInfoEntry(infeBox);
}

void MetaBox::addItem(const ItemInfoEntry& itemInfoEntry)
{
    mItemInfoBox.addItemInfoEntry(itemInfoEntry);
}

void MetaBox::addMdatItem(const std::uint32_t itemId,
                          FourCCInt type,
                          const String& name,
                          const std::uint64_t baseOffset)
{
    addItem(itemId, type, name);

    ItemLocation itemLocation;
    itemLocation.setItemID(itemId);
    itemLocation.setBaseOffset(baseOffset);
    itemLocation.setConstructionMethod(ItemLocation::ConstructionMethod::FILE_OFFSET);
    mItemLocationBox.addLocation(itemLocation);
}

void MetaBox::addItemExtent(const std::uint32_t itemId, const std::uint64_t offset, const std::uint64_t length)
{
    ItemLocationExtent locationExtent;
    locationExtent.mExtentOffset = offset;
    locationExtent.mExtentLength = length;
    mItemLocationBox.addExtent(itemId, locationExtent);
}

void MetaBox::addIdatItem(const std::uint32_t itemId, FourCCInt type, const String& name, const Vector<uint8_t>& data)
{
    const uint64_t offset = mItemDataBox.addData(data);
    addItem(itemId, type, name);
    ItemLocationExtent locationExtent;
    locationExtent.mExtentOffset = offset;
    locationExtent.mExtentLength = data.size();

    ItemLocation itemLocation;
    itemLocation.setItemID(itemId);
    itemLocation.addExtent(locationExtent);
    itemLocation.setConstructionMethod(ItemLocation::ConstructionMethod::IDAT_OFFSET);
    mItemLocationBox.addLocation(itemLocation);
}

void MetaBox::addItemIdatExtent(const std::uint32_t itemId, const Vector<uint8_t>& data)
{
    const uint64_t offset = mItemDataBox.addData(data);
    ItemLocationExtent locationExtent;
    locationExtent.mExtentOffset = offset;
    locationExtent.mExtentLength = data.size();
    mItemLocationBox.addExtent(itemId, locationExtent);
}

void MetaBox::setItemFileOffsetBase(const std::uint64_t baseOffset)
{
    auto& itemLocations = mItemLocationBox.getItemLocations();
    for (auto& iloc : itemLocations)
    {
        if (iloc.getConstructionMethod() == ItemLocation::ConstructionMethod::FILE_OFFSET)
        {
            iloc.setBaseOffset(baseOffset);
        }
    }
}

const ItemDataBox& MetaBox::getItemDataBox() const
{
    return mItemDataBox;
}

const ProtectionSchemeInfoBox& MetaBox::getProtectionSchemeInfoBox(std::uint16_t index) const
{
    return mItemProtectionBox.getEntry(index);
}

void MetaBox::setImageHidden(const std::uint32_t itemId, const bool isHidden)
{
    auto& infe = mItemInfoBox.getItemById(itemId);
    // bit 0 indicates the image is hidden
    if (isHidden)
    {
        infe.setFlags(infe.getFlags() | 1);
    }
    else
    {
        infe.setFlags(infe.getFlags() & ~1u);
    }
}

void MetaBox::writeBox(ISOBMFF::BitStream& bitstr) const
{
    writeFullBoxHeader(bitstr);

    mHandlerBox.writeBox(bitstr);
    mPrimaryItemBox.writeBox(bitstr);
    // The optional 'dinf' is currently not used, so it is not written:
    // mDataInformationBox.writeBox(bitstr);
    mItemLocationBox.writeBox(bitstr);
    mItemProtectionBox.writeBox(bitstr);
    mItemInfoBox.writeBox(bitstr);
    // Not writing optional box IPMPControlBox
    mItemReferenceBox.writeBox(bitstr);
    mItemDataBox.writeBox(bitstr);
    mItemPropertiesBox.writeBox(bitstr);
    mGroupsListBox.writeBox(bitstr);

    updateSize(bitstr);
}

void MetaBox::parseBox(ISOBMFF::BitStream& bitstr)
{
    parseFullBoxHeader(bitstr);

    while (bitstr.numBytesLeft() > 0)
    {
        FourCCInt boxType;
        BitStream subBitstr = bitstr.readSubBoxBitStream(boxType);

        if (boxType == "hdlr")
        {
            mHandlerBox.parseBox(subBitstr);
        }
        else if (boxType == "pitm")
        {
            mPrimaryItemBox.parseBox(subBitstr);
        }
        else if (boxType == "iloc")
        {
            mItemLocationBox.parseBox(subBitstr);
        }
        else if (boxType == "iinf")
        {
            mItemInfoBox.parseBox(subBitstr);
        }
        else if (boxType == "iref")
        {
            mItemReferenceBox.parseBox(subBitstr);
        }
        else if (boxType == "iprp")
        {
            mItemPropertiesBox.parseBox(subBitstr);
        }
        else if (boxType == "grpl")
        {
            mGroupsListBox.parseBox(subBitstr);
        }
        else if (boxType == "dinf")
        {
            mDataInformationBox.parseBox(subBitstr);
        }
        else if (boxType == "idat")
        {
            mItemDataBox.parseBox(subBitstr);
        }
        else if (boxType == "ipro")
        {
            mItemProtectionBox.parseBox(subBitstr);
        }
        // unsupported boxes are skipped
    }
}
