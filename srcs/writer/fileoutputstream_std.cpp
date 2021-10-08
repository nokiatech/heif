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

#include <fstream>

#include "OutputStreamInterface.h"
#include "customallocator.hpp"
#include "fileoutputstream.hpp"


namespace HEIF
{
    FileOutputStream::FileOutputStream(const char* aFilename)
        : mFilename(aFilename)
        , mFile(mFilename.c_str(), std::ofstream::out | std::ofstream::binary | std::ofstream::trunc)
    {
    }

    FileOutputStream::~FileOutputStream()
    {
        if (mFile.is_open())
        {
            mFile.close();
        }
    }

    bool FileOutputStream::is_open()
    {
        return mFile.is_open();
    }

    void FileOutputStream::seekp(std::uint64_t aPos)
    {
        mFile.seekp(static_cast<std::streamoff>(aPos));
    }

    std::uint64_t FileOutputStream::tellp()
    {
        return static_cast<std::uint64_t>(mFile.tellp());
    }

    void FileOutputStream::write(const void* aBuf, std::uint64_t aCount)
    {
        mFile.write(static_cast<const char*>(aBuf), static_cast<std::streamsize>(aCount));
    }

    void FileOutputStream::remove()
    {
        if (!mFilename.empty())
        {
            if (mFile.is_open())
            {
                mFile.close();
            }
            std::remove(mFilename.c_str());
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
        if (!static_cast<FileOutputStream*>(aFile)->is_open())
        {
            delete aFile;
            aFile = nullptr;
        }
        return aFile;
    }
}  // namespace HEIF
