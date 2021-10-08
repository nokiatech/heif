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

#include "initialviewingorientationbox.hpp"
#include "bitstream.hpp"

InitialViewingOrientationBox::InitialViewingOrientationBox()
    : FullBox("iivo", 0, 0)
{
}

const InitialViewingOrientationBox::InitialViewingOrientationSample&
InitialViewingOrientationBox::getVrInitialOrientation() const
{
    return mVrInitialOrientation;
}

InitialViewingOrientationBox::InitialViewingOrientationSample& InitialViewingOrientationBox::getVrInitialOrientation()
{
    return mVrInitialOrientation;
}

void InitialViewingOrientationBox::writeBox(BitStream& output) const
{
    writeFullBoxHeader(output);
    mVrInitialOrientation.regions.at(0).write(output, false);
    output.write8Bits(mVrInitialOrientation.refreshFlag ? 0b10000000 : 0b0);
    updateSize(output);
}

void InitialViewingOrientationBox::parseBox(BitStream& input)
{
    parseFullBoxHeader(input);
    mVrInitialOrientation.regions.at(0).read(input, false);
    mVrInitialOrientation.refreshFlag = (input.read8Bits() & 0b1000000) != 0;
}


InitialViewingOrientationBox::InitialViewingOrientationSample::InitialViewingOrientationSample()
    : InitialViewingOrientationBox::SphereRegionSample()
{
    regions.push_back(SphereRegion());
}
