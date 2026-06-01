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

#include "helpers.h"

#include <cstdio>
#include <limits>
#include <string>

#include "ErrorCodes.h"
#include "ErrorContext.h"

namespace HEIFPP
{
    class CodedImageItem;
    class DecoderConfig;
    class DerivedImageItem;
    class ImageItem;
    class Item;
    class MetaItem;
    class Sample;
    class Track;
}  // namespace HEIFPP

namespace HEIFPP
{
    // Sink-routed implementation of HEIF_LOG_INVARIANT (declared in helpers.h).
    //
    // Records the failure into the per-thread ErrorContext slot with
    // source = Internal_Invariant and code = ERROR_UNDEFINED, then falls back
    // to a single stderr line ONLY if no HEIFPP sink is installed -- so an
    // unconfigured embedder still sees the failure rather than silent
    // breakage of an internal-consistency check.
    //
    // noexcept: the call site is in branches that are already handling a
    // failure path; throwing here would change observable behavior and risk
    // unwinding through a destructor or noexcept boundary. We swallow any
    // std::bad_alloc from string construction.
    void reportInvariant(const char* file, int line, const char* msg) noexcept
    {
        try
        {
            ErrorContext ctx;
            ctx.code    = Result::ERROR_UNDEFINED;
            ctx.source  = ErrorSource::Internal_Invariant;
            ctx.message = (msg != nullptr) ? msg : "<no message>";
            ctx.file    = (file != nullptr) ? file : "<unknown>";
            ctx.line    = line;
            ctx.valid   = true;
            const bool sinkWillFire = hasErrorSink();
            setLastError(ctx); // dispatches to sink if installed
            if (!sinkWillFire)
            {
                // No sink -- keep the historical "loud failure" behavior so
                // an unconfigured embedder still notices broken invariants.
                std::fprintf(stderr,
                             "[heif] [Internal_Invariant@%s:%d] HEIF invariant violated: %s\n",
                             ctx.file, ctx.line, ctx.message.c_str());
            }
        }
        catch (...)
        {
            // Best-effort fallback: if even the slot population threw
            // (std::bad_alloc on the std::string), at least emit one line.
            std::fprintf(stderr,
                         "[heif] [Internal_Invariant@%s:%d] HEIF invariant violated (slot population failed): %s\n",
                         (file != nullptr) ? file : "<unknown>",
                         line,
                         (msg != nullptr) ? msg : "<no message>");
        }
    }

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
        HEIF_LOG_INVARIANT(false, "LinkArray::removeLink: target was not linked");
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
        return static_cast<uint32_t>(mList.size());
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
        return static_cast<std::uint8_t>((shift << 1) + 1);
    }

    BitStream::BitStream(std::uint8_t* aData, std::uint32_t aLen)
        : mSrc(aData)
        , mLen(aLen)
         
    {
    }

    std::uint32_t BitStream::peekBits(std::uint32_t bits)
    {
        std::uint32_t res      = 0;
        std::uint64_t tempCBit = mCBit;
        for (std::uint32_t i = 0; i < bits; i++)
        {
            auto index         = static_cast<std::uint32_t>(tempCBit / 8);
            std::uint32_t cbit = static_cast<std::uint8_t>(tempCBit - (static_cast<std::uint64_t>(index) * 8));
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
            std::uint32_t cbit = static_cast<std::uint8_t>(mCBit - (static_cast<std::uint64_t>(index) * 8));
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
                std::uint32_t cbit = static_cast<std::uint8_t>(mCBit - (static_cast<std::uint64_t>(index) * 8));

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
        return (static_cast<uint64_t>(mLen) * 8) - mCBit;
    }

    std::int32_t BitStream::getSignedExpGolombCode()  // se(v)
    {
        std::uint32_t codeNum = getExpGolombCode();
        auto signedVal        = static_cast<std::int32_t>((codeNum + 1) >> 1);

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
