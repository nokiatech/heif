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

#include "bitstream.hpp"
#include <cstring>
#include <limits>
#include <stdexcept>
#include "log.hpp"

namespace ISOBMFF
{
    BitStream::BitStream()
        : mStorage()
        , mCurrByte(0)
        , mByteOffset(0)
        , mBitOffset(0)
        , mStorageAllocated(true)
    {
    }

    BitStream::BitStream(const Vector<std::uint8_t>& strData)
        : mStorage(strData)
        , mCurrByte(0)
        , mByteOffset(0)
        , mBitOffset(0)
        , mStorageAllocated(false)
    {
    }

    BitStream::BitStream(BitStream&& other)
        : mStorage(std::move(other.mStorage))
        , mCurrByte(other.mCurrByte)
        , mByteOffset(other.mByteOffset)
        , mBitOffset(other.mBitOffset)
        , mStorageAllocated(other.mStorageAllocated)
    {
        other.mCurrByte         = {};
        other.mByteOffset       = {};
        other.mBitOffset        = {};
        other.mStorageAllocated = {};
        other.mStorage.clear();
    }

    BitStream& BitStream::operator=(BitStream&& other)
    {
        mCurrByte         = other.mCurrByte;
        mByteOffset       = other.mByteOffset;
        mBitOffset        = other.mBitOffset;
        mStorageAllocated = other.mStorageAllocated;
        mStorage          = std::move(other.mStorage);
        return *this;
    }

    BitStream::~BitStream()
    {
        if (mStorageAllocated == true)
        {
            mStorage.clear();
        }
    }

    std::uint64_t BitStream::getPos() const
    {
        return mByteOffset;
    }

    bool BitStream::isByteAligned() const
    {
        return mBitOffset ? false : true;
    }

    void BitStream::setPosition(const std::uint64_t position)
    {
        mByteOffset = position;
    }

    std::uint64_t BitStream::getSize() const
    {
        return mStorage.size();
    }

    void BitStream::setSize(const std::uint64_t newSize)
    {
        mStorage.resize(newSize);
    }

    const Vector<std::uint8_t>& BitStream::getStorage() const
    {
        return mStorage;
    }

    Vector<std::uint8_t>& BitStream::getStorage()
    {
        return mStorage;
    }

    void BitStream::reset()
    {
        mCurrByte   = 0;
        mBitOffset  = 0;
        mByteOffset = 0;
    }

    void BitStream::clear()
    {
        mStorage.clear();
    }

    void BitStream::skipBytes(const std::uint64_t count)
    {
        mByteOffset += count;
    }

    void BitStream::setByte(const std::uint64_t offset, const std::uint8_t byte)
    {
        mStorage.at(offset) = byte;
    }

    std::uint8_t BitStream::getByte(const std::uint64_t offset) const
    {
        return mStorage.at(offset);
    }

    std::uint64_t BitStream::numBytesLeft() const
    {
        return mStorage.size() - mByteOffset;
    }
    void BitStream::extract(const std::uint64_t begin, const std::uint64_t end, BitStream& dest) const
    {
        dest.clear();
        dest.reset();
        if (begin <= mStorage.size() && end <= mStorage.size() && begin <= end)
        {
            dest.mStorage.insert(dest.mStorage.begin(), mStorage.begin() + static_cast<std::int64_t>(begin),
                                 mStorage.begin() + static_cast<std::int64_t>(end));
        }
        else
        {
            throw RuntimeError("BitStream::readSubBoxBitStream trying to read 0 size box");
        }
    }

    void BitStream::writeBitStream(const BitStream& bitStr)
    {
        mStorage.insert(mStorage.end(), bitStr.mStorage.begin(), bitStr.mStorage.end());
    }


    void BitStream::write8Bits(const std::uint8_t bits)
    {
        mStorage.push_back(bits);
    }

    void BitStream::write16Bits(const std::uint16_t bits)
    {
        mStorage.push_back(static_cast<uint8_t>((bits >> 8) & 0xff));
        mStorage.push_back(static_cast<uint8_t>((bits) &0xff));
    }

    void BitStream::write24Bits(const std::uint32_t bits)
    {
        mStorage.push_back(static_cast<uint8_t>((bits >> 16) & 0xff));
        mStorage.push_back(static_cast<uint8_t>((bits >> 8) & 0xff));
        mStorage.push_back(static_cast<uint8_t>((bits) &0xff));
    }

    void BitStream::write32Bits(const std::uint32_t bits)
    {
        mStorage.push_back(static_cast<uint8_t>((bits >> 24) & 0xff));
        mStorage.push_back(static_cast<uint8_t>((bits >> 16) & 0xff));
        mStorage.push_back(static_cast<uint8_t>((bits >> 8) & 0xff));
        mStorage.push_back(static_cast<uint8_t>((bits) &0xff));
    }

    void BitStream::write64Bits(const std::uint64_t bits)
    {
        mStorage.push_back(static_cast<uint8_t>((bits >> 56) & 0xff));
        mStorage.push_back(static_cast<uint8_t>((bits >> 48) & 0xff));
        mStorage.push_back(static_cast<uint8_t>((bits >> 40) & 0xff));
        mStorage.push_back(static_cast<uint8_t>((bits >> 32) & 0xff));
        mStorage.push_back(static_cast<uint8_t>((bits >> 24) & 0xff));
        mStorage.push_back(static_cast<uint8_t>((bits >> 16) & 0xff));
        mStorage.push_back(static_cast<uint8_t>((bits >> 8) & 0xff));
        mStorage.push_back(static_cast<uint8_t>((bits) &0xff));
    }

    void BitStream::write8BitsArray(const Vector<std::uint8_t>& bits,
                                    const std::uint64_t len,
                                    const std::uint64_t srcOffset)
    {
        mStorage.insert(mStorage.end(), bits.begin() + static_cast<std::int64_t>(srcOffset),
                        bits.begin() + static_cast<std::int64_t>(srcOffset + len));
    }

    void BitStream::writeBits(std::uint64_t bits, std::uint32_t len)
    {
        if (len == 0)
        {
            logWarning() << "BitStream::writeBits called for zero-length bit sequence." << std::endl;
        }
        else
        {
            do
            {
                const unsigned int numBitsLeftInByte = 8 - mBitOffset;
                if (numBitsLeftInByte > len)
                {
                    mCurrByte =
                        mCurrByte |
                        (static_cast<unsigned int>((bits & (std::numeric_limits<std::uint64_t>::max() >> (64 - len)))
                                                   << (numBitsLeftInByte - len)));
                    mBitOffset += len;
                    len = 0;
                }
                else
                {
                    mCurrByte = mCurrByte | (static_cast<unsigned int>((bits >> (len - numBitsLeftInByte)) &
                                                                       ~((std::numeric_limits<std::uint64_t>::max()
                                                                          << (64 - numBitsLeftInByte)))));
                    mStorage.push_back((uint8_t) mCurrByte);
                    mBitOffset = 0;
                    mCurrByte  = 0;
                    len -= numBitsLeftInByte;
                }
            } while (len > 0);
        }
    }

    void BitStream::writeString(const String& srcString)
    {
        if (srcString.length() == 0)
        {
            logWarning() << "BitStream::writeString called for zero-length string." << std::endl;
        }

        for (const auto character : srcString)
        {
            mStorage.push_back(static_cast<unsigned char>(character));
        }
    }

    void BitStream::writeZeroTerminatedString(const String& srcString)
    {
        for (const auto character : srcString)
        {
            mStorage.push_back(static_cast<unsigned char>(character));
        }
        mStorage.push_back(0);
    }

    void BitStream::write32BitFloat(float value)
    {
        FloatToUint temp;
        temp.asFloat = value;
        write32Bits(temp.asUint);
    }

    float BitStream::read32BitFloat()
    {
        FloatToUint temp;
        temp.asUint = read32Bits();
        return temp.asFloat;
    }

    std::uint8_t BitStream::read8Bits()
    {
        const std::uint8_t ret = mStorage.at(mByteOffset);
        ++mByteOffset;
        return ret;
    }

    std::uint16_t BitStream::read16Bits()
    {
        std::uint16_t ret = mStorage.at(mByteOffset);
        mByteOffset++;
        ret = (ret << 8) | mStorage.at(mByteOffset);
        mByteOffset++;
        return ret;
    }

    std::uint32_t BitStream::read24Bits()
    {
        unsigned int ret = mStorage.at(mByteOffset);
        mByteOffset++;
        ret = (ret << 8) | mStorage.at(mByteOffset);
        mByteOffset++;
        ret = (ret << 8) | mStorage.at(mByteOffset);
        mByteOffset++;
        return ret;
    }

    std::uint32_t BitStream::read32Bits()
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

    std::uint64_t BitStream::read64Bits()
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

    void BitStream::read8BitsArray(Vector<std::uint8_t>& bits, const std::uint64_t len)
    {
        if (static_cast<std::size_t>(mByteOffset + len) <= mStorage.size())
        {
            bits.insert(bits.end(), mStorage.begin() + static_cast<std::int64_t>(mByteOffset),
                        mStorage.begin() + static_cast<std::int64_t>(mByteOffset + len));
            mByteOffset += len;
        }
        else
        {
            throw RuntimeError("BitStream::read8BitsArray trying to read outside of mStorage");
        }
    }

    void BitStream::readByteArrayToBuffer(char* buffer, const std::uint64_t len)
    {
        if (static_cast<std::size_t>(mByteOffset + len) <= mStorage.size())
        {
            std::memcpy(buffer, mStorage.data() + mByteOffset, len);
            mByteOffset += len;
        }
        else
        {
            throw RuntimeError("BitStream::read8BitsArray trying to read outside of mStorage");
        }
    }

    std::uint32_t BitStream::readBits(const std::uint32_t len)
    {
        std::uint32_t returnBits        = 0;
        std::uint32_t numBitsLeftInByte = 8 - mBitOffset;

        if (len == 0)
        {
            return 0;
        }

        if (numBitsLeftInByte >= len)
        {
            returnBits = (unsigned int) ((mStorage).at(mByteOffset) >> (numBitsLeftInByte - len)) &
                         (unsigned int) ((1 << len) - 1);
            mBitOffset += (unsigned int) len;
        }
        else
        {
            std::uint32_t numBitsToGo = len - numBitsLeftInByte;
            returnBits                = (mStorage).at(mByteOffset) & (((unsigned int) 1 << numBitsLeftInByte) - 1);
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
                    returnBits = (returnBits << numBitsToGo) |
                                 ((unsigned int) ((mStorage).at(mByteOffset) >> (8 - numBitsToGo)) &
                                  (((unsigned int) 1 << numBitsToGo) - 1));
                    mBitOffset += (unsigned int) (numBitsToGo);
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

    void BitStream::readStringWithLen(String& dstString, const std::uint32_t len)
    {
        dstString.clear();
        for (std::uint32_t i = 0; i < len; i++)
        {
            std::uint8_t currChar = read8Bits();
            dstString += static_cast<char>(currChar);
        }
    }

    void BitStream::readStringWithPosAndLen(String& dstString, const std::uint64_t pos, const std::uint32_t len)
    {
        dstString.clear();
        for (std::uint32_t i = 0; i < len; i++)
        {
            std::uint8_t currChar = getByte(pos + i);
            dstString += static_cast<char>(currChar);
        }
    }

    void BitStream::readZeroTerminatedString(String& dstString)
    {
        std::uint8_t currChar = 0xff;
        dstString.clear();

        while (mByteOffset < mStorage.size())
        {
            currChar = read8Bits();
            if (currChar != 0)
            {
                dstString += static_cast<char>(currChar);
            }
            else
            {
                break;
            }
        }
    }

    uint32_t BitStream::readExpGolombCode()
    {
        std::int32_t leadingZeroBits = -1;
        std::uint32_t codeNum;
        std::uint32_t tmpBit = 0;

        while (tmpBit == 0)
        {
            tmpBit = readBits(1);
            leadingZeroBits++;
        }

        std::uint32_t shiftAmount = static_cast<std::uint32_t>(leadingZeroBits);
        codeNum                   = ((std::uint32_t) 1 << shiftAmount) - 1 + readBits(shiftAmount);
        return codeNum;
    }

    int32_t BitStream::readSignedExpGolombCode()
    {
        unsigned int codeNum = readExpGolombCode();
        int signedVal        = int((codeNum + 1) >> 1);

        if ((codeNum & 1) == 0)
        {
            signedVal = -signedVal;
        }

        return signedVal;
    }

    BitStream BitStream::readSubBoxBitStream(FourCCInt& boxType)
    {
        std::uint64_t boxSize = read32Bits();

        boxType = read32Bits();

        std::uint64_t minBoxSize = 8;

        if (boxSize == 1)  // Check if 'largesize' field is used
        {
            boxSize = read64Bits();
            minBoxSize += 4;
            mByteOffset -= 8;
        }

        mByteOffset -= 8;

        if (boxSize < minBoxSize)
        {
            throw RuntimeError("BitStream::readSubBoxBitStream trying to read too small box");
        }

        BitStream subBitstr;
        extract(getPos(), getPos() + boxSize, subBitstr);
        mByteOffset += boxSize;

        return subBitstr;
    }
}  // namespace ISOBMFF
