/*
 * This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2021 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its
 * subsidiaries. All rights are reserved. Copying, including reproducing, storing, adapting or translating, any or all
 * of this material requires the prior written consent of Nokia.
 */

#include "helpers.h"

#include <limits>

#include "Heif.h"

namespace HEIFPP
{
    template <class T>
    void LinkArray<T>::addLink(T aTarget)
    {
        for (auto& t : mList)
        {
            if (t.first == aTarget)
            {
                ++t.second;
                return;
            }
        }
        mList.push_back({aTarget, 1});
    }
    template <class T>
    bool LinkArray<T>::removeLink(T aTarget)
    {
        if (aTarget)
        {
            for (auto it = mList.begin(); it != mList.end(); ++it)
            {
                if (aTarget == it->first)
                {
                    it->second--;
                    if (it->second == 0)
                    {
                        mList.erase(it);
                    }
                    return true;
                }
            }
        }
        // Tried to remove nonexistant link.
        HEIF_ASSERT(false);
        return false;
    }

    template <class T>
    bool LinkArray<T>::empty() const
    {
        return mList.empty();
    }
    template <class T>
    std::uint32_t LinkArray<T>::size() const
    {
        return (uint32_t) mList.size();
    }
    template <class T>
    std::pair<T, std::uint32_t>& LinkArray<T>::operator[](uint32_t aId)
    {
        return mList[aId];
    }
    template <class T>
    const std::pair<T, std::uint32_t>& LinkArray<T>::operator[](uint32_t aId) const
    {
        return mList[aId];
    }
    // instantiate templates here.
    template class LinkArray<Item*>;
    template class LinkArray<Track*>;
    template class LinkArray<Sample*>;
    template class LinkArray<MetaItem*>;
    template class LinkArray<ImageItem*>;
    template class LinkArray<CodedImageItem*>;
    template class LinkArray<DerivedImageItem*>;
    template class LinkArray<DecoderConfig*>;


    std::uint8_t expGolombCodeLength(const std::uint32_t val)
    {
        std::uint32_t i = (val + 1) >> 1;
        std::uint32_t shift;
        for (shift = 0; shift < 33 && i != 0; shift++)
        {
            i >>= 1;
        }
        return (shift << 1) + 1;
    }

    BitStream::BitStream(std::uint8_t* aData, std::uint32_t aLen)
        : mSrc(aData)
        , mLen(aLen)
        , mCBit(0)
        , mWriteByte(0)
    {
    }

    std::uint32_t BitStream::peekBits(std::uint32_t bits)
    {
        std::uint32_t res      = 0;
        std::uint64_t tempCBit = mCBit;
        for (std::uint32_t i = 0; i < bits; i++)
        {
            auto index         = static_cast<std::uint32_t>(tempCBit / 8);
            std::uint32_t cbit = static_cast<std::uint8_t>(tempCBit - (index * 8));
            res |= ((static_cast<std::uint32_t>(mSrc[index]) >> (7u - cbit)) & 1u) << (bits - i - 1u);
            tempCBit++;
        }
        return res;
    }

    std::uint32_t BitStream::getBits(std::uint32_t bits)
    {
        std::uint32_t res = 0;
        for (std::uint32_t i = 0; i < bits; i++)
        {
            auto index         = static_cast<std::uint32_t>(mCBit / 8);
            std::uint32_t cbit = static_cast<std::uint8_t>(mCBit - (index * 8));
            res |= ((static_cast<std::uint32_t>(mSrc[index]) >> (7u - cbit)) & 1u) << (bits - i - 1u);
            mCBit++;
        }
        return res;
    }

    void BitStream::writeBits(std::uint64_t bits, std::uint32_t len)
    {
        if (len == 0)
        {
            return;
        }
        else
        {
            do
            {
                auto index         = static_cast<std::uint32_t>(mCBit / 8);
                std::uint32_t cbit = static_cast<std::uint8_t>(mCBit - (index * 8));

                const unsigned int numBitsLeftInByte = 8 - cbit;
                if (numBitsLeftInByte > len)
                {
                    mWriteByte = mWriteByte | static_cast<unsigned int>(
                                                  (bits & (std::numeric_limits<std::uint64_t>::max() >> (64 - len)))
                                                  << (numBitsLeftInByte - len));
                    mSrc[index] = static_cast<uint8_t>(mWriteByte);
                    mCBit += len;
                    len = 0;
                }
                else
                {
                    mWriteByte  = mWriteByte | (static_cast<unsigned int>((bits >> (len - numBitsLeftInByte)) &
                                                                         ~((std::numeric_limits<std::uint64_t>::max()
                                                                            << (64 - numBitsLeftInByte)))));
                    mSrc[index] = static_cast<uint8_t>(mWriteByte);
                    mWriteByte  = 0;
                    mCBit += numBitsLeftInByte;
                    len -= numBitsLeftInByte;
                }
            } while (len > 0);
        }
    }

    bool BitStream::isByteAligned()
    {
        return (mCBit & 7u) == 0;
    }

    std::uint64_t BitStream::bitpos()
    {
        return mCBit;
    }

    void BitStream::setbitpos(std::uint64_t pos)
    {
        mCBit = pos;
    }

    std::uint64_t BitStream::bits_to_decode()
    {
        return (mLen * 8) - mCBit;
    }

    std::int32_t BitStream::getSignedExpGolombCode()  // se(v)
    {
        std::uint32_t codeNum = getExpGolombCode();
        auto signedVal        = std::int32_t((codeNum + 1) >> 1);

        if ((codeNum & 1) == 0)
        {
            signedVal = -signedVal;
        }

        return signedVal;
    }

    std::uint32_t BitStream::getExpGolombCode()  // ue(v)
    {
        std::int32_t leadingZeroBits = -1;
        std::uint32_t codeNum;
        std::uint32_t tmpBit = 0;

        while (tmpBit == 0)
        {
            tmpBit = getBits(1);
            leadingZeroBits++;
        }

        auto shiftAmount = static_cast<std::uint32_t>(leadingZeroBits);
        codeNum          = (static_cast<std::uint32_t>(1) << shiftAmount) - 1 + getBits(shiftAmount);
        return codeNum;
    }

}  // namespace HEIFPP
