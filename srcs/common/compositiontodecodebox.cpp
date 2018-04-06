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

#include "compositiontodecodebox.hpp"
#include <limits>
#include "bitstream.hpp"
#include "fullbox.hpp"

CompositionToDecodeBox::CompositionToDecodeBox()
    : FullBox("cslg", 0, 0)
    , mCompositionToDtsShift(0)
    , mLeastDecodeToDisplayDelta(0)
    , mGreatestDecodeToDisplayDelta(0)
    , mCompositionStartTime(0)
    , mCompositionEndTime(0)
{
}

namespace
{
    bool requires64Bits(const int64_t value)
    {
        if ((value > std::numeric_limits<std::int32_t>::max()) || (value < std::numeric_limits<std::int32_t>::min()))
        {
            return true;
        }
        return false;
    }
}  // anonymous namespace

void CompositionToDecodeBox::setCompositionToDtsShift(const std::int64_t compositionToDtsShift)
{
    mCompositionToDtsShift = compositionToDtsShift;
    updateVersion();
}

std::int64_t CompositionToDecodeBox::getCompositionToDtsShift() const
{
    return mCompositionToDtsShift;
}

void CompositionToDecodeBox::setLeastDecodeToDisplayDelta(const std::int64_t leastDecodeToDisplayDelta)
{
    mLeastDecodeToDisplayDelta = leastDecodeToDisplayDelta;
    updateVersion();
}

std::int64_t CompositionToDecodeBox::getLeastDecodeToDisplayDelta() const
{
    return mLeastDecodeToDisplayDelta;
}

void CompositionToDecodeBox::setGreatestDecodeToDisplayDelta(const std::int64_t greatestDecodeToDisplayDelta)
{
    mGreatestDecodeToDisplayDelta = greatestDecodeToDisplayDelta;
    updateVersion();
}

std::int64_t CompositionToDecodeBox::getGreatestDecodeToDisplayDelta() const
{
    return mGreatestDecodeToDisplayDelta;
}

void CompositionToDecodeBox::setCompositionStartTime(const std::int64_t compositionStartTime)
{
    mCompositionStartTime = compositionStartTime;
    updateVersion();
}

std::int64_t CompositionToDecodeBox::getCompositionStartTime() const
{
    return mCompositionStartTime;
}

void CompositionToDecodeBox::setCompositionEndTime(const std::int64_t compositionEndTime)
{
    mCompositionEndTime = compositionEndTime;
    updateVersion();
}

std::int64_t CompositionToDecodeBox::getCompositionEndTime() const
{
    return mCompositionEndTime;
}

void CompositionToDecodeBox::updateVersion()
{
    if (requires64Bits(mCompositionToDtsShift) || requires64Bits(mLeastDecodeToDisplayDelta) ||
        requires64Bits(mGreatestDecodeToDisplayDelta) || requires64Bits(mCompositionStartTime) ||
        requires64Bits(mCompositionEndTime))
    {
        setVersion(1);
    }
    else
    {
        setVersion(0);
    }
}

void CompositionToDecodeBox::writeBox(ISOBMFF::BitStream& bitstr) const
{
    // Write box headers
    writeFullBoxHeader(bitstr);

    if (getVersion() == 0)
    {
        bitstr.write32Bits(static_cast<std::uint32_t>(mCompositionToDtsShift));
        bitstr.write32Bits(static_cast<std::uint32_t>(mLeastDecodeToDisplayDelta));
        bitstr.write32Bits(static_cast<std::uint32_t>(mGreatestDecodeToDisplayDelta));
        bitstr.write32Bits(static_cast<std::uint32_t>(mCompositionStartTime));
        bitstr.write32Bits(static_cast<std::uint32_t>(mCompositionEndTime));
    }
    else
    {
        bitstr.write64Bits(static_cast<std::uint64_t>(mCompositionToDtsShift));
        bitstr.write64Bits(static_cast<std::uint64_t>(mLeastDecodeToDisplayDelta));
        bitstr.write64Bits(static_cast<std::uint64_t>(mGreatestDecodeToDisplayDelta));
        bitstr.write64Bits(static_cast<std::uint64_t>(mCompositionStartTime));
        bitstr.write64Bits(static_cast<std::uint64_t>(mCompositionEndTime));
    }

    // Update the size of the movie box
    updateSize(bitstr);
}

void CompositionToDecodeBox::parseBox(ISOBMFF::BitStream& bitstr)
{
    //  First parse the box header
    parseFullBoxHeader(bitstr);

    if (getVersion() == 0)
    {
        mCompositionToDtsShift        = static_cast<std::int32_t>(bitstr.read32Bits());
        mLeastDecodeToDisplayDelta    = static_cast<std::int32_t>(bitstr.read32Bits());
        mGreatestDecodeToDisplayDelta = static_cast<std::int32_t>(bitstr.read32Bits());
        mCompositionStartTime         = static_cast<std::int32_t>(bitstr.read32Bits());
        mCompositionEndTime           = static_cast<std::int32_t>(bitstr.read32Bits());
    }
    else
    {
        mCompositionToDtsShift        = static_cast<std::int64_t>(bitstr.read64Bits());
        mLeastDecodeToDisplayDelta    = static_cast<std::int64_t>(bitstr.read64Bits());
        mGreatestDecodeToDisplayDelta = static_cast<std::int64_t>(bitstr.read64Bits());
        mCompositionStartTime         = static_cast<std::int64_t>(bitstr.read64Bits());
        mCompositionEndTime           = static_cast<std::int64_t>(bitstr.read64Bits());
    }
}
