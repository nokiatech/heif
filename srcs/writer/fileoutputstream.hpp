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

#ifndef FILEOUTPUTSTREAM_HPP
#define FILEOUTPUTSTREAM_HPP

#if defined(_WIN32)
#include <Windows.h>
#else
#include <fstream>
#endif

#include "OutputStreamInterface.h"
#include "customallocator.hpp"

namespace HEIF
{
    class FileOutputStream : public OutputStreamInterface
    {
    public:
        FileOutputStream(const char* aFilename);

        ~FileOutputStream() override;

        bool is_open();

        void seekp(std::uint64_t aPos) override;

        std::uint64_t tellp() override;

        void write(const void* aBuf, std::uint64_t aCount) override;

        void remove() override;

        String getFileName() const;

    private:
        String mFilename;

#if defined(_WIN32)
        void close();

        HANDLE hFile;
        std::uint64_t mPos;
#else
        std::ofstream mFile;
#endif
    };
}  // namespace HEIF

#endif
