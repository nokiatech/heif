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

#include "vvcmixednalunittypepicentry.hpp"

#include "bitstream.hpp"
#include <vector>

VvcMixedNalUnitTypePicEntry::VvcMixedNalUnitTypePicEntry()
    : SampleGroupDescriptionEntry("minp")
     
{
}


std::uint32_t VvcMixedNalUnitTypePicEntry::getSize() const
{
    BitStream tmp;
    writeEntry(tmp);
    return static_cast<uint32_t>(tmp.getSize());
}


void VvcMixedNalUnitTypePicEntry::writeEntry(ISOBMFF::BitStream& bitstr) const
{
    bitstr.writeBits(mMixSubpTrackIdxs.size(), 16);
    for (const auto& ids : mMixSubpTrackIdxs)
    {
        bitstr.writeBits(ids.first, 32);
        bitstr.writeBits(ids.second, 32);
    }
    bitstr.writeBits(mPpsMixNaluTypesInPicBitPos, 10);
    bitstr.writeBits(mPpsId, 6);
}

void VvcMixedNalUnitTypePicEntry::parseEntry(ISOBMFF::BitStream& bitstr)
{
    const auto numMixNaluPicIdx = bitstr.read16Bits();
    mMixSubpTrackIdxs.reserve(numMixNaluPicIdx);
    for (auto i = 0; i < numMixNaluPicIdx; ++i)
    {
        // NB: read32Bits() has side effects (advances the bitstream cursor).
        // C++17 still leaves function-argument evaluation order unspecified,
        // so we must read into named locals to guarantee first-then-second
        // ordering. The previous push_back(make_pair(read, read)) form
        // happened to evaluate left-to-right, but emplace_back(read, read)
        // can evaluate right-to-left and silently swap the pair members.
        const auto first  = bitstr.read32Bits();
        const auto second = bitstr.read32Bits();
        mMixSubpTrackIdxs.emplace_back(first, second);
    }
    mPpsMixNaluTypesInPicBitPos = static_cast<uint16_t>(bitstr.readBits(10));
    mPpsId                      = static_cast<uint8_t>(bitstr.readBits(6));
}

std::uint16_t VvcMixedNalUnitTypePicEntry::getPpsMixNaluTypesInPicBitPos() const
{
    return mPpsMixNaluTypesInPicBitPos;
}

void VvcMixedNalUnitTypePicEntry::setPpsMixNaluTypesInPicBitPos(const std::uint16_t ppsMixNaluTypesInPicBitPos)
{
    mPpsMixNaluTypesInPicBitPos = ppsMixNaluTypesInPicBitPos;
}

std::uint8_t VvcMixedNalUnitTypePicEntry::getPpsId() const
{
    return mPpsId;
}

void VvcMixedNalUnitTypePicEntry::setPpsId(std::uint8_t ppsId)
{
    mPpsId = ppsId;
}

Vector<VvcMixedNalUnitTypePicEntry::MixSubpTrackIdxs> VvcMixedNalUnitTypePicEntry::getMixSubpTrackIds() const
{
    return mMixSubpTrackIdxs;
}

void VvcMixedNalUnitTypePicEntry::setMixSubpTrackIds(const Vector<MixSubpTrackIdxs>& mixSubpTrackIds)
{
    mMixSubpTrackIdxs = mixSubpTrackIds;
}
