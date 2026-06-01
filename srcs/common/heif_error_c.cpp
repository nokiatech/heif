/* This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2026 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled
 * by Nokia Corporation and/or its subsidiaries. All rights are reserved.
 */

// Pure-C error-inspection surface over the per-thread
// `HEIF::ErrorInfo` slot.  See `heif_error_c.h` for the full contract.
//
// All the heavy lifting (the `thread_local` slot, the optional sink
// dispatch, the `errorOriginName` / `errorCodeName` lookup tables) is
// owned by `heiferrorinfo.cpp`.  This translation unit is intentionally
// thin: it forwards through the existing accessors so the C facade and
// the C++ slot can never drift.
//
// Compiled per-target (added to both `srcs/reader/CMakeLists.txt` and
// `srcs/writer/CMakeLists.txt` next to `heiferrorinfo.cpp`) so
// `HEIF_DLL_PUBLIC` resolves to the correct dllexport / visibility
// attribute for the consuming static-vs-shared target -- same rationale
// as documented inline next to `../common/heiferrorinfo.cpp`.

#include "heif_error_c.h"

#include <cstdint>
#include <cstring>

#include "heiferrorinfo.h"

namespace
{
    // Shared truncating-copy helper for the two `(buf, len)` accessors.
    // Returns the length of `src` in bytes (excluding the NUL), and
    // writes at most `len - 1` bytes plus a terminator into `buf` when
    // `len > 0`.  Mirrors the `snprintf(3)` return-value contract so a
    // probe call with `(nullptr, 0)` can be used to size the buffer.
    std::size_t copyTruncating(const char* src, char* buf, std::size_t len) noexcept
    {
        const std::size_t n = (src != nullptr) ? std::strlen(src) : 0U;
        if ((buf != nullptr) && (len > 0U))
        {
            const std::size_t toCopy = (n < (len - 1U)) ? n : (len - 1U);
            if (toCopy > 0U && src != nullptr)
            {
                std::memcpy(buf, src, toCopy);
            }
            buf[toCopy] = '\0';
        }
        return n;
    }
}  // namespace

extern "C"
{

int32_t heif_get_last_error_code(void)
{
    // ErrorCode is std::uint8_t today, but we widen the FFI
    // signature to int32_t so a future enum growth is non-breaking.
    return static_cast<int32_t>(::HEIF::getErrorInfo().code());
}

int32_t heif_get_last_error_origin(void)
{
    return static_cast<int32_t>(::HEIF::getErrorInfo().origin());
}

const char* heif_get_last_error_origin_name(void)
{
    return ::HEIF::errorOriginName(::HEIF::getErrorInfo().origin());
}

const char* heif_get_last_error_code_name(void)
{
    return ::HEIF::errorCodeName(::HEIF::getErrorInfo().code());
}

const char* heif_get_last_error_file(void)
{
    // ErrorInfo::file() already normalises a missing source pointer to
    // the empty string, so this is always a valid C string.
    return ::HEIF::getErrorInfo().file();
}

uint32_t heif_get_last_error_line(void)
{
    return static_cast<uint32_t>(::HEIF::getErrorInfo().line());
}

size_t heif_get_last_error_message(char* buf, size_t len)
{
    return copyTruncating(::HEIF::getErrorInfo().message(), buf, len);
}

size_t heif_get_last_error_cause(char* buf, size_t len)
{
    return copyTruncating(::HEIF::getErrorInfo().causeWhat(), buf, len);
}

int32_t heif_has_last_error(void)
{
    return ::HEIF::hasErrorInfo() ? 1 : 0;
}

void heif_clear_last_error(void)
{
    ::HEIF::clearErrorInfo();
}

const char* heif_error_code_name(int32_t code)
{
    // Pure stateless mapping -- does NOT touch the per-thread slot
    // (that's heif_get_last_error_code_name's job).  We narrow back
    // to the underlying enum width: the C ABI keeps int32_t for
    // forward compatibility per heif_error_c.h's signature contract,
    // but `errorCodeName` itself takes the enum type.  A code that
    // does not fit in the enum's underlying type still resolves to
    // the `ErrorCode(<n>)` sentinel because errorCodeName's switch
    // falls through to the default arm for anything it does not
    // recognise.
    return ::HEIF::errorCodeName(static_cast<::HEIF::ErrorCode>(static_cast<std::uint8_t>(code)));
}

const char* heif_error_origin_name(int32_t origin)
{
    // Same rationale as heif_error_code_name above.  ErrorOrigin is
    // also `: std::uint8_t` (see heiferrorinfo.h); a wider value
    // dropped in here surfaces as the `ErrorOrigin(<n>)` sentinel.
    return ::HEIF::errorOriginName(static_cast<::HEIF::ErrorOrigin>(static_cast<std::uint8_t>(origin)));
}

}  // extern "C"
