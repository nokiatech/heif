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

#include <fstream>
#include "OutputStreamInterface.h"
#include "customallocator.hpp"

namespace
{
    class OfStream : public HEIF::OutputStreamInterface
    {
    public:
        OfStream(const char* aFilename)
            : mFilename(aFilename),
            mFile(mFilename.c_str(), std::ofstream::out | std::ofstream::binary | std::ofstream::trunc)
        {
        }
        ~OfStream()
        {
            if (mFile.is_open())
            {
                mFile.close();
            }
        }
        bool is_open()
        {
            return mFile.is_open();
        }
        virtual void seekp(std::uint64_t aPos) override
        {
            mFile.seekp((std::streamoff)aPos);
        }
        virtual std::uint64_t tellp() override
        {
            return (std::uint64_t)mFile.tellp();
        }
        virtual void write(const void* aBuf, std::uint64_t aCount) override
        {
            mFile.write((const char*) aBuf, (std::streamsize)aCount);
        }
        virtual void remove() override
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
    private:
        String mFilename;
        std::ofstream mFile;
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
