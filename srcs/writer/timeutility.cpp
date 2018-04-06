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

#include "timeutility.hpp"
#include <ctime>

unsigned int getSecondsSince1904()
{
    static const unsigned int SECONDS_FROM_1904_TO_1970 = 2082844800u;
    return (static_cast<unsigned int>(std::time(nullptr)) + SECONDS_FROM_1904_TO_1970);
}
