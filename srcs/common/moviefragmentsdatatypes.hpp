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

#ifndef MOVIEFRAGMENTSDATATYPES_HPP_
#define MOVIEFRAGMENTSDATATYPES_HPP_

#include <cstdint>
#include "bitstream.hpp"
#include "customallocator.hpp"

namespace MOVIEFRAGMENTS
{
    // Sample Flags Field as defined in 8.8.3.1 of ISO/IEC 14496-12:2015(E)
    struct SampleFlagsType
    {
        uint32_t reserved : 4,
            is_leading : 2,
            sample_depends_on : 2,
            sample_is_depended_on : 2,
            sample_has_redundancy : 2,
            sample_padding_value : 3,
            sample_is_non_sync_sample : 1,
            sample_degradation_priority : 16;
    };

    union SampleFlags {
        uint32_t flagsAsUInt;
        SampleFlagsType flags;

        static SampleFlags read(ISOBMFF::BitStream& bitstr)
        {
            SampleFlags r;
            r.flags.reserved                    = bitstr.readBits(4);
            r.flags.is_leading                  = bitstr.readBits(2);
            r.flags.sample_depends_on           = bitstr.readBits(2);
            r.flags.sample_is_depended_on       = bitstr.readBits(2);
            r.flags.sample_has_redundancy       = bitstr.readBits(2);
            r.flags.sample_padding_value        = bitstr.readBits(3);
            r.flags.sample_is_non_sync_sample   = bitstr.readBits(1);
            r.flags.sample_degradation_priority = bitstr.readBits(16);
            return r;
        }

        static void write(ISOBMFF::BitStream& bitstr, const SampleFlags& r)
        {
            bitstr.writeBits(r.flags.reserved, 4);
            bitstr.writeBits(r.flags.is_leading, 2);
            bitstr.writeBits(r.flags.sample_depends_on, 2);
            bitstr.writeBits(r.flags.sample_is_depended_on, 2);
            bitstr.writeBits(r.flags.sample_has_redundancy, 2);
            bitstr.writeBits(r.flags.sample_padding_value, 3);
            bitstr.writeBits(r.flags.sample_is_non_sync_sample, 1);
            bitstr.writeBits(r.flags.sample_degradation_priority, 16);
        }
    };

    struct SampleDefaults
    {
        std::uint32_t trackId;
        std::uint32_t defaultSampleDescriptionIndex;
        std::uint32_t defaultSampleDuration;
        std::uint32_t defaultSampleSize;
        MOVIEFRAGMENTS::SampleFlags defaultSampleFlags;
    };
}  // namespace MOVIEFRAGMENTS
#endif /* MOVIEFRAGMENTSDATATYPES_HPP_ */
