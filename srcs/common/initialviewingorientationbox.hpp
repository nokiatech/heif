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

#ifndef INITIALVIEWINGORIENTATIONBOX_HPP
#define INITIALVIEWINGORIENTATIONBOX_HPP

#include <cstdint>
#include "commontypes.hpp"
#include "customallocator.hpp"
#include "fullbox.hpp"

namespace ISOBMFF
{
    class BitStream;
}

class InitialViewingOrientationBox : public FullBox
{
public:
    struct SphereRegionSample
    {
        // There is alawys exactly one region in this use case of the SphereRegionSample
        // struct, but struct specification allows multiple regions per SphereRegionSample.
        std::vector<SphereRegion> regions;
    };

    struct InitialViewingOrientationSample : SphereRegionSample
    {
        bool refreshFlag = false;
        InitialViewingOrientationSample();
    };

    InitialViewingOrientationBox();
    virtual ~InitialViewingOrientationBox() = default;

    const InitialViewingOrientationSample& getVrInitialOrientation() const;
    InitialViewingOrientationSample& getVrInitialOrientation();

    virtual void writeBox(ISOBMFF::BitStream& output) const;
    virtual void parseBox(ISOBMFF::BitStream& input);

private:
    InitialViewingOrientationSample mVrInitialOrientation;
};

#endif
