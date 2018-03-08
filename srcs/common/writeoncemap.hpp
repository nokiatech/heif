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

#ifndef WRITEONCEMAP_HPP
#define WRITEONCEMAP_HPP

#include <algorithm>
#include <stdexcept>
#include "customallocator.hpp"

template <typename Key, typename Value>
class WriteOnceMap
{
public:
    typedef std::pair<Key, Value> Entry;

    // WriteOnceMap doesn't provide mutating iterators
    typedef typename Vector<Entry>::iterator iterator;
    typedef typename Vector<Entry>::const_iterator const_iterator;
    typedef typename Vector<Entry>::const_reverse_iterator reverse_iterator;
    typedef typename Vector<Entry>::const_reverse_iterator const_reverse_iterator;

    /** Creates an empty map. Should be filled in once and its keys
     * should never be changed after filling in the data for the best
     * performance. */
    WriteOnceMap();

    /** Creates a map from a given vector. The vector contents is
     * moved in and sorted when needed. You may also provide
     * an empty vector but with .reserve called beforehand in
     * anticipation of certain usage. */
    explicit WriteOnceMap(Vector<Entry>&& init);

    /** Copies a map. The map 'other' is validated first to reduce
     * the overhead of copying the map multiple times.  */
    WriteOnceMap(const WriteOnceMap<Key, Value>& other);

    /** Moves a map. No map is explicitly validated at this point. */
    WriteOnceMap(WriteOnceMap<Key, Value>&& other);

    ~WriteOnceMap();

    /** Copies a map. The _source_ map is validated first to reduce
     * the overhead of copying the map multiple times. */
    WriteOnceMap<Key, Value>& operator=(const WriteOnceMap<Key, Value>& other);
    WriteOnceMap<Key, Value>& operator=(WriteOnceMap<Key, Value>&& other);

    /** Implements the equal_range operatation familiar from Map */
    std::pair<iterator, iterator> equal_range(const Key& key);
    std::pair<const_iterator, const_iterator> equal_range(const Key& key) const;

    iterator find(const Key& key);
    const_iterator find(const Key& key) const;
    Value& at(const Key& key);
    const Value& at(const Key& key) const;
    size_t count(const Key& key) const;

    /** Adds a new element. Causes the container to be re-sorted on
     * the next read access.
     *
     * The container also re-sorts itself periodically to eliminate
     * long sequences of inserted duplicates. */
    void insert(const Entry& entry);
    void insert(Entry&& entry);
    void insert(const Key& key, const Value& value);

    iterator begin() const;
    iterator end() const;

    const_iterator cbegin() const;
    const_iterator cend() const;

    const_reverse_iterator rbegin() const;
    const_reverse_iterator rend() const;

    std::size_t size() const;

    Value& operator[](const Key& key);
    const Value& operator[](const Key& key) const;

    /* Passed off to the underlying vector. Calling this with an
     * argument larger than the number of elements in the vector
     * ensures the underlying vector will allocate the memory up-front
     * instead of upon insertion.
     *
     * Note that the number of elements in the vector may be larger
     * than the number of elements in the vector if you insert
     * duplicate keys (a later phase removes duplicates). */
    void reserve(size_t n);

private:
    mutable Vector<Entry> mData;
    mutable bool mRequireValidation;      /// set upon mutation; reset upon enforceValidate()
    mutable int mNumInsertionsCountdown;  /// once this reaches 0, the data is revalidated

    /** After calling this the container is sorted by its keys and its
     * keys are unique. */
    void checkValidation() const;

    /** Actually does the job. Split into these two parts to make
     * looking profiling output easier. */
    void makeValidated() const;

    static bool lessThanProjectFirst(const Entry& a, const Entry& b)
    {
        return a.first < b.first;
    };

    static bool equalThanProjectFirst(const Entry& a, const Entry& b)
    {
        return a.first == b.first;
    };
};

template <typename Key, typename Value>
WriteOnceMap<Key, Value>::WriteOnceMap()
    : mRequireValidation(false)
    , mNumInsertionsCountdown(10000)
{
    // nothing
}

template <typename Key, typename Value>
WriteOnceMap<Key, Value>::WriteOnceMap(Vector<Entry>&& init)
    : mRequireValidation(true)
    , mNumInsertionsCountdown(init.size() + 10000)
{
    mData = std::move(init);
}

template <typename Key, typename Value>
WriteOnceMap<Key, Value>::WriteOnceMap(const WriteOnceMap<Key, Value>& other)
{
    mData                   = other.mData;
    mRequireValidation      = other.mRequireValidation;
    mNumInsertionsCountdown = other.mNumInsertionsCountdown;
}

template <typename Key, typename Value>
WriteOnceMap<Key, Value>::WriteOnceMap(WriteOnceMap<Key, Value>&& other)
    : mData(std::move(other.mData))
    , mRequireValidation(other.mRequireValidation)
    , mNumInsertionsCountdown(other.mNumInsertionsCountdown)
{
    // nothing
}

template <typename Key, typename Value>
void WriteOnceMap<Key, Value>::makeValidated() const
{
    if (mData.size())
    {
        std::stable_sort(mData.begin(), mData.end(), lessThanProjectFirst);

        // we use std::unique because it's OK to forget duplicates in
        // the second list
        auto last = std::unique(mData.begin(), mData.end(), equalThanProjectFirst);
        mData.erase(last, mData.end());
    }
    mRequireValidation      = false;
    mNumInsertionsCountdown = static_cast<int>(mData.size() + 10000);
}

template <typename Key, typename Value>
void WriteOnceMap<Key, Value>::checkValidation() const
{
    if (mRequireValidation)
    {
        makeValidated();
    }
}

template <typename Key, typename Value>
WriteOnceMap<Key, Value>::~WriteOnceMap()
{
    // nothing
}

template <typename Key, typename Value>
WriteOnceMap<Key, Value>& WriteOnceMap<Key, Value>::operator=(const WriteOnceMap<Key, Value>& other)
{
    if (&other != this)
    {
        mData                   = other.mData;
        mRequireValidation      = other.mRequireValidation;
        mNumInsertionsCountdown = other.mNumInsertionsCountdown;
    }
    return *this;
}

template <typename Key, typename Value>
WriteOnceMap<Key, Value>& WriteOnceMap<Key, Value>::operator=(WriteOnceMap<Key, Value>&& other)
{
    if (&other != this)
    {
        mData                   = std::move(other.mData);
        mRequireValidation      = other.mRequireValidation;
        mNumInsertionsCountdown = other.mNumInsertionsCountdown;
    }
    return *this;
}

template <typename Key, typename Value>
void WriteOnceMap<Key, Value>::insert(const Entry& entry)
{
    // Allow inserting to the end without triggering validation, if it is already
    // in the proper order
    if (mData.size() == 0 || (mData.size() > 0 && !mRequireValidation && entry.first > mData.rbegin()->first))
    {
        mData.push_back(entry);
    }
    else
    {
        mData.push_back(entry);
        mRequireValidation = true;
        --mNumInsertionsCountdown;
        if (mNumInsertionsCountdown < 0)
        {
            checkValidation();
        }
    }
}

template <typename Key, typename Value>
void WriteOnceMap<Key, Value>::insert(Entry&& entry)
{
    // Allow inserting to the end without triggering validation, if it is already
    // in the proper order
    if (mData.size() == 0 || (mData.size() > 0 && !mRequireValidation && entry.first > mData.rbegin()->first))
    {
        mData.push_back(std::move(entry));
    }
    else
    {
        mData.push_back(std::move(entry));
        mRequireValidation = true;
        --mNumInsertionsCountdown;
        if (mNumInsertionsCountdown < 0)
        {
            checkValidation();
        }
    }
}

template <typename Key, typename Value>
void WriteOnceMap<Key, Value>::insert(const Key& key, const Value& value)
{
    insert(std::make_pair(key, value));
}

template <typename Key, typename Value>
typename WriteOnceMap<Key, Value>::iterator WriteOnceMap<Key, Value>::begin() const
{
    checkValidation();
    return mData.begin();
}

template <typename Key, typename Value>
typename WriteOnceMap<Key, Value>::iterator WriteOnceMap<Key, Value>::end() const
{
    checkValidation();
    return mData.end();
}

template <typename Key, typename Value>
typename WriteOnceMap<Key, Value>::const_iterator WriteOnceMap<Key, Value>::cbegin() const
{
    checkValidation();
    return mData.cbegin();
}

template <typename Key, typename Value>
typename WriteOnceMap<Key, Value>::const_iterator WriteOnceMap<Key, Value>::cend() const
{
    checkValidation();
    return mData.cend();
}

template <typename Key, typename Value>
typename WriteOnceMap<Key, Value>::const_reverse_iterator WriteOnceMap<Key, Value>::rbegin() const
{
    checkValidation();
    return mData.rbegin();
}

template <typename Key, typename Value>
typename WriteOnceMap<Key, Value>::const_reverse_iterator WriteOnceMap<Key, Value>::rend() const
{
    checkValidation();
    return mData.rend();
}

template <typename Key, typename Value>
std::size_t WriteOnceMap<Key, Value>::size() const
{
    checkValidation();
    return mData.size();
}

template <typename Key, typename Value>
std::pair<typename WriteOnceMap<Key, Value>::iterator, typename WriteOnceMap<Key, Value>::iterator>
WriteOnceMap<Key, Value>::equal_range(const Key& key)
{
    checkValidation();
    iterator lower = std::lower_bound(begin(), end(), std::make_pair(key, Value()),
                                      lessThanProjectFirst);
    if (lower != end() && lower->first == key)
    {
        iterator upper = std::upper_bound(lower, end(), std::make_pair(key, Value()),
                                          lessThanProjectFirst);
        return std::make_pair(lower, upper);
    }
    else
    {
        return std::make_pair(end(), end());
    }
}

template <typename Key, typename Value>
std::pair<typename WriteOnceMap<Key, Value>::const_iterator, typename WriteOnceMap<Key, Value>::const_iterator>
WriteOnceMap<Key, Value>::equal_range(const Key& key) const
{
    checkValidation();
    const_iterator lower = std::lower_bound(cbegin(), cend(), std::make_pair(key, Value()),
                                            lessThanProjectFirst);
    if (lower != end() && lower->first == key)
    {
        const_iterator upper = std::upper_bound(lower, cend(), std::make_pair(key, Value()),
                                                lessThanProjectFirst);
        return std::make_pair(lower, upper);
    }
    else
    {
        return std::make_pair(end(), end());
    }
}

template <typename Key, typename Value>
typename WriteOnceMap<Key, Value>::const_iterator
WriteOnceMap<Key, Value>::find(const Key& key) const
{
    checkValidation();
    const_iterator lower = std::lower_bound(begin(), end(), std::make_pair(key, Value()),
                                            lessThanProjectFirst);
    if (lower != end() && lower->first == key)
    {
        return lower;
    }
    else
    {
        return end();
    }
}

template <typename Key, typename Value>
Value&
WriteOnceMap<Key, Value>::at(const Key& key)
{
    checkValidation();
    auto lower = std::lower_bound(mData.begin(), mData.end(), std::make_pair(key, Value()),
                                  lessThanProjectFirst);
    if (lower != mData.end() && lower->first == key)
    {
        return lower->second;
    }
    else
    {
        throw LogicError("WriteOnceMap::at: tried to access non-existing element");
    }
}

template <typename Key, typename Value>
const Value&
WriteOnceMap<Key, Value>::at(const Key& key) const
{
    checkValidation();
    auto lower = std::lower_bound(mData.begin(), mData.end(), std::make_pair(key, Value()),
                                  lessThanProjectFirst);
    if (lower != mData.end() && lower->first == key)
    {
        return lower->second;
    }
    else
    {
        throw LogicError("WriteOnceMap::at: tried to access non-existing element");
    }
}

template <typename Key, typename Value>
size_t
WriteOnceMap<Key, Value>::count(const Key& key) const
{
    checkValidation();
    auto lower = std::lower_bound(mData.begin(), mData.end(), std::make_pair(key, Value()),
                                  lessThanProjectFirst);
    return (lower != mData.end() && lower->first == key) ? 1 : 0;
}

template <typename Key, typename Value>
typename Vector<std::pair<Key, Value>>::iterator
WriteOnceMap<Key, Value>::find(const Key& key)
{
    checkValidation();
    typename Vector<std::pair<Key, Value>>::iterator lower =
        std::lower_bound(mData.begin(), mData.end(), std::make_pair(key, Value()),
                         lessThanProjectFirst);
    if (lower != end() && lower->first == key)
    {
        return lower;
    }
    else
    {
        return mData.end();
    }
}

template <typename Key, typename Value>
Value&
    WriteOnceMap<Key, Value>::operator[](const Key& key)
{
    auto it = find(key);
    if (it == mData.end())
    {
        insert(std::make_pair(key, Value()));
        auto last = mData.end();
        --last;
        return last->second;
    }
    else
    {
        return it->second;
    }
}

template <typename Key, typename Value>
const Value&
    WriteOnceMap<Key, Value>::operator[](const Key& key) const
{
    auto it = find(key);
    if (it == end())
    {
        throw LogicError("WriteOnceMap::operator[]: tried to access non-existing element");
    }
    else
    {
        return it->second;
    }
}

template <typename Key, typename Value>
void WriteOnceMap<Key, Value>::reserve(size_t n)
{
    mData.reserve(n);
}

#endif  // WRITEONCEMAP_HPP
