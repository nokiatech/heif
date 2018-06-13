/*
 * This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2018 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its
 * subsidiaries. All rights are reserved. Copying, including reproducing, storing, adapting or translating, any or all
 * of this material requires the prior written consent of Nokia.
 */

#include "helpers.h"
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
    template class LinkArray<DecoderConfiguration*>;


    BitStream::BitStream(const std::uint8_t* aData, std::uint32_t aLen)
            : mSrc(aData)
            , mLen(aLen)
            , mCBit(0)
    {
    }
    std::uint32_t BitStream::getBits(std::uint32_t bits)
    {
        std::uint32_t res = 0;
        for (std::uint32_t i = 0; i < bits; i++)
        {
            std::uint32_t index = mCBit / 8;
            std::uint32_t cbit   = (std::uint8_t)(mCBit - (index * 8));
            res |= ((((std::uint32_t)mSrc[index]) >> (7u - cbit)) & 1u) << (bits - i - 1u);
            mCBit++;
        }
        return res;
    }
    bool BitStream::isByteAligned()
    {
        return (mCBit & 7u);
    }
    std::uint64_t BitStream::bitpos()
    {
        return mCBit;
    }
    std::uint64_t BitStream::bits_to_decode()
    {
        return (mLen * 8) - mCBit;
    }
}  // namespace HEIFPP
