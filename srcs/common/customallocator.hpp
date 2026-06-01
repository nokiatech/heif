/* This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2026 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its
 * subsidiaries. All rights are reserved.
 *
 * Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior
 * written consent of Nokia.
 */

#ifndef CUSTOMALLOCATOR_HPP_
#define CUSTOMALLOCATOR_HPP_

#include <list>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>
#include <cstddef>
#include <functional>
#include <iosfwd>
#include <new>
#include <utility>

namespace HEIF
{
    class CustomAllocator;
}
HEIF::CustomAllocator* getDefaultAllocator();
bool setCustomAllocator(HEIF::CustomAllocator* customAllocator);
HEIF::CustomAllocator* getCustomAllocator();

void* customAllocate(size_t size);
void customDeallocate(void* ptr);

template <typename T>
T* customAllocateArray(size_t n)
{
    size_t size  = sizeof(T);
    auto* header = static_cast<size_t*>(customAllocate(sizeof(size_t) + size * n));
    T* ptr       = reinterpret_cast<T*>(header + 1);
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

// The CUSTOM_NEW / CUSTOM_DELETE macros are intentionally not parenthesized:
//   * `class` is a type-name used in sizeof(class), placement-new of `class`,
//     and explicit `~class()` destruction -- none of those grammar positions
//     accept a parenthesized type.
//   * `args` is a parens-tuple by call convention (callers always write
//     `CUSTOM_NEW(Foo, (a, b))`); wrapping it in parens here would expand to
//     `Foo((a, b))` and call the ctor with a single comma-tuple expression.
//
// DEPRECATED -- use customNew<T>(...) / customDelete(ptr) instead.
// Retained only for third-party code that may reference them; all in-tree
// call sites have been migrated to the function-template equivalents.
// NOLINTBEGIN(bugprone-macro-parentheses)
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
// NOLINTEND(bugprone-macro-parentheses)
#define CUSTOM_DELETE_ARRAY(object, class)               \
    {                                                    \
        customDestructAndDeallocateArray<class>(object); \
    }

// Type-safe replacements for CUSTOM_NEW / CUSTOM_DELETE.
// T is deduced or explicitly specified; no separate type token to get wrong.

/// Allocate via the custom allocator and placement-construct T.
/// Returns a raw T*; caller is responsible for pairing with customDelete().
template <typename T, typename... Args>
T* customNew(Args&&... args)
{
    void* mem = customAllocate(sizeof(T));
    try
    {
        return new (mem) T(std::forward<Args>(args)...);
    }
    catch (...)
    {
        customDeallocate(mem);
        throw;
    }
}

/// Destroy and deallocate a pointer returned by customNew<T>().
/// T is deduced from the pointer type -- no way to pass the wrong destructor.
/// nullptr is a no-op (matches free(3) / delete convention).
template <typename T>
void customDelete(T* ptr)
{
    if (ptr)
    {
        ptr->~T();
        customDeallocate(ptr);
    }
}

#if !defined(HEIF_GCC_ALLOCATOR_FIX)
#define HEIF_GCC_ALLOCATOR_FIX 0
#endif
template <typename T>
class Allocator
{
public:
    using value_type = T;

#if HEIF_GCC_ALLOCATOR_FIX == 1
    // GCC versions prior to 6 had incomplete, non-compliant c++11 allocator support.
    // and require declaring all of these, which in turn causes compliant implementations to work non-optimally.
    // Also if using 'libc++' we should not do this.
    using size_type       = std::size_t;
    using pointer         = T*;
    using const_pointer   = const T*;
    using reference       = T&;
    using const_reference = const T&;
    using difference_type = std::ptrdiff_t;
    template <typename U>
    struct rebind
    {
        using other = Allocator<U>;
    };

    template <class U, class... Args>
    void construct(U* p, Args&&... args)
    {
        ::new ((void*) p) U(std::forward<Args>(args)...);
    }
    template <class U>
    void destroy(U* p)
    {
        p->~U();
    }
#endif

    Allocator() noexcept = default;

    ~Allocator() = default;

    template <class U>
    // NOLINTBEGIN(google-explicit-constructor): the std::allocator interface
    // requires this rebind-style converting ctor to be implicit so the standard
    // library can transparently convert Allocator<T> to Allocator<U>.
    Allocator(const Allocator<U>&) noexcept
    {
    }
    // NOLINTEND(google-explicit-constructor)

    T* allocate(const std::size_t n) const
    {
        // When T is a pointer type (e.g. Allocator<SomeBox*> backing a vector
        // of pointers), sizeof(T) is intentionally sizeof(void*) -- we are
        // sizing storage for an array of pointers, not for the pointees.
        return static_cast<T*>(customAllocate(n * sizeof(T)));  // NOLINT(bugprone-sizeof-expression)
    }

    void deallocate(T* const p, std::size_t) const noexcept
    {
        // reinterpret_cast (not C-cast or static_cast) makes the deliberate
        // type-erasure explicit when T itself is a pointer type, satisfying
        // bugprone-multi-level-implicit-pointer-conversion.
        return customDeallocate(reinterpret_cast<void*>(p));
    }

    template <class U>
    bool operator==(const Allocator<U>&) const noexcept
    {
        return true;
    }
    template <class U>
    bool operator!=(const Allocator<U>&) const noexcept
    {
        return false;
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

template <typename K, typename V, typename Compare = std::less<K>>
using MultiMap = std::multimap<K, V, Compare, Allocator<std::pair<const K, V>>>;

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
    return UniquePtr<T, ParentWithVirtualDeleteOrSelf>(customNew<T>(std::forward<Args>(args)...));
}

namespace ISOBMFF
{
    class Exception
    {
    public:
        Exception() = default;
        explicit Exception(const char* msg)
            : mMessage(msg)
        {
        }
        virtual const char* what() const
        {
            return mMessage;
        }
        virtual ~Exception() = default;

    private:
        const char* mMessage;
    };
}  // namespace ISOBMFF

class RuntimeError : public ISOBMFF::Exception
{
public:
    explicit RuntimeError(const char* str)
        : Exception(str)
    {
    }

    explicit RuntimeError(const String& str)
        : Exception(str.c_str())
    {
    }
};

class LogicError : public ISOBMFF::Exception
{
public:
    explicit LogicError(const char* str)
        : Exception(str)
    {
    }
};

#endif  // CUSTOMALLOCATOR_HPP_
