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

#ifndef TRACKTYPEBOX_HPP
#define TRACKTYPEBOX_HPP

#include "brandandcompatiblebrandsbasebox.hpp"
#include "fullbox.hpp"

/** @brief Track Type Box class. Box payload is the same between ftyp and ttyp, so implementation is shared.
 */
class TrackTypeBox : public BrandAndCompatibleBrandsBaseBox<FullBox>
{
public:
    TrackTypeBox();
    ~TrackTypeBox() override = default;
};

#endif  // TRACKTYPEBOX_HPP
