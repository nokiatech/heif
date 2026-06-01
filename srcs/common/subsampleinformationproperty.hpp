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

#ifndef SUBSAMPLEINFORMATIONPROPERTY_HPP
#define SUBSAMPLEINFORMATIONPROPERTY_HPP

#include <cstdint>

#include "fullbox.hpp"
#include "customallocator.hpp"
#include <cstddef>

namespace ISOBMFF
{
    class BitStream;
}

/** Sub-sample information item property `'subs'` (ISO/IEC 14496-12 Section 8.7.7).
 *  Describes byte-range sub-divisions within a coded image item's data,
 *  mapping each sub-sample to a priority and codec-specific role.
 *  Zero or more instances may be associated with an HEVC or AVC image item.
 *
 *  When used as an item property the box always carries exactly one entry
 *  (sample_delta == 1); the sub-sample array within that entry is what
 *  the accessors below expose. */
class SubSampleInformationProperty : public FullBox
{
public:
    SubSampleInformationProperty();
    ~SubSampleInformationProperty() override = default;

    struct SubSampleEntry
    {
        uint32_t subsampleSize;             ///< Byte count of this sub-sample.
        uint8_t  subsamplePriority;         ///< Lower values are more important.
        uint8_t  discardable;               ///< Non-zero if this sub-sample may be discarded.
        uint32_t codecSpecificParameters;   ///< Codec-defined semantics.
    };

    /** Replace the entire sub-sample list. */
    void setSubSamples(const Vector<SubSampleEntry>& entries);

    /** @return Current sub-sample list. */
    const Vector<SubSampleEntry>& getSubSamples() const;

    /** @return Number of sub-samples. */
    std::size_t getSubSampleCount() const;

    /** Write box/property to ISOBMFF::BitStream.
     *  @see Box::writeBox() */
    void writeBox(ISOBMFF::BitStream& output) const override;

    /** Parse box/property from ISOBMFF::BitStream.
     *  @see Box::parseBox() */
    void parseBox(ISOBMFF::BitStream& input) override;

private:
    Vector<SubSampleEntry> mSubSamples;
};

#endif  // SUBSAMPLEINFORMATIONPROPERTY_HPP
