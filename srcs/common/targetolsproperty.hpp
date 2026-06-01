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

#ifndef TARGETOLSPROPERTY_HPP
#define TARGETOLSPROPERTY_HPP

#include <cstdint>

#include "bbox.hpp"

namespace ISOBMFF
{
    class BitStream;
}

/** Target Output Layer Set item property `'tols'` (ISO/IEC 23008-12 Section 6.5.x).
 *  Identifies the target Output Layer Set within an `'oinf'` operating-points
 *  table for a multi-layer (e.g. L-HEVC) coded image item. The box is a plain
 *  Box (not a FullBox) and carries a single 16-bit `target_ols_index` payload
 *  selecting one entry from the `oinf` OLS list. Mandatory exactly once on
 *  every `lhv1` item per the specification. */
class TargetOlsProperty : public Box
{
public:
    TargetOlsProperty();
    ~TargetOlsProperty() override = default;

    /** Set the target Output Layer Set index. */
    void setOlsIndex(std::uint16_t index);

    /** @return Target OLS index of the selected operating point. */
    std::uint16_t getOlsIndex() const;

    /** Write box/property to ISOBMFF::BitStream.
     *  @see Box::writeBox() */
    void writeBox(ISOBMFF::BitStream& output) const override;

    /** Parse box/property from ISOBMFF::BitStream.
     *  @see Box::parseBox() */
    void parseBox(ISOBMFF::BitStream& input) override;

private:
    std::uint16_t mOlsIdx;  ///< Target OLS index identifying the selected operating point.
};

#endif /* TARGETOLSPROPERTY_HPP */
