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

#ifndef HEIFFILEALLOCATOR_H
#define HEIFFILEALLOCATOR_H

#include <cstdlib>
#include "heifexport.h"

namespace HEIF
{
    class HEIF_DLL_PUBLIC CustomAllocator
    {
    public:
        /** Allocate n objects each of size size */
        virtual void* allocate(size_t n, size_t size) = 0;

        /** Release a pointer returned from allocate */
        virtual void deallocate(void* ptr) = 0;

    protected:
        CustomAllocator()          = default;
        virtual ~CustomAllocator() = default;
    };
}  // namespace HEIF

#endif  // HEIFFILEALLOCATOR_H
