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

#include "creationtimeinformation.hpp"

#include "bitstream.hpp"

CreationTimeProperty::CreationTimeProperty()
    : FullBox("crtt", 0, 0)
    , mCreationTime(0)
{
}

void CreationTimeProperty::setCreationTime(const std::uint64_t modificationTime)
{
    mCreationTime = modificationTime;
}

std::uint64_t CreationTimeProperty::getCreationTime() const
{
    return mCreationTime;
}

void CreationTimeProperty::writeBox(BitStream& output) const
{
    writeFullBoxHeader(output);
    output.write64Bits(mCreationTime);
    updateSize(output);
}

void CreationTimeProperty::parseBox(BitStream& input)
{
    parseFullBoxHeader(input);
    mCreationTime = input.read64Bits();
}
