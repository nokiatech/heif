/* This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2026 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled
 * by Nokia Corporation and/or its subsidiaries. All rights are reserved.
 */

// Pure-C runtime-version accessors -- see `heif_version_c.h` for the
// full contract and rationale.  Implementation is intentionally
// trivial: the integer triple comes straight from the build-time
// macros baked by `buildinfo.hpp.in`, and the string accessor
// forwards to the existing `BuildInfo::Version` literal so the C
// surface and the C++ surface (`Reader::GetVersion()` /
// `Writer::GetVersion()`) cannot disagree.
//
// Compiled per-target (added to both `srcs/reader/CMakeLists.txt`
// and `srcs/writer/CMakeLists.txt` next to `heif_error_c.cpp`) so
// `HEIF_DLL_PUBLIC` resolves to the correct dllexport / visibility
// attribute for the consuming static-vs-shared target -- same
// rationale as documented inline next to `heiferrorinfo.cpp`.

#include "heif_version_c.h"

#include <cstring>

#include "buildinfo.hpp"

extern "C"
{

void heif_get_runtime_version(uint32_t* out_major, uint32_t* out_minor, uint32_t* out_patch)
{
    // Each out-pointer is independently optional -- a binding that only
    // wants the major (for a SOVERSION handshake) passes NULL for the
    // other two.  No error path: this is a pure read of compile-time
    // constants; we cannot fail.
    if (out_major != nullptr)
    {
        *out_major = static_cast<uint32_t>(HEIF_VERSION_MAJOR);
    }
    if (out_minor != nullptr)
    {
        *out_minor = static_cast<uint32_t>(HEIF_VERSION_MINOR);
    }
    if (out_patch != nullptr)
    {
        *out_patch = static_cast<uint32_t>(HEIF_VERSION_PATCH);
    }
}

const char* heif_get_runtime_version_string(void)
{
    // BuildInfo::Version is a `constexpr auto` initialised from the
    // CMake-substituted `@GIT_DESCRIBE@` token (see buildinfo.hpp.in
    // and the matching configure_file in srcs/CMakeLists.txt).  Its
    // storage is a string literal embedded in the library, so the
    // pointer is valid for the lifetime of the loaded module -- exactly
    // what the C contract promises.
    return BuildInfo::Version;
}

uint32_t heif_get_runtime_version_number(void)
{
    // Same encoding as the build-time HEIF_VERSION_NUMBER macro so a
    // binding can compare runtime ?>= compile-time with a single
    // integer compare.
    return static_cast<uint32_t>(HEIF_VERSION_NUMBER);
}

int32_t heif_get_abi_handshake(heif_abi_handshake_t* out)
{
    if (out == nullptr)
    {
        return -1;
    }
    const uint32_t callerSize = out->struct_size;
    if (callerSize < 4)
    {
        return -1;
    }

    // Build the library-side record on the stack.
    heif_abi_handshake_t lib;
    lib.struct_size = static_cast<uint32_t>(sizeof(heif_abi_handshake_t));
    lib.major       = static_cast<uint32_t>(HEIF_VERSION_MAJOR);
    lib.minor       = static_cast<uint32_t>(HEIF_VERSION_MINOR);
    lib.patch       = static_cast<uint32_t>(HEIF_VERSION_PATCH);
    lib.flags       = 0;

    // Copy the prefix the caller can receive; zero any trailing
    // bytes the caller reserved but the library does not yet fill.
    const uint32_t copyBytes = callerSize < sizeof(lib) ? callerSize : static_cast<uint32_t>(sizeof(lib));
    std::memcpy(out, &lib, copyBytes);
    if (callerSize > sizeof(lib))
    {
        std::memset(reinterpret_cast<char*>(out) + sizeof(lib), 0, callerSize - sizeof(lib));
    }
    // Stamp the caller's original struct_size back so the caller
    // can still inspect it after the call.
    out->struct_size = callerSize;
    return 0;
}

}  // extern "C"
