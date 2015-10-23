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

#include "metabox.hpp"

MetaBox::MetaBox() :
    FullBox("meta", 0, 0),
    mHandlerBox(),
    mPrimaryItemBox(),
    mDataInformationBox(),
    mItemLocationBox(),
    mItemProtectionBox(),
    mItemInfoBox(2),
    mItemReferenceBox(),
    mItemDataBox(),
    mItemPropertiesBox(),
    mGroupsListBox()
{
}

std::string MetaBox::getHandlerType() const
{
    return mHandlerBox.getHandlerType();
}

void MetaBox::setHandlerType(const std::string& handler)
{
    mHandlerBox.setHandlerType(handler);
}

const PrimaryItemBox& MetaBox::getPrimaryItemBox() const
{
    return mPrimaryItemBox;
}

void MetaBox::setPrimaryItem(const std::uint16_t itemId)
{
    mPrimaryItemBox.setItemId(itemId);

    if (mItemLocationBox.hasItemIdEntry(itemId))
    {
        auto urlBox = std::make_shared<DataEntryUrlBox>();
        urlBox->setFlags(1); // Flag 0x01 tells the data is in this file. DataEntryUrlBox will write only its header.
        const unsigned int index = mDataInformationBox.addDataEntryBox(urlBox);
        mItemLocationBox.setItemDataReferenceIndex(itemId, index);
    }
}

const ItemInfoBox& MetaBox::getItemInfoBox() const
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


void MetaBox::addProperty(std::shared_ptr<Box> box, const std::vector<std::uint32_t>& itemIds, const bool essential)
{
    mItemPropertiesBox.addProperty(box, itemIds, essential);
}


void MetaBox::addProperty(const unsigned int index, const std::vector<std::uint32_t>& itemIds, const bool essential)
{
    mItemPropertiesBox.associateProperty(index, itemIds, essential);
}


const GroupsListBox& MetaBox::getGroupsListBox() const
{
    return mGroupsListBox;
}

void MetaBox::addAlternateGroup(const std::vector<std::uint32_t>& itemIds)
{
    EntityToGroupBox entityToGroupBox;
    entityToGroupBox.setEntityIds(itemIds);
    mGroupsListBox.addEntityToGroupBox(entityToGroupBox);
}

const DataInformationBox& MetaBox::getDataInformationBox() const
{
    return mDataInformationBox;
}

void MetaBox::addItemReference(const std::string& type, const std::uint16_t fromId, const std::uint16_t toId)
{
    mItemReferenceBox.add(type, fromId, toId);
}

void MetaBox::addIloc(const std::uint16_t itemId, const std::uint32_t offset, const std::uint32_t length,
    const std::uint32_t baseOffset)
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

void MetaBox::addItem(const std::uint16_t itemId, const std::string& type, const std::string& name, const bool hidden)
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

void MetaBox::addMdatItem(const std::uint16_t itemId, const std::string& type, const std::string& name,
    const std::uint32_t baseOffset)
{
    addItem(itemId, type, name);

    ItemLocation itemLocation;
    itemLocation.setItemID(itemId);
    itemLocation.setBaseOffset(baseOffset);
    itemLocation.setConstructionMethod(ItemLocation::ConstructionMethod::FILE_OFFSET);
    mItemLocationBox.addLocation(itemLocation);
}

void MetaBox::addItemExtent(const std::uint16_t itemId, const std::uint32_t offset, const std::uint32_t length)
{
    ItemLocationExtent locationExtent;
    locationExtent.mExtentOffset = offset;
    locationExtent.mExtentLength = length;
    mItemLocationBox.addExtent(itemId, locationExtent);
}


void MetaBox::addIdatItem(const std::uint16_t itemId, const std::string& type, const std::string& name,
    const std::vector<uint8_t>& data)
{
    const unsigned int offset = mItemDataBox.addData(data);
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

void MetaBox::addItemIdatExtent(const std::uint16_t itemId, const std::vector<uint8_t>& data)
{
    const unsigned int offset = mItemDataBox.addData(data);
    ItemLocationExtent locationExtent;
    locationExtent.mExtentOffset = offset;
    locationExtent.mExtentLength = data.size();
    mItemLocationBox.addExtent(itemId, locationExtent);
}

const ItemDataBox& MetaBox::getItemDataBox() const
{
    return mItemDataBox;
}

const ProtectionSchemeInfoBox& MetaBox::getProtectionSchemeInfoBox(size_t index) const
{
    return mItemProtectionBox.getEntry(index);
}

void MetaBox::writeBox(BitStream& bitstr)
{
    writeFullBoxHeader(bitstr);

    mHandlerBox.writeBox(bitstr);
    mPrimaryItemBox.writeBox(bitstr);
    mDataInformationBox.writeBox(bitstr);
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

void MetaBox::parseBox(BitStream& bitstr)
{
    parseFullBoxHeader(bitstr);

    while (bitstr.numBytesLeft() > 0)
    {
        std::string boxType;
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
