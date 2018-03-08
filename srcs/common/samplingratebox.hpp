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

#ifndef SAMPLINGRATEBOX_HPP
#define SAMPLINGRATEBOX_HPP

#include <vector>
#include "customallocator.hpp"
#include "fullbox.hpp"

/**
 * @brief Sampling Rate Box class
 * @details 'srat' box implementation as specified in the ISOBMFF specification.
 */
class SamplingRateBox : public FullBox
{
public:
    SamplingRateBox();
    SamplingRateBox(const SamplingRateBox& box);
    virtual ~SamplingRateBox() = default;

    /// @returns uint32_t sampling rate
    std::uint32_t getSamplingRate() const;

    /** @brief Sets sampling rate into box.
     *  @param [in] samplingRate std::uint32_t Sampling rate */
    void setSamplingRate(std::uint32_t samplingRate);

    /**
     * @brief Serialize box data to the ISOBMFF::BitStream.
     * @see Box::writeBox()
     */
    virtual void writeBox(ISOBMFF::BitStream& bitstr) const;

    /**
     * @brief Deserialize box data from the ISOBMFF::BitStream.
     * @see Box::parseBox()
     */
    virtual void parseBox(ISOBMFF::BitStream& bitstr);

private:
    std::uint32_t mSamplingRate;
};

#endif /* end of include guard: SAMPLINGRATEBOX_HPP */
