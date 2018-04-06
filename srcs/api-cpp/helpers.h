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

#pragma once

#include <stdint.h>
#include <string.h>
#include <map>
#include <vector>
#if (defined(_DEBUG) || defined(DEBUG)) || (!defined(NDEBUG))
#define HEIF_DEBUG
#include <assert.h>
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
        HEIF_ASSERT(false);
        return false;
    }
    template <class type>
    bool AddItemTo(std::vector<type>& list, type item)
    {
        for (auto it = list.begin(); it != list.end(); ++it)
        {
            if (item == *it)
            {
                HEIF_ASSERT(false);
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
        uint32_t size() const;
        void addLink(T aTarget);
        bool removeLink(T aTarget);
        std::pair<T, uint32_t>& operator[](uint32_t);

    protected:
        std::vector<std::pair<T, uint32_t>> mList;

    private:
        LinkArray<T>& operator=(const LinkArray<T>&) = delete;
        LinkArray<T>(const LinkArray<T>&)            = delete;
        LinkArray<T>(LinkArray<T>&&)                 = delete;
    };

}  // namespace HEIFPP
