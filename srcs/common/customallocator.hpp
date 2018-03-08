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

#ifndef CUSTOMALLOCATOR_HPP_
#define CUSTOMALLOCATOR_HPP_

#include <list>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <vector>
#include "../api/common/heifallocator.h"

HEIF::CustomAllocator* getDefaultAllocator();
bool setCustomAllocator(HEIF::CustomAllocator* customAllocator);
HEIF::CustomAllocator* getCustomAllocator();

void* customAllocate(size_t size);
void customDeallocate(void* ptr);

template <typename T>
T* customAllocateArray(size_t n)
{
    size_t size    = sizeof(T);
    size_t* header = static_cast<size_t*>(customAllocate(sizeof(size_t) + size * n));
    T* ptr         = reinterpret_cast<T*>(header + 1);
    for (size_t c = 0; c < n; ++c)
    {
        try
        {
            new (ptr + c) T();
        }
        catch (...)
        {
            for (; c > 0; --c)
            {
                (ptr + c - 1)->~T();
            }
            throw;
        }
    }
    *header = n;
    return ptr;
}

template <typename T>
void customDestructAndDeallocateArray(T* ptr)
{
    if (ptr)
    {
        size_t* header = reinterpret_cast<size_t*>(ptr) - 1;
        for (size_t i = 0; i < *header; ++i)
        {
            (ptr + i)->~T();
        }
        customDeallocate(header);
    }
}

// used with UniquePtr, SharedPtr
template <typename T>
class CustomDelete : public std::default_delete<T>
{
public:
    void operator()(T* ptr) const
    {
        ptr->~T();
        customDeallocate(ptr);
    }
};

#define CUSTOM_NEW(class, args) (new (customAllocate(sizeof(class))) class args)
#define CUSTOM_NEW_ARRAY(class, n) (customAllocateArray<class>(n))
#define CUSTOM_DELETE(object, class)  \
    {                                 \
        if (object)                   \
        {                             \
            object->~class();         \
            customDeallocate(object); \
        }                             \
    }
#define CUSTOM_DELETE_ARRAY(object, class)               \
    {                                                    \
        customDestructAndDeallocateArray<class>(object); \
    }

template <typename T>
class Allocator : public std::allocator<T>
{
public:
    typedef size_t size_type;
    typedef T* pointer;
    typedef const T* const_pointer;

    template <typename U>
    struct rebind
    {
        typedef Allocator<U> other;
    };

    pointer allocate(size_type n, const void* hint = 0)
    {
        (void) hint;
        return static_cast<pointer>(getCustomAllocator()->allocate(n, sizeof(T)));
    }

    void deallocate(pointer p, size_type n)
    {
        (void) n;
        return getCustomAllocator()->deallocate(p);
    }

    Allocator() throw()
        : std::allocator<T>()
    {
    }
    Allocator(const Allocator& a) throw()
        : std::allocator<T>(a)
    {
    }
    template <class U>
    Allocator(const Allocator<U>& a) throw()
        : std::allocator<T>(a)
    {
    }
    ~Allocator() throw()
    {
    }
};

typedef std::basic_string<char, std::char_traits<char>, Allocator<char>> String;

template <typename T>
using Vector = std::vector<T, Allocator<T>>;

template <typename T>
using List = std::list<T, Allocator<T>>;

template <typename T, typename Compare = std::less<T>>
using Set = std::set<T, Compare, Allocator<T>>;

template <typename K, typename V, typename Compare = std::less<K>>
using Map = std::map<K, V, Compare, Allocator<std::pair<const K, V>>>;

typedef std::basic_istringstream<char, std::char_traits<char>, Allocator<char>> IStringStream;
typedef std::basic_ostringstream<char, std::char_traits<char>, Allocator<char>> OStringStream;

template <typename T, typename ParentWithVirtualDeleteOrSelf = T>
using UniquePtr = std::unique_ptr<T, CustomDelete<ParentWithVirtualDeleteOrSelf>>;

template <class T>
std::shared_ptr<T> makeCustomShared()
{
    return std::allocate_shared<T>(Allocator<T>());
}

template <class T, class... Args>
std::shared_ptr<T> makeCustomShared(Args&&... args)
{
    return std::allocate_shared<T>(Allocator<T>(), std::forward<Args>(args)...);
}

template <typename T, typename ParentWithVirtualDeleteOrSelf, typename... Args>
UniquePtr<T, ParentWithVirtualDeleteOrSelf> makeCustomUnique(Args&&... args)
{
    return UniquePtr<T, ParentWithVirtualDeleteOrSelf>(CUSTOM_NEW(T, (std::forward<Args>(args)...)));
}

class Exception
{
public:
    Exception()
    {
    }
    Exception(const String&)
    {
    }
    virtual const char* what() const
    {
        return "";
    }
    virtual ~Exception()
    {
    }
};

class RuntimeError : public Exception
{
public:
    RuntimeError(const String& str)
        : Exception(str)
    {
    }
};

class LogicError : public Exception
{
public:
    LogicError(const String& str)
        : Exception(str)
    {
    }
};

#endif  // CUSTOMALLOCATOR_HPP_
