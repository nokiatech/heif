/*
 * This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2026 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its
 * subsidiaries. All rights are reserved. Copying, including reproducing, storing, adapting or translating, any or all
 * of this material requires the prior written consent of Nokia.
 */

#pragma once

#include <cstdint>
#include <stdexcept>
#include <utility>
#include <vector>

// HEIF_ASSERT: a fatal precondition guard, *always* active (release builds included).
// Intended for situations where, if the condition is false, the very next line
// of code would cause a hard fault (null deref, integer underflow, etc.).
// Originally this was no-op'd in NDEBUG builds, silently turning every check
// into dead code; now it always evaluates the expression and throws on failure
// so misuse can never go unnoticed.
//
// Use HEIF_LOG_INVARIANT for "shouldn't reach here" markers in branches that
// are already handled gracefully (return early, log, continue) and where
// terminating the host program would be inappropriate.
#if (defined(_DEBUG) || defined(DEBUG)) || (!defined(NDEBUG))
#define HEIF_DEBUG
#endif

// Stringify __LINE__ so the assertion message embeds the source location at
// compile time (no per-call snprintf cost). The two-level macro indirection is
// the standard idiom for stringifying a token-paste of a macro value.
#define HEIFPP_ASSERT_STR_(x) #x
#define HEIFPP_ASSERT_STR(x)  HEIFPP_ASSERT_STR_(x)

#define HEIF_ASSERT(x)                                                                                                 \
    do                                                                                                                 \
    {                                                                                                                  \
        if (!(x))                                                                                                      \
        {                                                                                                              \
            throw std::logic_error("HEIF_ASSERT failed: " #x " at " __FILE__ ":" HEIFPP_ASSERT_STR(__LINE__));         \
        }                                                                                                              \
    } while (0)

// HEIF_LOG_INVARIANT: a non-fatal "this branch should be unreachable" marker.
// Use this at sites where the surrounding code already handles the failure
// (returns an error code, breaks, ignores) and where throwing would be a
// behavior change that could break callers relying on the silent-skip.
//
// Routes through HEIFPP::reportInvariant(file, line, msg) which:
//   - populates the per-thread ErrorContext slot with
//     source = ErrorSource::Internal_Invariant, code = INVARIANT_VIOLATION
//   - dispatches to the installed HEIFPP error sink (if any)
//   - falls back to writing one line to stderr if no sink is installed,
//     so an unconfigured embedder still sees the failure.
//
// This macro honours the HEIFPP sink contract: embedders that have
// installed a sink receive every invariant failure through the sink
// and stderr stays clean. If no sink is installed, falls back to stderr.
namespace HEIFPP { void reportInvariant(const char* file, int line, const char* msg) noexcept; }
#define HEIF_LOG_INVARIANT(cond, msg)                                              \
    do                                                                             \
    {                                                                              \
        if (!(cond))                                                               \
        {                                                                          \
            ::HEIFPP::reportInvariant(__FILE__, __LINE__, (msg));                  \
        }                                                                          \
    } while (0)

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
#define IsItemIn(a, b) (FindItemIn((a), (b)) != (a).end())

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

    public:
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
        std::uint64_t mCBit{0};
        std::uint32_t mWriteByte{0};
    };


}  // namespace HEIFPP
