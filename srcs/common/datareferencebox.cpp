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

#include "datareferencebox.hpp"
#include "log.hpp"

#include <stdexcept>

DataEntryBox::DataEntryBox(FourCCInt boxType, const std::uint8_t version, const std::uint32_t flags)
    : FullBox(boxType, version, flags)
    , mLocation()
{
}

void DataEntryBox::setLocation(const String& location)
{
    mLocation = location;
}

const String DataEntryBox::getLocation() const
{
    return mLocation;
}

DataEntryUrlBox::DataEntryUrlBox(IsSelfContained isSelfContained)
    : DataEntryBox("url ", 0, isSelfContained == NotSelfContained ? 0 : 1)
{
}

void DataEntryUrlBox::writeBox(ISOBMFF::BitStream& bitstr) const
{
    writeFullBoxHeader(bitstr);

    // If the self-contained flag is set, no string is present. The box terminates with the entry-flags field.
    // This form is supported only by DataEntryUrlBox, not DataEntryUrnBox.
    if (!(getFlags() & 1))
    {
        bitstr.writeZeroTerminatedString(getLocation());
    }

    updateSize(bitstr);
}

void DataEntryUrlBox::parseBox(ISOBMFF::BitStream& bitstr)
{
    parseFullBoxHeader(bitstr);

    // See comment in DataEntryUrlBox::writeBox()
    if (!(getFlags() & 1))
    {
        String location;
        bitstr.readZeroTerminatedString(location);
        setLocation(location);
    }
}

DataEntryUrnBox::DataEntryUrnBox()
    : DataEntryBox("urn ", 0, 0)
    , mName()
{
}

void DataEntryUrnBox::setName(const String& name)
{
    mName = name;
}

const String DataEntryUrnBox::getName() const
{
    return mName;
}

void DataEntryUrnBox::writeBox(ISOBMFF::BitStream& bitstr) const
{
    writeFullBoxHeader(bitstr);
    bitstr.writeZeroTerminatedString(mName);
    bitstr.writeZeroTerminatedString(getLocation());
    updateSize(bitstr);
}

void DataEntryUrnBox::parseBox(ISOBMFF::BitStream& bitstr)
{
    parseFullBoxHeader(bitstr);
    bitstr.readZeroTerminatedString(mName);
    String location;
    bitstr.readZeroTerminatedString(location);
    setLocation(location);
}

DataReferenceBox::DataReferenceBox()
    : FullBox("dref", 0, 0)
    , mDataEntries()
{
}

unsigned int DataReferenceBox::addEntry(std::shared_ptr<DataEntryBox> dataEntryBox)
{
    mDataEntries.push_back(dataEntryBox);
    return static_cast<unsigned int>(mDataEntries.size());
}

void DataReferenceBox::writeBox(ISOBMFF::BitStream& bitstr) const
{
    writeFullBoxHeader(bitstr);

    bitstr.write32Bits(static_cast<std::uint32_t>(mDataEntries.size()));
    for (auto& entry : mDataEntries)
    {
        entry->writeBox(bitstr);
    }

    updateSize(bitstr);
}

void DataReferenceBox::parseBox(ISOBMFF::BitStream& bitstr)
{
    parseFullBoxHeader(bitstr);

    const unsigned int entryCount = bitstr.read32Bits();
    for (unsigned int i = 0; i < entryCount; ++i)
    {
        FourCCInt boxType;
        BitStream subBitStream = bitstr.readSubBoxBitStream(boxType);

        std::shared_ptr<DataEntryBox> dataEntryBox;
        if (boxType == "urn ")
        {
            dataEntryBox = makeCustomShared<DataEntryUrnBox>();
            dataEntryBox->parseBox(subBitStream);
        }
        else if (boxType == "url ")
        {
            dataEntryBox = makeCustomShared<DataEntryUrlBox>();
            dataEntryBox->parseBox(subBitStream);
        }
        else
        {
            throw RuntimeError("An unknown box inside dref");
        }
        mDataEntries.push_back(dataEntryBox);
    }
}
