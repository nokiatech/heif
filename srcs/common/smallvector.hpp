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

#ifndef SMALLVECTOR_HPP
#define SMALLVECTOR_HPP

#include <algorithm>
#include <iterator>
#include <stdexcept>
#include "customallocator.hpp"

/** SmallVector is a fixed-overhead container for storing a small
 * number of values (most of the time). For the exceptional case
 * of overflowing the buffer an Vector will be used to store
 * the contents instead.
 */

template <typename Value, size_t Size>
class SmallVector
{
private:
    typedef ::Vector<Value> Vector;

public:
    typedef typename Vector::difference_type difference_type;
    typedef typename Vector::size_type size_type;
    typedef Value value_type;

    template <typename BaseContainer, typename Iterator, typename VectorIterator>
    struct iterator_root
    {
    public:
        typedef typename Vector::difference_type difference_type;
        typedef Value value_type;
        typedef std::random_access_iterator_tag iterator_category;
        typedef typename BaseContainer::value_type& reference;
        typedef typename BaseContainer::value_type* pointer;

        bool operator==(const iterator_root<BaseContainer, Iterator, VectorIterator>& other) const
        {
            return small ? smallIndex == other.smallIndex : iterator == other.iterator;
        }
        bool operator!=(const iterator_root<BaseContainer, Iterator, VectorIterator>& other) const
        {
            return !(*this == other);
        }
        bool operator<(const iterator_root<BaseContainer, Iterator, VectorIterator>& other) const
        {
            return small ? smallIndex < other.smallIndex : iterator < other.iterator;
        }
        bool operator<=(const iterator_root<BaseContainer, Iterator, VectorIterator>& other) const
        {
            return small ? smallIndex <= other.smallIndex : iterator <= other.iterator;
        }
        bool operator>(const iterator_root<BaseContainer, Iterator, VectorIterator>& other) const
        {
            return small ? smallIndex > other.smallIndex : iterator > other.iterator;
        }
        bool operator>=(const iterator_root<BaseContainer, Iterator, VectorIterator>& other) const
        {
            return small ? smallIndex >= other.smallIndex : iterator >= other.iterator;
        }

        iterator_root()
        {
        }

        iterator_root(BaseContainer& container_, signed int index_)
            : container(container_)
            , small(true)
            , smallIndex(index_)
        {
            // nothing
        }
        iterator_root(BaseContainer& container_, const VectorIterator& iterator_)
            : container(container_)
            , small(false)
            , smallIndex(0)
            , iterator(iterator_)
        {
            // nothing
        }

    protected:
        BaseContainer& container;
        bool small;
        signed int smallIndex;  // can be negative for rend()
        VectorIterator iterator;

        friend class SmallVector<Value, Size>;
    };

    template <typename BaseContainer, typename Iterator, typename VectorIterator>
    struct iterator_base : public iterator_root<BaseContainer, Iterator, VectorIterator>
    {
    protected:
        typedef iterator_root<BaseContainer, Iterator, VectorIterator> root;

    public:
        iterator_base()
        {
        }
        iterator_base(BaseContainer& container_, signed int index_)
            : root(container_, index_)
        {
        }
        iterator_base(BaseContainer& container_, const VectorIterator& iterator_)
            : root(container_, iterator_)
        {
        }

        Iterator& operator++()
        {
            if (root::small)
                ++root::smallIndex;
            else
                ++root::iterator;
            return *static_cast<Iterator*>(this);
        }
        Iterator& operator--()
        {
            if (root::small)
                --root::smallIndex;
            else
                --root::iterator;
            return *static_cast<Iterator*>(this);
        }
        Iterator operator++(int)
        {
            auto prev = *static_cast<Iterator*>(this);
            if (root::small)
                ++root::smallIndex;
            else
                ++root::iterator;
            return prev;
        }
        Iterator operator--(int)
        {
            auto prev = *static_cast<Iterator*>(this);
            if (root::small)
                --root::smallIndex;
            else
                --root::iterator;
            return prev;
        }

        Iterator& operator+=(int n)
        {
            if (root::small)
                root::smallIndex += n;
            else
                root::iterator += n;
            return *static_cast<Iterator*>(this);
        }
        Iterator& operator-=(int n)
        {
            if (root::small)
                root::smallIndex -= n;
            else
                root::iterator -= n;
            return *static_cast<Iterator*>(this);
        }

        // I would like to use argument const Iterator& here, but for some reason it won't compile
        difference_type operator-(const iterator_base<BaseContainer, Iterator, VectorIterator>& other) const
        {
            if (root::small)
            {
                return root::smallIndex - other.smallIndex;
            }
            else
            {
                return root::iterator - other.iterator;
            }
        }
    };

    template <typename BaseContainer, typename Iterator, typename VectorIterator>
    struct reverse_iterator_base : public iterator_root<BaseContainer, Iterator, VectorIterator>
    {
    protected:
        typedef iterator_root<BaseContainer, Iterator, VectorIterator> root;

    public:
        reverse_iterator_base()
        {
        }
        reverse_iterator_base(BaseContainer& container_, signed int index_)
            : root(container_, index_)
        {
        }
        reverse_iterator_base(BaseContainer& container_, const VectorIterator& iterator_)
            : root(container_, iterator_)
        {
        }

        Iterator& operator++()
        {
            if (root::small)
                --root::smallIndex;
            else
                ++root::iterator;
            return *static_cast<Iterator*>(this);
        }
        Iterator& operator--()
        {
            if (root::small)
                ++root::smallIndex;
            else
                --root::iterator;
            return *static_cast<Iterator*>(this);
        }
        Iterator operator++(int)
        {
            auto prev = *static_cast<Iterator*>(this);
            if (root::small)
                --root::smallIndex;
            else
                ++root::iterator;
            return prev;
        }
        Iterator operator--(int)
        {
            auto prev = *static_cast<Iterator*>(this);
            if (root::small)
                ++root::smallIndex;
            else
                --root::iterator;
            return prev;
        }

        Iterator& operator+=(int n)
        {
            if (root::small)
                root::smallIndex -= n;
            else
                root::iterator -= n;
            return *static_cast<Iterator*>(this);
        }
        Iterator& operator-=(int n)
        {
            if (root::small)
                root::smallIndex += n;
            else
                root::iterator -= n;
            return *static_cast<Iterator*>(this);
        }

        difference_type operator-(const reverse_iterator_base<BaseContainer, Iterator, VectorIterator>& other) const
        {
            if (root::small)
            {
                return other.smallIndex - root::smallIndex;
            }
            else
            {
                return root::iterator - other.iterator;
            }
        }

        bool operator<(const iterator_root<BaseContainer, Iterator, VectorIterator>& other) const
        {
            return root::small ? (root::smallIndex > other.smallIndex) : (root::iterator < other.root::iterator);
        }
        bool operator<=(const iterator_root<BaseContainer, Iterator, VectorIterator>& other) const
        {
            return root::small ? (root::smallIndex >= other.smallIndex) : (root::iterator <= other.root::iterator);
        }
        bool operator>(const iterator_root<BaseContainer, Iterator, VectorIterator>& other) const
        {
            return root::small ? (root::smallIndex < other.smallIndex) : (root::iterator > other.root::iterator);
        }
        bool operator>=(const iterator_root<BaseContainer, Iterator, VectorIterator>& other) const
        {
            return root::small ? (root::smallIndex <= other.smallIndex) : (root::iterator >= other.root::iterator);
        }
    };

    struct iterator : public iterator_base<SmallVector<Value, Size>, iterator, typename Vector::iterator>
    {
        typedef iterator_base<SmallVector<Value, Size>, iterator, typename Vector::iterator> base;
        iterator()
        {
        }
        iterator(SmallVector<Value, Size>& container_, signed int index_)
            : base(container_, index_)
        {
        }
        iterator(SmallVector<Value, Size>& container_, const typename Vector::iterator& iterator_)
            : base(container_, iterator_)
        {
        }
        Value& operator*()
        {
            return base::small ? base::container.mSmallContents[base::smallIndex] : *base::iterator;
        }
        const Value& operator*() const
        {
            return base::small ? base::container.mSmallContents[base::smallIndex] : *base::iterator;
        }
    };

    struct const_iterator
        : public iterator_base<const SmallVector<Value, Size>, const_iterator, typename Vector::const_iterator>
    {
        typedef iterator_base<const SmallVector<Value, Size>, const_iterator, typename Vector::const_iterator> base;
        const_iterator()
        {
        }
        const_iterator(const SmallVector& container_, signed int index_)
            : base(container_, index_)
        {
        }
        const_iterator(const SmallVector& container_, const typename Vector::const_iterator& iterator_)
            : base(container_, iterator_)
        {
        }
        const Value& operator*() const
        {
            return base::small ? base::container.mSmallContents[base::smallIndex] : *base::iterator;
        }
    };

    struct reverse_iterator
        : public reverse_iterator_base<SmallVector<Value, Size>, reverse_iterator, typename Vector::reverse_iterator>
    {
        typedef reverse_iterator_base<SmallVector<Value, Size>, reverse_iterator, typename Vector::reverse_iterator>
            base;
        reverse_iterator()
        {
        }
        reverse_iterator(SmallVector<Value, Size>& container_, signed int index_)
            : base(container_, index_)
        {
        }
        reverse_iterator(SmallVector<Value, Size>& container_, const typename Vector::reverse_iterator& iterator_)
            : base(container_, iterator_)
        {
        }
        Value& operator*()
        {
            return base::small ? base::container.mSmallContents[base::smallIndex] : *base::iterator;
        }
        const Value& operator*() const
        {
            return base::small ? base::container.mSmallContents[base::smallIndex] : *base::iterator;
        }
    };

    struct const_reverse_iterator : public reverse_iterator_base<const SmallVector<Value, Size>,
                                                                 const_reverse_iterator,
                                                                 typename Vector::const_reverse_iterator>
    {
        typedef reverse_iterator_base<const SmallVector<Value, Size>,
                                      const_reverse_iterator,
                                      typename Vector::const_reverse_iterator>
            base;
        const_reverse_iterator()
        {
        }
        const_reverse_iterator(const SmallVector& container_, signed int index_)
            : base(container_, index_)
        {
        }
        const_reverse_iterator(const SmallVector& container_, const typename Vector::const_reverse_iterator& iterator_)
            : base(container_, iterator_)
        {
        }
        const Value& operator*() const
        {
            return base::small ? base::container.mSmallContents[base::smallIndex] : *base::iterator;
        }
    };

    SmallVector();
    SmallVector(const SmallVector<Value, Size>& other);
    ~SmallVector();

    SmallVector<Value, Size>& operator=(const SmallVector<Value, Size>& other);

    iterator begin();
    iterator end();

    const_iterator begin() const;
    const_iterator end() const;

    const_iterator cbegin() const;
    const_iterator cend() const;

    reverse_iterator rbegin();
    reverse_iterator rend();

    const_reverse_iterator rbegin() const;
    const_reverse_iterator rend() const;

    void push_back(const Value& value);

    Value& at(size_t index);
    Value at(size_t index) const;

    Value& operator[](size_t index);
    Value operator[](size_t index) const;

    size_type size() const;

    bool isSmall() const;

    /* For super-fast access! */
    Value* data()
    {
        return mSmall ? mSmallContents : mVector->data();
    }
    const Value* data() const
    {
        return mSmall ? mSmallContents : mVector->data();
    }

private:
    /// moves elements from mSmallContents to mVector and sets mSmall = false
    void convertLarge();

    bool mSmall;  // true if mNumElements and mSmallContents are used, otherwise mVector is used
    size_type mNumElements;
    Value mSmallContents[Size];
    UniquePtr<Vector> mVector;
};

template <typename Value, size_t Size>
SmallVector<Value, Size>::SmallVector()
    : mSmall(true)
    , mNumElements(0)
{
    // nothing
}

template <typename Value, size_t Size>
SmallVector<Value, Size>::~SmallVector()
{
    // nothing
}

template <typename Value, size_t Size>
SmallVector<Value, Size>::SmallVector(const SmallVector<Value, Size>& other)
    : mSmall(other.mSmall)
    , mNumElements(other.mNumElements)
{
    if (mSmall)
    {
        std::copy(other.mSmallContents, other.mSmallContents + mNumElements, mSmallContents);
    }
    else
    {
        mVector.reset(CUSTOM_NEW(Vector, (*other.mVector)));
    }
}

template <typename Value, size_t Size>
SmallVector<Value, Size>& SmallVector<Value, Size>::operator=(const SmallVector<Value, Size>& other)
{
    if (this != &other)
    {
        auto prevNumElements = mNumElements;
        mVector.reset();
        mSmall       = other.mSmall;
        mNumElements = other.mNumElements;
        for (Value* it = mSmallContents + mNumElements; it < mSmallContents + prevNumElements; ++it)
        {
            *it = Value();
        }
        if (mSmall)
        {
            std::copy(other.mSmallContents, other.mSmallContents + mNumElements, mSmallContents);
        }
        else
        {
            mVector.reset(CUSTOM_NEW(Vector, (*other.mVector)));
        }
    }
    return *this;
}

template <typename Value, size_t Size>
typename SmallVector<Value, Size>::iterator SmallVector<Value, Size>::begin()
{
    return mSmall ? iterator(*this, 0) : iterator(*this, mVector->begin());
}

template <typename Value, size_t Size>
typename SmallVector<Value, Size>::iterator SmallVector<Value, Size>::end()
{
    return mSmall ? iterator(*this, static_cast<signed int>(mNumElements)) : iterator(*this, mVector->end());
}

template <typename Value, size_t Size>
typename SmallVector<Value, Size>::const_iterator SmallVector<Value, Size>::begin() const
{
    return mSmall ? const_iterator(*this, 0) : const_iterator(*this, mVector->cbegin());
}

template <typename Value, size_t Size>
typename SmallVector<Value, Size>::const_iterator SmallVector<Value, Size>::end() const
{
    return mSmall ? const_iterator(*this, static_cast<signed int>(mNumElements))
                  : const_iterator(*this, mVector->cend());
}

template <typename Value, size_t Size>
typename SmallVector<Value, Size>::const_iterator SmallVector<Value, Size>::cbegin() const
{
    return mSmall ? const_iterator(*this, 0) : const_iterator(*this, mVector->begin());
}

template <typename Value, size_t Size>
typename SmallVector<Value, Size>::const_iterator SmallVector<Value, Size>::cend() const
{
    return mSmall ? const_iterator(*this, static_cast<signed int>(mNumElements))
                  : const_iterator(*this, mVector->end());
}

template <typename Value, size_t Size>
typename SmallVector<Value, Size>::reverse_iterator SmallVector<Value, Size>::rbegin()
{
    return mSmall ? reverse_iterator(*this, static_cast<signed int>(mNumElements) - 1)
                  : reverse_iterator(*this, mVector->rbegin());
}

template <typename Value, size_t Size>
typename SmallVector<Value, Size>::reverse_iterator SmallVector<Value, Size>::rend()
{
    return mSmall ? reverse_iterator(*this, -1) : reverse_iterator(*this, mVector->rend());
}

template <typename Value, size_t Size>
typename SmallVector<Value, Size>::const_reverse_iterator SmallVector<Value, Size>::rbegin() const
{
    return mSmall ? const_reverse_iterator(*this, static_cast<signed int>(mNumElements) - 1)
                  : const_reverse_iterator(*this, mVector->rbegin());
}

template <typename Value, size_t Size>
typename SmallVector<Value, Size>::const_reverse_iterator SmallVector<Value, Size>::rend() const
{
    return mSmall ? const_reverse_iterator(*this, -1) : const_reverse_iterator(*this, mVector->rend());
}

template <typename Value, size_t Size>
void SmallVector<Value, Size>::convertLarge()
{
    if (mSmall)
    {
        mSmall = false;
        mVector.reset(CUSTOM_NEW(Vector, (mSmallContents, mSmallContents + mNumElements)));
        for (size_type i = 0; i < mNumElements; ++i)
        {
            mSmallContents[i] = Value();
        }
        mNumElements = 0;
    }
}

template <typename Value, size_t Size>
void SmallVector<Value, Size>::push_back(const Value& value)
{
    if (mSmall)
    {
        if (mNumElements == Size)
        {
            convertLarge();
            mVector->push_back(value);
        }
        else
        {
            mSmallContents[mNumElements] = value;
            ++mNumElements;
        }
    }
    else
    {
        mVector->push_back(value);
    }
}

template <typename Value, size_t Size>
Value& SmallVector<Value, Size>::at(size_t index)
{
    if (mSmall)
    {
        if (index >= mNumElements)
        {
            throw LogicError("SmallVector::at: index out of bounds");
        }
        else
        {
            return mSmallContents[index];
        }
    }
    else
    {
        return mVector->at(index);
    }
}

template <typename Value, size_t Size>
Value SmallVector<Value, Size>::at(size_t index) const
{
    if (mSmall)
    {
        if (index >= mNumElements)
        {
            throw LogicError("SmallVector::at: index out of bounds");
        }
        else
        {
            return mSmallContents[index];
        }
    }
    else
    {
        return mVector->at(index);
    }
}

template <typename Value, size_t Size>
Value& SmallVector<Value, Size>::operator[](size_t index)
{
    return mSmall ? mSmallContents[index] : (*mVector)[index];
}

template <typename Value, size_t Size>
Value SmallVector<Value, Size>::operator[](size_t index) const
{
    return mSmall ? mSmallContents[index] : (*mVector)[index];
}

template <typename Value, size_t Size>
typename SmallVector<Value, Size>::size_type SmallVector<Value, Size>::size() const
{
    return mSmall ? mNumElements : mVector->size();
}

#endif  // SMALLVECTOR_HPP
