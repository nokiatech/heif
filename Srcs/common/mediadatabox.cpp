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

#include "mediadatabox.hpp"

#include <stdexcept>

using namespace std;

MediaDataBox::MediaDataBox() :
    Box("mdat"),
    mData(),
    mDataOffsetArray(),
    mDataLengthArray()
{
    writeBoxHeader(mData); // write Box header
}

const vector<uint8_t>& MediaDataBox::getData() const
{
    return mData.getStorage();
}

void MediaDataBox::writeBox(BitStream& bitstr)
{
    const vector<uint8_t>& data = mData.getStorage();
    bitstr.write8BitsArray(data, data.size());
}

void MediaDataBox::parseBox(BitStream& /*bitstr*/)
{
    // Intentionally empty
}

size_t MediaDataBox::addData(const vector<uint8_t>& srcData)
{
    const unsigned offset = mData.getSize(); // offset from the beginning of the box (mData contains box header already)

    mDataOffsetArray.push_back(offset);  // current offset
    mDataLengthArray.push_back(srcData.size());   // length of the data to be added

    mData.write8BitsArray(srcData, srcData.size());  // add media data to bitstream

    updateSize(mData);

    return offset;
}

void MediaDataBox::addNalData(const vector<vector<uint8_t>>& srcData)
{
    unsigned int totalLen = 0;

    unsigned int dataOffset = mData.getSize();  // data offset for the image within the media data box

    for (const auto& i : srcData)
    {
        addNalData(i);
        totalLen += mDataLengthArray.back();
        mDataOffsetArray.pop_back();
        mDataLengthArray.pop_back();
    }

    mDataOffsetArray.push_back(dataOffset);
    mDataLengthArray.push_back(totalLen);   // total length of the data added
}

void MediaDataBox::addNalData(const vector<uint8_t>& srcData)
{
    unsigned int startCodeLen;
    unsigned int startCodePos;
    unsigned int currPos = 0;
    unsigned int totalLen = 0;

    mDataOffsetArray.push_back(mData.getSize());  // record offset for the picture to be added

    // replace start codes with nal length fields
    startCodeLen = findStartCode(srcData, 0, startCodePos);
    currPos += startCodeLen;
    while (currPos < srcData.size())
    {
        startCodeLen = findStartCode(srcData, currPos, startCodePos); // find next start code to determine NAL unit size
        const unsigned int nalLen = startCodePos - currPos;

        mData.write32Bits(nalLen);  // write length field
        mData.write8BitsArray(srcData, nalLen, currPos);  // write data (except for the start code)

        currPos = startCodePos + startCodeLen;
        totalLen += (nalLen + 4);
    }

    mDataLengthArray.push_back(totalLen);   // total length of the data added

    updateSize(mData);
}

unsigned int MediaDataBox::findStartCode(const vector<uint8_t>& srcData, const unsigned int searchStartPos,
    unsigned int& startCodePos)
{
    unsigned int i = searchStartPos;
    unsigned int len = 0;
    bool startCodeFound = false;
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
        len = 0;
    }

    return len;
}
