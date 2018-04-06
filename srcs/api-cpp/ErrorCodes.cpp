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

#include "ErrorCodes.h"

namespace HEIFPP
{
    Result convertErrorCode(HEIF::ErrorCode error)
    {
        if (error == HEIF::ErrorCode::OK)
        {
            return Result::OK;
        }
        else if (error == HEIF::ErrorCode::ALREADY_INITIALIZED)
        {
            return Result::ALREADY_INITIALIZED;
        }
        else if (error == HEIF::ErrorCode::PRIMARY_ITEM_NOT_SET)
        {
            return Result::PRIMARY_ITEM_NOT_SET;
        }
        else if (error == HEIF::ErrorCode::BRANDS_NOT_SET)
        {
            return Result::BRANDS_NOT_SET;
        }
        else if (error == HEIF::ErrorCode::HIDDEN_PRIMARY_ITEM)
        {
            return Result::HIDDEN_PRIMARY_ITEM;
        }
        else
        {
            return Result::ERROR_UNDEFINED;
        }
    }
}  // namespace HEIFPP