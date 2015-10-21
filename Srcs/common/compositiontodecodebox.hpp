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

#ifndef COMPOSITIONTODECODEBOX_HPP
#define COMPOSITIONTODECODEBOX_HPP

#include "bitstream.hpp"
#include "fullbox.hpp"

/** @brief Composition To Decode Box class
 *  @details 'cslg' box implementation. Extends from FullBox. Only version 0 is supported.
 */
class CompositionToDecodeBox : public FullBox
{
public:
    CompositionToDecodeBox();
    virtual ~CompositionToDecodeBox() = default;

    /// @param [in] compositionTosDtsShift Composition to Dts shift value as specified in the ISOBMFF standard
    void setCompositionToDtsShift(std::int32_t compositionTosDtsShift);

    /// @return Composition to Dts shift value
    std::int32_t getCompositionToDtsShift() const;

    /// @param [in] leastDecodeToDisplayDelta Least Decode to Display Time Delta value as specified in the ISOBMFF standard
    void setLeastDecodeToDisplayDelta(std::int32_t leastDecodeToDisplayDelta);

    /// @return Least Decode to Display Time Delta value
    std::int32_t getLeastDecodeToDisplayDelta() const;

    /// @param [in] greatestDecodeToDisplayDelta Greatest Decode to Display Time Delta value as specified in the ISOBMFF standard
    void setGreatestDecodeToDisplayDelta(std::int32_t greatestDecodeToDisplayDelta);

    /// @return Greatest Decode to Display Time Delta value
    std::int32_t getGreatestDecodeToDisplayDelta() const;

    /// @param [in] compositionStartTime Composition start time as specified in the ISOBMFF standard
    void setCompositionStartTime(std::int32_t compositionStartTime);

    /// @return Composition start time value
    std::int32_t getCompositionStartTime() const;

    /// @param [in] compositionEndTime Composition end time as specified in the ISOBMFF standard
    void setCompositionEndTime(std::int32_t compositionEndTime);

    /// @return Composition end time value
    std::int32_t getCompositionEndTime() const;


    /** @brief Creates the bitstream that represents the box in the ISOBMFF file
     *  @param [out] bitstr Bitstream that contains the box data. */
    void writeBox(BitStream& bitstr);

    /** @brief Parses a Composition To Decode Box bitstream and fills in the necessary member variables
     *  @param [in]  bitstr Bitstream that contains the box data */
    void parseBox(BitStream& bitstr);

private:
    ///@brief Composition to Dts shift value as specified in the ISOBMFF standard
    std::int32_t mCompositionToDtsShift;

    ///@brief Least Decode to Display Time Delta value as specified in the ISOBMFF standard
    std::int32_t mLeastDecodeToDisplayDelta;

    ///@brief Greatest Decode to Display Time Delta value as specified in the ISOBMFF standard
    std::int32_t mGreatestDecodeToDisplayDelta;

    ///@brief Composition start time value as specified in the ISOBMFF standard
    std::int32_t mCompositionStartTime;

    ///@brief Composition end time value as specified in the ISOBMFF standard
    std::int32_t mCompositionEndTime;
};

#endif /* end of include guard: COMPOSITIONTODECODEBOX_HPP */

