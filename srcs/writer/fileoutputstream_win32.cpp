/* This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2021 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
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
        // TODO: handle large 2gb+ writes. (win32 api limitation)
        WriteFile(hFile, aBuf, (DWORD) aCount, NULL, NULL);
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
