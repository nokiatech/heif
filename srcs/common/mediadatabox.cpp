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

#include "mediadatabox.hpp"

#include <stdexcept>

using namespace std;

MediaDataBox::MediaDataBox()
    : Box("mdat")
    , mData()
    , mDataOffsetArray()
    , mDataLengthArray()
{
    writeBoxHeader(mData);  // write Box header
}

const Vector<uint8_t>& MediaDataBox::getData() const
{
    return mData.getStorage();
}

void MediaDataBox::writeBox(ISOBMFF::BitStream& bitstr) const
{
    const Vector<uint8_t>& data = mData.getStorage();
    bitstr.write8BitsArray(data, data.size());
}

void MediaDataBox::parseBox(BitStream& /*bitstr*/)
{
    // Intentionally empty
}

std::uint64_t MediaDataBox::addData(const Vector<uint8_t>& srcData)
{
    std::uint64_t offset = mData.getSize();  // offset from the beginning of the box (mData contains box header already)

    mDataOffsetArray.push_back(offset);          // current offset
    mDataLengthArray.push_back(srcData.size());  // length of the data to be added

    mData.write8BitsArray(srcData, srcData.size());  // add media data to bitstream

    updateSize(mData);

    return offset;
}

void MediaDataBox::addNalData(const Vector<Vector<uint8_t>>& srcData)
{
    std::uint64_t totalLen = 0;

    size_t dataOffset = mData.getSize();  // data offset for the image within the media data box

    for (const auto& i : srcData)
    {
        addNalData(i);
        totalLen += mDataLengthArray.back();
        mDataOffsetArray.pop_back();
        mDataLengthArray.pop_back();
    }

    mDataOffsetArray.push_back(static_cast<std::uint64_t>(dataOffset));
    mDataLengthArray.push_back(totalLen);  // total length of the data added
}

void MediaDataBox::addNalData(const Vector<uint8_t>& srcData)
{
    std::uint64_t startCodeLen;
    std::uint64_t startCodePos;
    std::uint64_t currPos  = 0;
    std::uint64_t totalLen = 0;

    mDataOffsetArray.push_back(static_cast<std::uint64_t>(mData.getSize()));  // record offset for the picture to be added

    // replace start codes with nal length fields
    startCodeLen = findStartCode(srcData, 0, startCodePos);
    currPos += startCodeLen;
    while (currPos < srcData.size())
    {
        startCodeLen               = findStartCode(srcData, currPos, startCodePos);  // find next start code to determine NAL unit size
        const std::uint64_t nalLen = startCodePos - currPos;

        mData.write32Bits(static_cast<uint32_t>(nalLen));  // write length field
        mData.write8BitsArray(srcData, nalLen, currPos);   // write data (except for the start code)

        currPos = startCodePos + startCodeLen;
        totalLen += (nalLen + 4);
    }

    mDataLengthArray.push_back(totalLen);  // total length of the data added

    updateSize(mData);
}

std::uint64_t MediaDataBox::findStartCode(const Vector<uint8_t>& srcData, const std::uint64_t searchStartPos,
                                          std::uint64_t& startCodePos)
{
    std::uint64_t i          = searchStartPos;
    std::uint64_t len        = 0;
    bool startCodeFound      = false;
    const size_t srcDataSize = srcData.size();

    while (i < srcDataSize && !startCodeFound)
    {
        const uint8_t byte = srcData[i];
        if (byte == 0)
        {
            ++len;
        }
        else if (len > 1 && byte == 1)
        {
            ++len;
            startCodeFound = true;
        }
        else
        {
            len = 0;
        }
        ++i;
    }

    if (startCodeFound)
    {
        startCodePos = i - len;
    }
    else
    {
        startCodePos = i;
        len          = 0;
    }

    return len;
}
