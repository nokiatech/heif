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

#include "mediadatabox.hpp"

#include <fstream>
#include <limits>
#include <stdexcept>

using namespace std;

MediaDataBox::MediaDataBox()
    : Box("mdat")
    , mHeaderData()
    , mMediaData()
    , mTotalDataSize(0)
    , mDataOffsetArray()
    , mDataLengthArray()
{
    setLargeSize();
    writeBoxHeader(mHeaderData);  // write Box header
}

//void MediaDataBox::writeBox(std::ofstream& output) const
/*void MediaDataBox::writeBox(InternalOutputStream* output) const
{
    output->write(reinterpret_cast<const char*>(mHeaderData.getStorage().data()),
                 static_cast<streamsize>(mHeaderData.getStorage().size()));
    for (const auto& dataBlock : mMediaData)
    {
        output->write(reinterpret_cast<const char*>(dataBlock.data()), static_cast<streamsize>(dataBlock.size()));
    }
}*/

std::pair<const ISOBMFF::BitStream&, const List<Vector<uint8_t>>&> MediaDataBox::getSerializedData() const
{
    return { mHeaderData,mMediaData };
}

void MediaDataBox::writeBox(ISOBMFF::BitStream& bitstr) const
{
    const Vector<uint8_t>& data = mHeaderData.getStorage();
    bitstr.write8BitsArray(data, data.size());

    for (const auto& dataBlock : mMediaData)
    {
        bitstr.write8BitsArray(dataBlock, dataBlock.size());
    }
}

void MediaDataBox::parseBox(BitStream& /*bitstr*/)
{
    // Intentionally empty
}

void MediaDataBox::updateSize(ISOBMFF::BitStream& bitstr)
{
    uint64_t boxSize = mTotalDataSize + bitstr.getSize();
    setSize(boxSize);

    if ((boxSize > std::numeric_limits<std::uint32_t>::max()) && (getLargeSize() == false))
    {
        throw RuntimeError("Box::updateSize(): Box size exceeds 4GB but large size for 64-bit size field was not set.");
    }

    // Write updated size to the bitstream.
    if (getLargeSize())
    {
        bitstr.setByte(8, (boxSize >> 56) & 0xff);
        bitstr.setByte(9, (boxSize >> 48) & 0xff);
        bitstr.setByte(10, (boxSize >> 40) & 0xff);
        bitstr.setByte(11, (boxSize >> 32) & 0xff);
        bitstr.setByte(12, (boxSize >> 24) & 0xff);
        bitstr.setByte(13, (boxSize >> 16) & 0xff);
        bitstr.setByte(14, (boxSize >> 8) & 0xff);
        bitstr.setByte(15, boxSize & 0xff);
    }
    else
    {
        bitstr.setByte(0, (boxSize >> 24) & 0xff);
        bitstr.setByte(1, (boxSize >> 16) & 0xff);
        bitstr.setByte(2, (boxSize >> 8) & 0xff);
        bitstr.setByte(3, boxSize & 0xff);
    }
}

std::uint64_t MediaDataBox::addData(const Vector<uint8_t>& srcData)
{
    std::uint64_t offset =
        mHeaderData.getSize() + mTotalDataSize;  // offset from the beginning of the box (including header)

    mDataOffsetArray.push_back(offset);          // current offset
    mDataLengthArray.push_back(srcData.size());  // length of the data to be added

    mMediaData.push_back(srcData);
    mTotalDataSize += srcData.size();

    updateSize(mHeaderData);
    return offset;
}

std::uint64_t MediaDataBox::addData(const uint8_t* buffer, const uint64_t bufferSize)
{
    std::uint64_t offset =
        mHeaderData.getSize() + mTotalDataSize;  // offset from the beginning of the box (including header)

    mDataOffsetArray.push_back(offset);      // current offset
    mDataLengthArray.push_back(bufferSize);  // length of the data to be added

    // NOTE: casts should be (const uint8_t*) but clang + libc++ does the initialization in a non-optimal way if
    // constness changes
    // casting to (uint8_t*) allows the compiler to just do a memcpy.
    // does not affect GCC since it ALWAYS does init non-optimally.
    Vector<uint8_t> tmp(buffer, buffer + bufferSize);
    mMediaData.insert(mMediaData.end(), std::move(tmp));

    mTotalDataSize += bufferSize;

    updateSize(mHeaderData);
    return offset;
}

void MediaDataBox::addNalData(const Vector<Vector<uint8_t>>& srcData)
{
    std::uint64_t totalLen = 0;

    size_t dataOffset = mHeaderData.getSize() + mTotalDataSize;  // data offset for the image within the media data box

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

    mDataOffsetArray.push_back(static_cast<std::uint64_t>(
        mHeaderData.getSize() + mTotalDataSize));  // record offset for the picture to be added

    Vector<uint8_t> mediaDataEntry;
    mediaDataEntry.reserve(srcData.size());

    // replace start codes with nal length fields
    startCodeLen = findStartCode(srcData, 0, startCodePos);
    currPos += startCodeLen;
    while (currPos < srcData.size())
    {
        startCodeLen =
            findStartCode(srcData, currPos, startCodePos);  // find next start code to determine NAL unit size
        const std::uint64_t nalLen = startCodePos - currPos;

        // write length field
        mediaDataEntry.push_back(static_cast<uint8_t>((uint32_t(nalLen) >> 24) & 0xff));
        mediaDataEntry.push_back(static_cast<uint8_t>((uint32_t(nalLen) >> 16) & 0xff));
        mediaDataEntry.push_back(static_cast<uint8_t>((uint32_t(nalLen) >> 8) & 0xff));
        mediaDataEntry.push_back(static_cast<uint8_t>(uint32_t(nalLen) & 0xff));

        Vector<uint8_t>::const_iterator sourceIt =
            srcData.begin() + static_cast<Vector<uint8_t>::difference_type>(currPos);
        mediaDataEntry.insert(mediaDataEntry.end(), sourceIt,
                              sourceIt + static_cast<Vector<uint8_t>::difference_type>(nalLen));

        currPos = startCodePos + startCodeLen;
        totalLen += (nalLen + 4);
    }

    mMediaData.push_back(std::move(mediaDataEntry));
    mTotalDataSize += mMediaData.back().size();

    mDataLengthArray.push_back(totalLen);  // total length of the data added

    updateSize(mHeaderData);
}

std::uint64_t MediaDataBox::findStartCode(const Vector<uint8_t>& srcData,
                                          const std::uint64_t searchStartPos,
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
