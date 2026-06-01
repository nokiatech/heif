/* This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2026 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its
 * subsidiaries. All rights are reserved.
 *
 * Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior
 * written consent of Nokia.
 */

#ifndef COMPOSITIONTODECODEBOX_HPP
#define COMPOSITIONTODECODEBOX_HPP

#include "fullbox.hpp"
#include <cstdint>

/** @brief Composition To Decode Box class
 *  @details 'cslg' box implementation. Extends from FullBox. Only version 0 is supported.
 */
class CompositionToDecodeBox : public FullBox
{
public:
    CompositionToDecodeBox();
    ~CompositionToDecodeBox() override = default;

    /// @param [in] compositionToDtsShift Composition to Dts shift value as specified in the ISOBMFF standard
    void setCompositionToDtsShift(std::int64_t compositionToDtsShift);

    /// @return Composition to Dts shift value
    std::int64_t getCompositionToDtsShift() const;

    /// @param [in] leastDecodeToDisplayDelta Least Decode to Display Time Delta value as specified in the ISOBMFF
    /// standard
    void setLeastDecodeToDisplayDelta(std::int64_t leastDecodeToDisplayDelta);

    /// @return Least Decode to Display Time Delta value
    std::int64_t getLeastDecodeToDisplayDelta() const;

    /// @param [in] greatestDecodeToDisplayDelta Greatest Decode to Display Time Delta value as specified in the ISOBMFF
    /// standard
    void setGreatestDecodeToDisplayDelta(std::int64_t greatestDecodeToDisplayDelta);

    /// @return Greatest Decode to Display Time Delta value
    std::int64_t getGreatestDecodeToDisplayDelta() const;

    /// @param [in] compositionStartTime Composition start time as specified in the ISOBMFF standard
    void setCompositionStartTime(std::int64_t compositionStartTime);

    /// @return Composition start time value
    std::int64_t getCompositionStartTime() const;

    /// @param [in] compositionEndTime Composition end time as specified in the ISOBMFF standard
    void setCompositionEndTime(std::int64_t compositionEndTime);

    /// @return Composition end time value
    std::int64_t getCompositionEndTime() const;

    /** @brief Creates the bitstream that represents the box in the ISOBMFF file
     *  @param [out] bitstr Bitstream that contains the box data. */
    void writeBox(ISOBMFF::BitStream& bitstr) const override;

    /** @brief Parses a Composition To Decode Box bitstream and fills in the necessary member variables
     *  @param [in]  bitstr Bitstream that contains the box data */
    void parseBox(ISOBMFF::BitStream& bitstr) override;

private:
    /// @brief Update box version depending on required value range.
    void updateVersion();

    /// @brief Composition to Dts shift value as specified in the ISOBMFF standard
    std::int64_t mCompositionToDtsShift{0};

    /// @brief Least Decode to Display Time Delta value as specified in the ISOBMFF standard
    std::int64_t mLeastDecodeToDisplayDelta{0};

    /// @brief Greatest Decode to Display Time Delta value as specified in the ISOBMFF standard
    std::int64_t mGreatestDecodeToDisplayDelta{0};

    /// @brief Composition start time value as specified in the ISOBMFF standard
    std::int64_t mCompositionStartTime{0};

    /// @brief Composition end time value as specified in the ISOBMFF standard
    std::int64_t mCompositionEndTime{0};
};

#endif /* end of include guard: COMPOSITIONTODECODEBOX_HPP */
