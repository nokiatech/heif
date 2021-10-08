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

#pragma once

#include <cstdint>
#include <map>
#include <vector>
#if (defined(_DEBUG) || defined(DEBUG)) || (!defined(NDEBUG))
#define HEIF_DEBUG
#include <cassert>
#define HEIF_ASSERT(x) assert(x)
#else
#define HEIF_ASSERT(x)
#endif

namespace HEIFPP
{
    template <class type>
    typename std::vector<type>::iterator FindItemIn(std::vector<type>& list, type item)
    {
        for (auto it = list.begin(); it != list.end(); ++it)
        {
            if (item == *it)
            {
                return it;
            }
        }
        return list.end();
    }
    template <class type>
    bool RemoveItemFrom(std::vector<type>& list, type item)
    {
        for (auto it = list.begin(); it != list.end(); ++it)
        {
            if (item == *it)
            {
                list.erase(it);
                return true;
            }
        }
        return false;
    }
    template <class type>
    bool AddItemTo(std::vector<type>& list, type item)
    {
        for (auto it = list.begin(); it != list.end(); ++it)
        {
            if (item == *it)
            {
                return false;
            }
        }
        list.push_back(item);
        return true;
    }
#define IsItemIn(a, b) (FindItemIn(a, b) != a.end())

    template <class T>
    class LinkArray
    {
    public:
        LinkArray()  = default;
        ~LinkArray() = default;
        bool empty() const;
        std::uint32_t size() const;
        void addLink(T aTarget);
        bool removeLink(T aTarget);
        std::pair<T, std::uint32_t>& operator[](uint32_t);
        const std::pair<T, std::uint32_t>& operator[](uint32_t) const;

    protected:
        std::vector<std::pair<T, std::uint32_t>> mList;

    private:
        LinkArray<T>& operator=(const LinkArray<T>&) = delete;
        LinkArray<T>& operator=(LinkArray<T>&&) = delete;
        LinkArray<T>(const LinkArray<T>&)       = delete;
        LinkArray<T>(LinkArray<T>&&)            = delete;
    };


    std::uint8_t expGolombCodeLength(const uint32_t val);

    class BitStream
    {
    public:
        BitStream(std::uint8_t* aData, std::uint32_t aLen);
        std::uint32_t peekBits(std::uint32_t bits);
        std::uint32_t getBits(std::uint32_t bits);
        void writeBits(std::uint64_t bits, std::uint32_t len);
        bool isByteAligned();
        std::uint64_t bitpos();
        void setbitpos(std::uint64_t pos);
        std::uint64_t bits_to_decode();
        std::int32_t getSignedExpGolombCode();  // se(v)
        std::uint32_t getExpGolombCode();       // ue(v)

    protected:
        std::uint8_t* mSrc;
        std::uint32_t mLen;
        std::uint64_t mCBit;
        std::uint32_t mWriteByte;
    };


}  // namespace HEIFPP
