/*
 * This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2018 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its
 * subsidiaries. All rights are reserved. Copying, including reproducing, storing, adapting or translating, any or all
 * of this material requires the prior written consent of Nokia.
 */

#pragma once

#include "heifcommondatatypes.h"

namespace HEIFPP
{
    enum class Result
    {
        OK                   = 0,
        INDEX_OUT_OF_BOUNDS  = 1,
        ALREADY_INITIALIZED  = 2,
        PRIMARY_ITEM_NOT_SET = 3,
        BRANDS_NOT_SET       = 4,
        HIDDEN_PRIMARY_ITEM  = 5,
        INVALID_HANDLE       = 6,
        ALREADY_IN_GROUP     = 7,
        ALREADY_SET          = 8,
        INVALID_CONFIG       = 9,
        ERROR_UNDEFINED      = 999
    };

    Result convertErrorCode(HEIF::ErrorCode error);
}  // namespace HEIFPP
