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

#ifndef SAMPLEGROUP_HPP
#define SAMPLEGROUP_HPP

#include <string>
class SampleGroupDescriptionBox;
class SampleToGroupBox;

/**
 * @brief SampleGroup implementation
 * @details This is the base class to generate sample groups.
 */
class SampleGroup
{
public:
    SampleGroup(const std::string& groupingType);
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
    std::string mGroupingType; ///< Sample grouping type value
};


#endif /* end of include guard: SAMPLEGROUP_HPP */

