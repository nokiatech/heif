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

#include "samplegroup.hpp"
#include "samplegroupdescriptionbox.hpp"
#include "sampletogroupbox.hpp"

SampleGroup::SampleGroup(const FourCCInt& groupingType)
    : mGroupingType(groupingType)
{
}
