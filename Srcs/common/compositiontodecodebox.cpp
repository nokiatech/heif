/* Copyright (c) 2015, Nokia Technologies Ltd.
 * All rights reserved.
 *
 * Licensed under the Nokia High-Efficiency Image File Format (HEIF) License (the "License").
 *
 * You may not use the High-Efficiency Image File Format except in compliance with the License.
 * The License accompanies the software and can be found in the file "LICENSE.TXT".
 *
 * You may also obtain the License at:
 * https://nokiatech.github.io/heif/license.txt
 */

#include "compositiontodecodebox.hpp"
#include "bitstream.hpp"
#include "fullbox.hpp"

CompositionToDecodeBox::CompositionToDecodeBox() :
    FullBox("cslg", 0, 0),
    mCompositionToDtsShift(0),
    mLeastDecodeToDisplayDelta(0),
    mGreatestDecodeToDisplayDelta(0),
    mCompositionStartTime(0),
    mCompositionEndTime(0)
{
}

void CompositionToDecodeBox::setCompositionToDtsShift(const std::int32_t compositionToDtsShift)
{
    mCompositionToDtsShift = compositionToDtsShift;
}

std::int32_t CompositionToDecodeBox::getCompositionToDtsShift() const
{
    return mCompositionToDtsShift;
}

void CompositionToDecodeBox::setLeastDecodeToDisplayDelta(const std::int32_t leastDecodeToDisplayDelta)
{
    mLeastDecodeToDisplayDelta = leastDecodeToDisplayDelta;
}

std::int32_t CompositionToDecodeBox::getLeastDecodeToDisplayDelta() const
{
    return mLeastDecodeToDisplayDelta;
}

void CompositionToDecodeBox::setGreatestDecodeToDisplayDelta(const std::int32_t greatestDecodeToDisplayDelta)
{
    mGreatestDecodeToDisplayDelta = greatestDecodeToDisplayDelta;
}

std::int32_t CompositionToDecodeBox::getGreatestDecodeToDisplayDelta() const
{
    return mGreatestDecodeToDisplayDelta;
}

void CompositionToDecodeBox::setCompositionStartTime(const std::int32_t compositionStartTime)
{
    mCompositionStartTime = compositionStartTime;
}

std::int32_t CompositionToDecodeBox::getCompositionStartTime() const
{
    return mCompositionStartTime;
}

void CompositionToDecodeBox::setCompositionEndTime(const std::int32_t compositionEndTime)
{
    mCompositionEndTime = compositionEndTime;
}

std::int32_t CompositionToDecodeBox::getCompositionEndTime() const
{
    return mCompositionEndTime;
}

/// @todo Version > 0 support may be added if needed.
void CompositionToDecodeBox::writeBox(BitStream& bitstr)
{
    // Write box headers
    writeFullBoxHeader(bitstr);

    bitstr.write32Bits(static_cast<std::uint32_t>(mCompositionToDtsShift));
    bitstr.write32Bits(static_cast<std::uint32_t>(mLeastDecodeToDisplayDelta));
    bitstr.write32Bits(static_cast<std::uint32_t>(mGreatestDecodeToDisplayDelta));
    bitstr.write32Bits(static_cast<std::uint32_t>(mCompositionStartTime));
    bitstr.write32Bits(static_cast<std::uint32_t>(mCompositionEndTime));

    // Update the size of the movie box
    updateSize(bitstr);
}

/// @todo Version > 0 support may be added if needed.
void CompositionToDecodeBox::parseBox(BitStream& bitstr)
{
    //  First parse the box header
    parseFullBoxHeader(bitstr);

    mCompositionToDtsShift = static_cast<std::int32_t>(bitstr.read32Bits());
    mLeastDecodeToDisplayDelta = static_cast<std::int32_t>(bitstr.read32Bits());
    mGreatestDecodeToDisplayDelta = static_cast<std::int32_t>(bitstr.read32Bits());
    mCompositionStartTime = static_cast<std::int32_t>(bitstr.read32Bits());
    mCompositionEndTime = static_cast<std::int32_t>(bitstr.read32Bits());
}

