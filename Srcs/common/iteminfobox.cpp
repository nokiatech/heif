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

#include "iteminfobox.hpp"

#include <stdexcept>

using namespace std;

ItemInfoBox::ItemInfoBox() :
    ItemInfoBox(0)
{
}

ItemInfoBox::ItemInfoBox(const uint8_t version) :
    FullBox("iinf", version, 0),
    mItemInfoList()
{
}

uint16_t ItemInfoBox::getEntryCount() const
{
    return mItemInfoList.size();
}

void ItemInfoBox::addItemInfoEntry(const ItemInfoEntry& infoEntry)
{
    mItemInfoList.push_back(infoEntry);
}

const ItemInfoEntry& ItemInfoBox::getItemInfoEntry(const int idx) const
{
    return mItemInfoList.at(idx);
}

ItemInfoEntry ItemInfoBox::getItemById(const uint16_t itemId) const
{
    for (const auto& item : mItemInfoList)
    {
        if (item.getItemID() == itemId)
        {
            return item;
        }
    }
    throw runtime_error("Requested ItemInfoEntry not found.");
}

void ItemInfoBox::clear()
{
    mItemInfoList.clear();
}

void ItemInfoBox::writeBox(BitStream& bitstr)
{
    writeFullBoxHeader(bitstr);

    if (getVersion() == 0)
    {
        bitstr.write16Bits(mItemInfoList.size());
    }
    else
    {
        bitstr.write32Bits(mItemInfoList.size());
    }

    for (auto& entry : mItemInfoList)
    {
        entry.writeBox(bitstr);
    }

    updateSize(bitstr);
}

void ItemInfoBox::parseBox(BitStream& bitstr)
{
    parseFullBoxHeader(bitstr);

    size_t entryCount = 0;

    if (getVersion() == 0)
    {
        entryCount = bitstr.read16Bits();
    }
    else
    {
        entryCount = bitstr.read32Bits();
    }

    for (size_t i = 0; i < entryCount; ++i)
    {
        ItemInfoEntry infoEntry;
        infoEntry.parseBox(bitstr);
        addItemInfoEntry(infoEntry);
    }
}

unsigned int ItemInfoBox::countNumberOfItems(const string& itemType)
{
    unsigned int numberOfItems = 0;

    for (const auto& entry : mItemInfoList)
    {
        if (entry.getItemType() == itemType)
        {
            ++numberOfItems;
        }
    }

    return numberOfItems;
}

// return item and its index for the specified itemType and itemID
ItemInfoEntry* ItemInfoBox::findItemWithTypeAndID(const string& itemType, const unsigned int itemID, unsigned int& index)
{
    ItemInfoEntry* entry = nullptr;
    unsigned int currIndex = 0;

    for (auto i = mItemInfoList.begin(); i != mItemInfoList.end(); ++i)
    {
        if (i->getItemType() == itemType)
        {
            if (i->getItemID() == itemID)
            {
                entry = &(*i);
                index = currIndex;
                break;
            }
            else
            {
                ++currIndex;
            }
        }
    }

    return entry;
}

ItemInfoEntry::ItemInfoEntry() :
    FullBox("infe", 0, 0),
    mItemID(0),
    mItemProtectionIndex(0),
    mItemName(),
    mContentType(),
    mContentEncoding(),
    mExtensionType(),
    mItemInfoExtension(nullptr),
    mItemType(),
    mItemUriType()
{
}

ItemInfoEntry::~ItemInfoEntry()
{
    delete mItemInfoExtension;
}

void ItemInfoEntry::setItemID(const uint16_t id)
{
    mItemID = id;
}

uint16_t ItemInfoEntry::getItemID() const
{
    return mItemID;
}

void ItemInfoEntry::setItemProtectionIndex(const uint16_t idx)
{
    mItemProtectionIndex = idx;
}

uint16_t ItemInfoEntry::getItemProtectionIndex() const
{
    return mItemProtectionIndex;
}

void ItemInfoEntry::setItemName(const string& name)
{
    mItemName = name;
}

const string& ItemInfoEntry::getItemName() const
{
    return mItemName;
}

void ItemInfoEntry::setContentType(const string& contentType)
{
    mContentType = contentType;
}

const string& ItemInfoEntry::getContentType() const
{
    return mContentType;
}

void ItemInfoEntry::setContentEncoding(const string& contentEncoding)
{
    mContentEncoding = contentEncoding;
}
const string& ItemInfoEntry::getContentEncoding() const
{
    return mContentEncoding;
}

void ItemInfoEntry::setExtensionType(const string& extensionType)
{
    mExtensionType = extensionType;
}

const string& ItemInfoEntry::getExtensionType() const
{
    return mExtensionType;
}

void ItemInfoEntry::setItemType(const string& itemType)
{
    mItemType = itemType;
}

const string& ItemInfoEntry::getItemType() const
{
    return mItemType;
}

void ItemInfoEntry::setItemUriType(const string& itemUriType)
{
    mItemUriType = itemUriType;
}
const string& ItemInfoEntry::getItemUriType() const
{
    return mItemUriType;
}

void ItemInfoEntry::writeBox(BitStream& bitstr)
{
    writeFullBoxHeader(bitstr);

    if (getVersion() == 0 || getVersion() == 1)
    {
        bitstr.write16Bits(mItemID);
        bitstr.write16Bits(mItemProtectionIndex);
        bitstr.writeZeroTerminatedString(mItemName);
        bitstr.writeZeroTerminatedString(mContentType);
        bitstr.writeZeroTerminatedString(mContentEncoding);
    }
    if (getVersion() == 1)
    {
        bitstr.writeString(mExtensionType);
        mItemInfoExtension->write(bitstr);
    }
    if (getVersion() == 2)
    {
        bitstr.write16Bits(mItemID);
        bitstr.write16Bits(mItemProtectionIndex);
        bitstr.writeString(mItemType);
        bitstr.writeZeroTerminatedString(mItemName);
        if (mItemType == "mime")
        {
            bitstr.writeZeroTerminatedString(mContentType);
            bitstr.writeZeroTerminatedString(mContentEncoding);
        }
        else if (mItemType == "uri ")
        {
            bitstr.writeZeroTerminatedString(mItemUriType);
        }
    }

    updateSize(bitstr);
}

void FDItemInfoExtension::write(BitStream& bitstr)
{
    bitstr.writeZeroTerminatedString(mContentLocation);
    bitstr.writeZeroTerminatedString(mContentMD5);
    bitstr.write32Bits((uint32_t) ((mContentLength >> 32) & 0xffffffff));
    bitstr.write32Bits((uint32_t) (mContentLength & 0xffffffff));
    bitstr.write32Bits((uint32_t) ((mTransferLength >> 32) & 0xffffffff));
    bitstr.write32Bits((uint32_t) (mTransferLength & 0xffffffff));
    bitstr.write8Bits(mEntryCount);
    for (unsigned int i = 0; i < mEntryCount; i++)
    {
        bitstr.write32Bits(mGroupID.at(i));
    }
}

void ItemInfoEntry::parseBox(BitStream& bitstr)
{
    parseFullBoxHeader(bitstr);

    if (getVersion() == 0 || getVersion() == 1)
    {
        mItemID = bitstr.read16Bits();
        mItemProtectionIndex = bitstr.read16Bits();
        bitstr.readZeroTerminatedString(mItemName);
        bitstr.readZeroTerminatedString(mContentType);
        bitstr.readZeroTerminatedString(mContentEncoding);
    }
    if (getVersion() == 1)
    {
        bitstr.readStringWithLen(mExtensionType, 4);
        FDItemInfoExtension *itemInfoExt = new FDItemInfoExtension;
        mItemInfoExtension = itemInfoExt;
        itemInfoExt->parse(bitstr);
    }
    if (getVersion() == 2)
    {
        mItemID = bitstr.read16Bits();
        mItemProtectionIndex = bitstr.read16Bits();
        bitstr.readStringWithLen(mItemType, 4);
        bitstr.readZeroTerminatedString(mItemName);
        if (mItemType == "mime")
        {
            bitstr.readZeroTerminatedString(mContentType);
            bitstr.readZeroTerminatedString(mContentEncoding);
        }
        else if (mItemType == "uri ")
        {
            bitstr.readZeroTerminatedString(mItemUriType);
        }
    }
}

void FDItemInfoExtension::parse(BitStream& bitstr)
{
    bitstr.readZeroTerminatedString(mContentLocation);
    bitstr.readZeroTerminatedString(mContentMD5);
    mContentLength = ((uint64_t) bitstr.read32Bits()) << 32;
    mContentLength += bitstr.read32Bits();
    mTransferLength = ((uint64_t) bitstr.read32Bits()) << 32;
    mTransferLength += bitstr.read32Bits();
    mEntryCount = bitstr.read8Bits();
    for (unsigned int i = 0; i < mEntryCount; i++)
    {
        mGroupID.at(i) = bitstr.read32Bits();
    }
}

ItemInfoEntry* ItemInfoBox::findItemWithType(const string& itemType, const unsigned int index)
{
    ItemInfoEntry* entry = nullptr;
    unsigned int currIndex = 0;

    for (auto i = mItemInfoList.begin(); i != mItemInfoList.end(); ++i)
    {
        if (i->getItemType() == itemType)
        {
            if (index == currIndex)
            {
                entry = &(*i);
                break;
            }
            else
            {
                ++currIndex;
            }
        }
    }

    return entry;
}

vector<ItemInfoEntry> ItemInfoBox::getItemsByType(const std::string& itemType) const
{
    vector<ItemInfoEntry> items;
    for (const auto& i : mItemInfoList)
    {
        if (i.getItemType() == itemType)
        {
            items.push_back(i);
        }
    }
    return items;
}

FDItemInfoExtension::FDItemInfoExtension() :
    mContentLocation(),
    mContentMD5(),
    mContentLength(0),
    mTransferLength(0),
    mEntryCount(0),
    mGroupID(256, 0)
{
}

void FDItemInfoExtension::setContentLocation(const string& location)
{
    mContentLocation = location;
}

const string& FDItemInfoExtension::getContentLocation()
{
    return mContentLocation;
}

void FDItemInfoExtension::setContentMD5(const string& md5)
{
    mContentMD5 = md5;
}

const string& FDItemInfoExtension::getContentMD5()
{
    return mContentMD5;
}

void FDItemInfoExtension::setContentLength(const uint64_t length)
{
    mContentLength = length;
}

uint64_t FDItemInfoExtension::getContentLength()
{
    return mContentLength;
}

void FDItemInfoExtension::setTranferLength(const uint64_t length)
{
    mTransferLength = length;
}

uint64_t FDItemInfoExtension::getTranferLength()
{
    return mTransferLength;
}

void FDItemInfoExtension::setNumGroupID(const uint8_t numID)
{
    mEntryCount = numID;
}

uint8_t FDItemInfoExtension::getNumGroupID()
{
    return mEntryCount;
}

void FDItemInfoExtension::setGroupID(const int idx, const uint32_t id)
{
    mGroupID.at(idx) = id;
}

uint32_t FDItemInfoExtension::getGroupID(const int idx)
{
    return mGroupID.at(idx);
}

