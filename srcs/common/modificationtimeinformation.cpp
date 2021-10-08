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

#include "modificationtimeinformation.hpp"

#include "bitstream.hpp"

ModificationTimeProperty::ModificationTimeProperty()
    : FullBox("mdft", 0, 0)
    , mModificationTime(0)
{
}

void ModificationTimeProperty::setModificationTime(const std::uint64_t modificationTime)
{
    mModificationTime = modificationTime;
}

std::uint64_t ModificationTimeProperty::getModificationTime() const
{
    return mModificationTime;
}

void ModificationTimeProperty::writeBox(BitStream& output) const
{
    writeFullBoxHeader(output);
    output.write64Bits(mModificationTime);
    updateSize(output);
}

void ModificationTimeProperty::parseBox(BitStream& input)
{
    parseFullBoxHeader(input);
    mModificationTime = input.read64Bits();
}
