/* This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2018 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its subsidiaries. All rights are reserved.
 *
 * Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior written consent of Nokia.
 */

#ifndef SAMPLEGROUP_HPP
#define SAMPLEGROUP_HPP

#include "customallocator.hpp"
#include "fourccint.hpp"
class SampleGroupDescriptionBox;
class SampleToGroupBox;

/**
 * @brief SampleGroup implementation
 * @details This is the base class to generate sample groups.
 */
class SampleGroup
{
public:
    SampleGroup(const FourCCInt& groupingType);
    virtual ~SampleGroup() = default;

    /**
     * @brief Fill the SampleGroupDescription Box
     * @details  To be implemented by the inheriting class
     * @param [in] sgpd SampleGroupDescriptionBox. */
    virtual void fillSgpd(SampleGroupDescriptionBox* sgpd) = 0;

    /**
     * @brief Fill the SampleToGroup Box
     * @details To be implemented by the inheriting class
     * @param [in] sbgp SampleToGroupBox. */
    virtual void fillSbgp(SampleToGroupBox& sbgp) = 0;

protected:
    FourCCInt mGroupingType;  ///< Sample grouping type value
};


#endif /* end of include guard: SAMPLEGROUP_HPP */
