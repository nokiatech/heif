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

#include <Windows.h>
#include "OutputStreamInterface.h"
#include "customallocator.hpp"

namespace
{
    class OfStream : public HEIF::OutputStreamInterface
    {
        String mFilename;
        HANDLE hFile;
        std::uint64_t mPos;

    public:
        OfStream(const char* aFilename) : mFilename(aFilename)
        {
            mPos  = 0;
            hFile = CreateFileA(mFilename.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        }
        virtual ~OfStream() override
        {
            close();
        }
        void close()
        {
            if (is_open())
            {
                CloseHandle(hFile);
                hFile = INVALID_HANDLE_VALUE;
            }
        }
        bool is_open()
        {
            return (hFile != INVALID_HANDLE_VALUE);
        }
        virtual void seekp(std::uint64_t aPos) override
        {
            LARGE_INTEGER pos;
            pos.QuadPart = aPos;
            SetFilePointerEx(hFile, pos, NULL, FILE_BEGIN);
        }
        virtual std::uint64_t tellp() override
        {
            return mPos;
        }
        virtual void write(const void* aBuf, std::uint64_t aCount) override
        {
            //TODO: handle large 2gb+ writes. (win32 api limitation)
            WriteFile(hFile, aBuf, (DWORD)aCount, NULL, NULL);
            mPos += aCount;
        }
        virtual void remove() override
        {
            if (!mFilename.empty())
            {
                close();
                DeleteFileA(mFilename.c_str());
                mFilename.clear();
            }

        }
    };
}

namespace HEIF
{
    OutputStreamInterface* ConstructFileStream(const char* aFilename)
    {
        OutputStreamInterface* aFile = new OfStream(aFilename);
        if (!((OfStream*) aFile)->is_open())
        {
            delete aFile;
            aFile = nullptr;
        }
        return aFile;
    }
}