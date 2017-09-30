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

#include "bitstream.hpp"
#include "log.hpp"

using namespace std;

BitStream::BitStream() :
    mStorage(),
    mCurrByte(0),
    mByteOffset(0),
    mBitOffset(0),
    mStorageAllocated(true)
{
}

BitStream::BitStream(const std::vector<std::uint8_t>& strData) :
    mStorage(strData),
    mCurrByte(0),
    mByteOffset(0),
    mBitOffset(0),
    mStorageAllocated(false)
{
}

BitStream::~BitStream()
{
    if (mStorageAllocated == true)
    {
        mStorage.clear();
    }
}

uint32_t BitStream::getPos() const
{
    return mByteOffset;
}

bool BitStream::isByteAligned() const
{
    return mBitOffset ? false : true;
}

void BitStream::setPosition(const unsigned int position)
{
    mByteOffset = position;
}

uint32_t BitStream::getSize() const
{
    return mStorage.size();
}

void BitStream::setSize(const unsigned int newSize)
{
    mStorage.resize(newSize);
}

const vector<uint8_t>& BitStream::getStorage() const
{
    return mStorage;
}

void BitStream::reset()
{
    mCurrByte = 0;
    mBitOffset = 0;
    mByteOffset = 0;
}

void BitStream::clear()
{
    mStorage.clear();
}

void BitStream::skipBytes(const unsigned int x)
{
    mByteOffset += x;
}

void BitStream::setByte(const unsigned int offset, const uint8_t byte)
{
    mStorage.at(offset) = byte;
}

uint8_t BitStream::getByte(const unsigned int offset) const
{
    return mStorage.at(offset);
}

unsigned int BitStream::numBytesLeft() const
{
    return mStorage.size() - mByteOffset;
}
void BitStream::extract(const int begin, const int end, BitStream& dest) const
{
    dest.clear();
    dest.reset();
    dest.mStorage.insert(dest.mStorage.begin(), mStorage.begin() + begin, mStorage.begin() + end);
}

void BitStream::write8Bits(const unsigned int bits)
{
    mStorage.push_back(static_cast<uint8_t>(bits));
}

void BitStream::write16Bits(const unsigned int bits)
{
    mStorage.push_back(static_cast<uint8_t>((bits >> 8) & 0xff));
    mStorage.push_back(static_cast<uint8_t>((bits) & 0xff));
}

void BitStream::write24Bits(const unsigned int bits)
{
    mStorage.push_back(static_cast<uint8_t>((bits >> 16) & 0xff));
    mStorage.push_back(static_cast<uint8_t>((bits >> 8) & 0xff));
    mStorage.push_back(static_cast<uint8_t>((bits) & 0xff));
}

void BitStream::write32Bits(const unsigned int bits)
{
    mStorage.push_back(static_cast<uint8_t>((bits >> 24) & 0xff));
    mStorage.push_back(static_cast<uint8_t>((bits >> 16) & 0xff));
    mStorage.push_back(static_cast<uint8_t>((bits >> 8) & 0xff));
    mStorage.push_back(static_cast<uint8_t>((bits) & 0xff));
}


void BitStream::write64Bits(const unsigned long long int bits)
{
    mStorage.push_back(static_cast<uint8_t>((bits >> 56) & 0xff));
    mStorage.push_back(static_cast<uint8_t>((bits >> 48) & 0xff));
    mStorage.push_back(static_cast<uint8_t>((bits >> 40) & 0xff));
    mStorage.push_back(static_cast<uint8_t>((bits >> 32) & 0xff));
    mStorage.push_back(static_cast<uint8_t>((bits >> 24) & 0xff));
    mStorage.push_back(static_cast<uint8_t>((bits >> 16) & 0xff));
    mStorage.push_back(static_cast<uint8_t>((bits >> 8) & 0xff));
    mStorage.push_back(static_cast<uint8_t>((bits) & 0xff));
}

void BitStream::write8BitsArray(const vector<uint8_t>& bits, const unsigned int len, const unsigned int srcOffset)
{
    mStorage.insert(mStorage.end(), bits.begin() + srcOffset, bits.begin() + srcOffset + len);
}

void BitStream::writeBits(const unsigned int bits, unsigned int len)
{
    do
    {
        const unsigned int numBitsLeftInByte = 8 - mBitOffset;
        if (numBitsLeftInByte > len)
        {
            mCurrByte = mCurrByte | ((bits & ((1 << len) - 1)) << (numBitsLeftInByte - len));
            mBitOffset += len;
            len = 0;
        }
        else
        {
            mCurrByte = mCurrByte | ((bits >> (len - numBitsLeftInByte)) & ((1 << numBitsLeftInByte) - 1));
            mStorage.push_back((uint8_t) mCurrByte);
            mBitOffset = 0;
            mCurrByte = 0;
            len -= numBitsLeftInByte;
        }
    }
    while (len > 0);
}

void BitStream::writeString(const string& srcString)
{
    if (srcString.length() == 0)
    {
        logWarning() << "BitStream::writeString called for zero-length string." << endl;
    }

    for (const auto character : srcString)
    {
        mStorage.push_back(character);
    }
}

void BitStream::writeZeroTerminatedString(const string& srcString)
{
    for (const auto character : srcString)
    {
        mStorage.push_back(character);
    }
    mStorage.push_back('\0');
}

uint8_t BitStream::read8Bits()
{
    const uint8_t ret = mStorage.at(mByteOffset);
    ++mByteOffset;
    return ret;
}

uint16_t BitStream::read16Bits()
{
    uint16_t ret = mStorage.at(mByteOffset);
    mByteOffset++;
    ret = (ret << 8) | mStorage.at(mByteOffset);
    mByteOffset++;
    return ret;
}

unsigned int BitStream::read24Bits()
{
    unsigned int ret = mStorage.at(mByteOffset);
    mByteOffset++;
    ret = (ret << 8) | mStorage.at(mByteOffset);
    mByteOffset++;
    ret = (ret << 8) | mStorage.at(mByteOffset);
    mByteOffset++;
    return ret;
}

unsigned int BitStream::read32Bits()
{
    unsigned int ret = mStorage.at(mByteOffset);
    mByteOffset++;
    ret = (ret << 8) | mStorage.at(mByteOffset);
    mByteOffset++;
    ret = (ret << 8) | mStorage.at(mByteOffset);
    mByteOffset++;
    ret = (ret << 8) | mStorage.at(mByteOffset);
    mByteOffset++;
    return ret;
}

unsigned long long int BitStream::read64Bits()
{
    unsigned long long int ret = mStorage.at(mByteOffset);
    mByteOffset++;
    ret = (ret << 8) | mStorage.at(mByteOffset);
    mByteOffset++;
    ret = (ret << 8) | mStorage.at(mByteOffset);
    mByteOffset++;
    ret = (ret << 8) | mStorage.at(mByteOffset);
    mByteOffset++;
    ret = (ret << 8) | mStorage.at(mByteOffset);
    mByteOffset++;
    ret = (ret << 8) | mStorage.at(mByteOffset);
    mByteOffset++;
    ret = (ret << 8) | mStorage.at(mByteOffset);
    mByteOffset++;
    ret = (ret << 8) | mStorage.at(mByteOffset);
    mByteOffset++;

    return ret;
}

void BitStream::read8BitsArray(vector<uint8_t>& bits, const unsigned int len)
{
    bits.insert(bits.end(), mStorage.begin() + mByteOffset, mStorage.begin() + mByteOffset + len);
    mByteOffset += len;
}

unsigned int BitStream::readBits(const int len)
{
    unsigned int returnBits = 0;
    int numBitsLeftInByte = 8 - mBitOffset;

    if (len == 0)
    {
        return 0;
    }

    if (numBitsLeftInByte >= len)
    {
        returnBits = ((mStorage).at(mByteOffset) >> (numBitsLeftInByte - len)) & ((1 << len) - 1);
        mBitOffset += len;
    }
    else
    {
        int numBitsToGo = len - numBitsLeftInByte;
        returnBits = (mStorage).at(mByteOffset) & ((1 << numBitsLeftInByte) - 1);
        mByteOffset++;
        mBitOffset = 0;
        while (numBitsToGo > 0)
        {
            if (numBitsToGo >= 8)
            {
                returnBits = (returnBits << 8) | (mStorage).at(mByteOffset);
                mByteOffset++;
                numBitsToGo -= 8;
            }
            else
            {
                returnBits = (returnBits << numBitsToGo)
                    | (((mStorage).at(mByteOffset) >> (8 - numBitsToGo)) & ((1 << numBitsToGo) - 1));
                mBitOffset += numBitsToGo;
                numBitsToGo = 0;
            }
        }
    }

    if (mBitOffset == 8)
    {
        mByteOffset++;
        mBitOffset = 0;
    }

    return returnBits;
}

void BitStream::readStringWithLen(string& dstString, const unsigned int len)
{
    dstString.clear();
    for (unsigned int i = 0; i < len; i++)
    {
        char currChar = read8Bits();
        dstString += currChar;
    }
}

void BitStream::readStringWithPosAndLen(string& dstString, const unsigned int pos, const unsigned int len)
{
    dstString.clear();
    for (unsigned int i = 0; i < len; i++)
    {
        char currChar = getByte(pos + i);
        dstString += currChar;
    }
}

void BitStream::readZeroTerminatedString(string& dstString)
{
    char currChar;

    dstString.clear();

    while (mByteOffset != getSize() && (currChar = read8Bits()) != '\0')
    {
        dstString += currChar;
        currChar = read8Bits();
    }
}

unsigned int BitStream::readExpGolombCode()
{
    int leadingZeroBits = -1;
    unsigned int codeNum;
    int tmpBit = 0;

    while (tmpBit == 0)
    {
        tmpBit = readBits(1);
        leadingZeroBits++;
    }

    codeNum = (1 << leadingZeroBits) - 1 + readBits(leadingZeroBits);

    return codeNum;
}

int BitStream::readSignedExpGolombCode()
{
    unsigned int codeNum = readExpGolombCode();
    int signedVal = int((codeNum + 1) >> 1);

    if ((codeNum & 1) == 0)
    {
        signedVal = -signedVal;
    }

    return signedVal;
}

BitStream BitStream::readSubBoxBitStream(FourCCInt& boxType)
{
    uint64_t boxSize = read32Bits();
    boxType = read32Bits();

    if (boxSize == 1) // Check if 'largesize' field is used
    {
        boxSize = read64Bits();
        mByteOffset -= 8;
    }

    mByteOffset -= 8;

    BitStream subBitstr;
    extract(getPos(), getPos() + boxSize, subBitstr);
    mByteOffset += boxSize;

    return subBitstr;
}
