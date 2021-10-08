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

#include "timeutility.hpp"

#include <ctime>

namespace TimeUtility
{
    bool useFakeTime       = false;
    std::uint64_t fakeTime = 0;

    void setFakeTime(uint64_t fakeTimeToSet)
    {
        useFakeTime = true;
        fakeTime    = fakeTimeToSet;
    }


    std::uint64_t getSecondsSince1904()
    {
        if (useFakeTime)
        {
            return fakeTime;
        }

        static const std::uint64_t SECONDS_FROM_1904_TO_1970 = 2082844800u;
        return static_cast<std::uint64_t>(std::time(nullptr)) + SECONDS_FROM_1904_TO_1970;
    }

}  // namespace TimeUtility
