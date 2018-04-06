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

#include "editbox.hpp"
#include "log.hpp"

#include <stdexcept>

EditBox::EditBox()
    : Box("edts")
    , mEditListBox(nullptr)
{
}

void EditBox::writeBox(ISOBMFF::BitStream& bitstr) const
{
    writeBoxHeader(bitstr);
    if (mEditListBox != nullptr)
    {
        mEditListBox->writeBox(bitstr);
    }
    updateSize(bitstr);
}

void EditBox::parseBox(ISOBMFF::BitStream& bitstr)
{
    parseBoxHeader(bitstr);

    // if there a data available in the file
    while (bitstr.numBytesLeft() > 0)
    {
        // Extract contained box bitstream and type
        FourCCInt boxType;
        BitStream subBitstr = bitstr.readSubBoxBitStream(boxType);

        // Handle this box based on the type
        if (boxType == "elst")
        {
            mEditListBox = makeCustomShared<EditListBox>();
            mEditListBox->parseBox(subBitstr);
        }
    }
}

void EditBox::setEditListBox(std::shared_ptr<EditListBox> editListBox)
{
    mEditListBox = std::move(editListBox);
}

const EditListBox* EditBox::getEditListBox() const
{
    return mEditListBox.get();
}

EditListBox::EditListBox()
    : FullBox("elst", 0, 0)
{
}

void EditListBox::addEntry(const EntryVersion0& entry)
{
    if (mEntryVersion1.size() != 0 || getVersion() != 0)
    {
        throw RuntimeError("Invalid attempt to add version0 EditListBox entries.");
    }
    mEntryVersion0.push_back(entry);
}

void EditListBox::addEntry(const EntryVersion1& entry)
{
    if (mEntryVersion0.size() != 0 || getVersion() != 1)
    {
        throw RuntimeError("Invalid attempt to add version1 EditList entries.");
    }
    mEntryVersion1.push_back(entry);
}

std::uint32_t EditListBox::numEntry() const
{
    size_t size = 0;
    switch (getVersion())
    {
    case 0:
        size = mEntryVersion0.size();
        break;
    case 1:
        size = mEntryVersion1.size();
        break;
    default:
        throw RuntimeError("Not supported EditListBox entry version (only 0 and 1 are supported).");
    }
    return static_cast<std::uint32_t>(size);
}

void EditListBox::writeBox(ISOBMFF::BitStream& bitstr) const
{
    writeFullBoxHeader(bitstr);
    if (mEntryVersion0.empty() == false)
    {
        bitstr.write32Bits(static_cast<std::uint32_t>(mEntryVersion0.size()));
        for (const auto& entry : mEntryVersion0)
        {
            bitstr.write32Bits(entry.mSegmentDuration);
            bitstr.write32Bits(static_cast<unsigned int>(entry.mMediaTime));
            bitstr.write16Bits(entry.mMediaRateInteger);
            bitstr.write16Bits(entry.mMediaRateFraction);
        }
    }
    else if (mEntryVersion1.empty() == false)
    {
        bitstr.write32Bits(static_cast<std::uint32_t>(mEntryVersion1.size()));
        for (const auto& entry : mEntryVersion0)
        {
            bitstr.write64Bits(entry.mSegmentDuration);
            bitstr.write64Bits(static_cast<std::uint64_t>(entry.mMediaTime));
            bitstr.write16Bits(entry.mMediaRateInteger);
            bitstr.write16Bits(entry.mMediaRateFraction);
        }
    }
    updateSize(bitstr);
}

void EditListBox::parseBox(ISOBMFF::BitStream& bitstr)
{
    parseFullBoxHeader(bitstr);

    const std::uint32_t entryCount = bitstr.read32Bits();

    if (getVersion() == 0)
    {
        for (std::uint32_t i = 0; i < entryCount; ++i)
        {
            EntryVersion0 entryVersion0;
            entryVersion0.mSegmentDuration   = bitstr.read32Bits();
            entryVersion0.mMediaTime         = static_cast<std::int32_t>(bitstr.read32Bits());
            entryVersion0.mMediaRateInteger  = bitstr.read16Bits();
            entryVersion0.mMediaRateFraction = bitstr.read16Bits();
            mEntryVersion0.push_back(entryVersion0);
        }
    }
    else if (getVersion() == 1)
    {
        for (uint32_t i = 0; i < entryCount; ++i)
        {
            EntryVersion1 entryVersion1;
            entryVersion1.mSegmentDuration   = bitstr.read64Bits();
            entryVersion1.mMediaTime         = static_cast<std::int64_t>(bitstr.read64Bits());
            entryVersion1.mMediaRateInteger  = bitstr.read16Bits();
            entryVersion1.mMediaRateFraction = bitstr.read16Bits();
            mEntryVersion1.push_back(entryVersion1);
        }
    }
}
