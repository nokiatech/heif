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

#include "datareferencebox.hpp"
#include "log.hpp"

#include <stdexcept>

DataEntryBox::DataEntryBox(const FourCCInt boxType, const std::uint8_t version, const std::uint32_t flags) :
    FullBox(boxType, version, flags),
    mLocation()
{
}

void DataEntryBox::setLocation(const std::string& location)
{
    mLocation = location;
}

const std::string DataEntryBox::getLocation() const
{
    return mLocation;
}

DataEntryUrlBox::DataEntryUrlBox() :
    DataEntryBox("url ", 0, 0)
{
}

void DataEntryUrlBox::writeBox(BitStream& bitstr)
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

void DataEntryUrlBox::parseBox(BitStream& bitstr)
{
    parseFullBoxHeader(bitstr);

    // See comment in DataEntryUrlBox::writeBox()
    if (!(getFlags() & 1))
    {
        std::string location;
        bitstr.readZeroTerminatedString(location);
        setLocation(location);
    }
}

DataEntryUrnBox::DataEntryUrnBox() :
    DataEntryBox("urn ", 0, 0),
    mName()
{
}

void DataEntryUrnBox::setName(const std::string& name)
{
    mName = name;
}

const std::string DataEntryUrnBox::getName() const
{
    return mName;
}

void DataEntryUrnBox::writeBox(BitStream& bitstr)
{
    writeFullBoxHeader(bitstr);
    bitstr.writeZeroTerminatedString(mName);
    bitstr.writeZeroTerminatedString(getLocation());
    updateSize(bitstr);
}

void DataEntryUrnBox::parseBox(BitStream& bitstr)
{
    parseFullBoxHeader(bitstr);
    bitstr.readZeroTerminatedString(mName);
    std::string location;
    bitstr.readZeroTerminatedString(location);
    setLocation(location);
}

DataReferenceBox::DataReferenceBox() :
    FullBox("dref", 0, 0),
    mDataEntries()
{
}

unsigned int DataReferenceBox::addEntry(std::shared_ptr<DataEntryBox> dataEntryBox)
{
    mDataEntries.push_back(dataEntryBox);
    return mDataEntries.size();
}

void DataReferenceBox::writeBox(BitStream& bitstr)
{
    writeFullBoxHeader(bitstr);

    bitstr.write32Bits(mDataEntries.size());
    for (auto& entry : mDataEntries)
    {
        entry->writeBox(bitstr);
    }

    updateSize(bitstr);
}

void DataReferenceBox::parseBox(BitStream& bitstr)
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
            dataEntryBox = std::make_shared<DataEntryUrnBox>();
            dataEntryBox->parseBox(subBitStream);
        }
        else if (boxType == "url ")
        {
            dataEntryBox = std::make_shared<DataEntryUrlBox>();
            dataEntryBox->parseBox(subBitStream);
        }
        else
        {
            throw std::runtime_error("An unknown box inside dref");
        }
        mDataEntries.push_back(dataEntryBox);
    }
}
