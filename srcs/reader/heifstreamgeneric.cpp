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

#include "heifstreamgeneric.hpp"
#include "customallocator.hpp"

#include "heifstreamfile.hpp"

#ifdef HEIF_USE_LINUX_FILESTREAM
#include "heifstreamlinux.hpp"
#endif  // HEIF_USE_LINUX_FILESTREAM

namespace HEIF
{
    StreamInterface* openFile(const char* filename)
    {
#ifdef HEIF_USE_LINUX_FILESTREAM
        return CUSTOM_NEW(LinuxStream, (filename));
#else
        return CUSTOM_NEW(FileStream, (filename));
#endif  // HEIF_USE_LINUX_FILESTREAM
    }
}  // namespace HEIF
