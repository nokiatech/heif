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

#include <Windows.h>

#include "OutputStreamInterface.h"
#include "customallocator.hpp"
#include "fileoutputstream.hpp"

namespace HEIF
{
    FileOutputStream::FileOutputStream(const char* aFilename)
        : mFilename(aFilename)
    {
        mPos  = 0;
        hFile = CreateFileA(mFilename.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    }

    FileOutputStream::~FileOutputStream()
    {
        close();
    }

    void FileOutputStream::close()
    {
        if (is_open())
        {
            CloseHandle(hFile);
            hFile = INVALID_HANDLE_VALUE;
        }
    }

    bool FileOutputStream::is_open()
    {
        return (hFile != INVALID_HANDLE_VALUE);
    }

    void FileOutputStream::seekp(std::uint64_t aPos)
    {
        LARGE_INTEGER pos;
        pos.QuadPart = aPos;
        SetFilePointerEx(hFile, pos, NULL, FILE_BEGIN);
    }

    std::uint64_t FileOutputStream::FileOutputStream::tellp()
    {
        return mPos;
    }

    void FileOutputStream::write(const void* aBuf, std::uint64_t aCount)
    {
        // Win32 `WriteFile` takes a `DWORD` (unsigned 32-bit) byte count,
        // so a single call cannot move more than ~4 GiB and a naive
        // `(DWORD) aCount` cast silently truncates anything larger.
        // Loop in `MAX_CHUNK`-sized writes so the surface contract
        // ("write `aCount` bytes") holds for arbitrary 64-bit counts.
        // `MAX_CHUNK` is set just below `INT32_MAX` (2 GiB - 64 KiB)
        // for headroom against any Win32 internal arithmetic that may
        // sign-extend the count; matches the chunking convention used
        // by the standard library's `std::filebuf` on MSVC.
        constexpr std::uint64_t MAX_CHUNK = 0x7FFF0000ULL;  // 2 GiB - 64 KiB

        const std::uint8_t* p = static_cast<const std::uint8_t*>(aBuf);
        std::uint64_t remaining = aCount;
        while (remaining > 0U)
        {
            const DWORD chunk = (remaining > MAX_CHUNK)
                                    ? static_cast<DWORD>(MAX_CHUNK)
                                    : static_cast<DWORD>(remaining);
            DWORD written = 0U;
            // OutputStreamInterface::write is `void`-returning by contract
            // (see srcs/api/common/heifoutputstreaminterface.h) -- there is
            // no in-band error path.  A failed WriteFile leaves the stream
            // in a state that the writer-side adversarial suite
            // (adversarial_writer_test.cpp) is the canonical contract pin
            // for; we still advance `mPos` by the *requested* count so that
            // a subsequent caller's offset accounting matches what they
            // asked for, mirroring the original single-shot behavior.
            (void) WriteFile(hFile, p, chunk, &written, NULL);
            p += chunk;
            remaining -= chunk;
        }
        mPos += aCount;
    }

    void FileOutputStream::remove()
    {
        if (!mFilename.empty())
        {
            close();
            DeleteFileA(mFilename.c_str());
            mFilename.clear();
        }
    }

    String FileOutputStream::getFileName() const
    {
        return mFilename;
    }

    OutputStreamInterface* ConstructFileStream(const char* aFilename)
    {
        OutputStreamInterface* aFile = new FileOutputStream(aFilename);
        if (!((FileOutputStream*) aFile)->is_open())
        {
            delete aFile;
            aFile = nullptr;
        }
        return aFile;
    }
}  // namespace HEIF
