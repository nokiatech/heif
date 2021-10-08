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

#ifndef TIMEUTILITY_HPP
#define TIMEUTILITY_HPP

#include <cstdint>

namespace TimeUtility
{
    /**
     * @brief setFakeTime For testing purposes, set value returned by getSecondsSince1904().
     * @param fakeTimeToSet Value which will be returned by getSecondsSince1904().
     */
    void setFakeTime(uint64_t fakeTimeToSet);

    /** @return Seconds passed since 1st of January 1904. */
    std::uint64_t getSecondsSince1904();
}  // namespace TimeUtility

#endif  // TIMEUTILITY_HPP
