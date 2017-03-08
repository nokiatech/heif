/* Copyright (c) 2015-2017, Nokia Technologies Ltd.
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

#ifndef TIMEWRITER_HPP
#define TIMEWRITER_HPP

#include "timetosamplebox.hpp"

class CompositionOffsetBox;
class CompositionToDecodeBox;

/**
 * @brief Class for writing TimeToSampleBox, CompositionOffsetBox and CompositionToDecodeBox.
 * @details CompositionOffsetBox and CompositionToDecodeBox are not always required. Need for them should be
 * queried from methods isCompositionOffsetBoxRequired() and isCompositionToDecodeBoxRequired().
 */
class TimeWriter
{
public:
    /**
     * @brief TimeWriter constructor
     * @param clockTicks Timescale from the Movie Header Box, number of time units passing in one second.
     */
    TimeWriter(std::uint32_t clockTicks);
    virtual ~TimeWriter() = default;

    /**
     * @brief Set display rate
     * @param displayRate New display rate
     */
    void setDisplayRate(std::uint32_t displayRate);

    /**
     * @brief Set sample orders
     * @details Based on input the need for CompositionOffsetBox and CompositionToDecodeBox is decided.
     * @param decodeOrder  Sample decode order vector
     * @param displayOrder Sample display order vector
     * @param nonOutput Make the first sample non-output sample by setting its display time to the minimum value.
     */
    void loadOrder(const std::vector<std::uint32_t>& decodeOrder, const std::vector<std::uint32_t>& displayOrder,
        bool nonOutput);

    /**
     * @brief Write TimeToSampleBox content
     * @param [out] timeToSampleBox TimeToSampleBox to write into
     */
    void fillTimeToSampleBox(TimeToSampleBox& timeToSampleBox);

    /**
     * @brief Write CompositionOffsetBox content
     * @param [out] compositionOffsetBox CompositionOffsetBox to write into
     */
    void fillCompositionOffsetBox(CompositionOffsetBox& compositionOffsetBox);

    /**
     * @brief Write CompositionToDecodeBox content
     * @param [out] compositionToDecodeBox CompositionToDecodeBox to write into
     * @param [in] nonOput Ignore the first sample when searching the minimum display offset.
     */
    void fillCompositionToDecodeBox(CompositionToDecodeBox& compositionToDecodeBox, bool nonOutput);

    /**
     *  @pre Method loadOrder has been called
     *  @return True if Composition Offset Box is required */
    bool isCompositionOffsetBoxRequired() const;

    /**
     * @pre Method loadOrder has been called
     * @return True if Composition Decode Box is required */
    bool isCompositionToDecodeBoxRequired() const;

private:
    std::uint32_t mDisplayRate;
    std::uint32_t mClockTicks;  ///< Timescale from the Movie Header Box, number of time units passing in one second.
    std::uint32_t mDecodeStartTime;
    std::uint32_t mDisplayStartTime;
    bool mIsCompositionOffsetBoxRequired;
    bool mIsCompositionToDecodeBoxRequired;

    std::vector<std::int32_t>  mDisplayOffset;
    std::vector<std::uint32_t> mDecodeOrder;
    std::vector<std::uint32_t> mDisplayOrder;
    std::vector<std::uint32_t> mDecodeTime;
    std::vector<std::uint32_t> mDisplayTime;
};


#endif /* end of include guard: TIMEWRITER_HPP */

