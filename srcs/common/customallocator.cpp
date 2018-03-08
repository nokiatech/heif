/* This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2018 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its subsidiaries. All rights are reserved.
 *
 * Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior written consent of Nokia.
 */

#include "customallocator.hpp"

HEIF::CustomAllocator::CustomAllocator()
{
    // nothing
}

HEIF::CustomAllocator::~CustomAllocator()
{
    // nothing
}

class DefaultAllocator : public HEIF::CustomAllocator
{
public:
    DefaultAllocator()
    {
    }
    ~DefaultAllocator()
    {
    }

    void* allocate(size_t n, size_t size) override
    {
        return malloc(n * size);
    }
    void deallocate(void* ptr) override
    {
        free(ptr);
    }
};

alignas(DefaultAllocator) char defaultAllocatorData[sizeof(DefaultAllocator)];
HEIF::CustomAllocator* defaultAllocator;
static HEIF::CustomAllocator* customAllocator;

bool setCustomAllocator(HEIF::CustomAllocator* customAllocator_)
{
    if (!customAllocator || !customAllocator_)
    {
        customAllocator = customAllocator_;
        return true;
    }
    else
    {
        return false;
    }
}

HEIF::CustomAllocator* getDefaultAllocator()
{
    if (!defaultAllocator)
    {
        defaultAllocator = new (defaultAllocatorData) DefaultAllocator();
    }
    return defaultAllocator;
}

HEIF::CustomAllocator* getCustomAllocator()
{
    if (!customAllocator)
    {
        defaultAllocator = getDefaultAllocator();
        customAllocator  = defaultAllocator;
    }
    return customAllocator;
}

void* customAllocate(size_t size)
{
    return getCustomAllocator()->allocate(size, 1);
}

void customDeallocate(void* ptr)
{
    getCustomAllocator()->deallocate(ptr);
}
