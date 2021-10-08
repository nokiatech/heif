/* This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2021 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its
 * subsidiaries. All rights are reserved.
 *
 * Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior
 * written consent of Nokia.
 */

#include "segmentindexbox.hpp"

#include <cassert>
#include <iostream>
#include <stdexcept>

SegmentIndexBox::SegmentIndexBox(uint8_t version)
    : FullBox("sidx", version, 0)
    , mReferenceID(0)
    , mTimescale(0)
    , mEarliestPresentationTime(0)
    , mFirstOffset(0)
    , mReferences()
    , mReserveTotal(0)
{
}

void SegmentIndexBox::setSpaceReserve(size_t reserveTotal)
{
    mReserveTotal = reserveTotal;
}

void SegmentIndexBox::setReferenceId(const uint32_t referenceID)
{
    mReferenceID = referenceID;
}

uint32_t SegmentIndexBox::getReferenceId() const
{
    return mReferenceID;
}

void SegmentIndexBox::setTimescale(const uint32_t timescale)
{
    mTimescale = timescale;
}

uint32_t SegmentIndexBox::getTimescale() const
{
    return mTimescale;
}

void SegmentIndexBox::setEarliestPresentationTime(const uint64_t earliestPresentationTime)
{
    mEarliestPresentationTime = earliestPresentationTime;
}

uint64_t SegmentIndexBox::getEarliestPresentationTime() const
{
    return mEarliestPresentationTime;
}

void SegmentIndexBox::setFirstOffset(const uint64_t firstOffset)
{
    mFirstOffset = firstOffset;
}

uint64_t SegmentIndexBox::getFirstOffset() const
{
    return mFirstOffset;
}

void SegmentIndexBox::addReference(const SegmentIndexBox::Reference& reference)
{
    mReferences.push_back(reference);
    assert(mReserveTotal == 0 || mReferences.size() < mReserveTotal);
}

Vector<SegmentIndexBox::Reference> SegmentIndexBox::getReferences() const
{
    return mReferences;
}

void SegmentIndexBox::writeBox(BitStream& bitstr) const
{
    const uint32_t referenceSize = 3 * 4;
    const auto reserveBytes      = static_cast<uint32_t>((mReserveTotal - mReferences.size()) * referenceSize);

    writeFullBoxHeader(bitstr);
    bitstr.write32Bits(mReferenceID);
    bitstr.write32Bits(mTimescale);
    if (getVersion() == 0)
    {
        bitstr.write32Bits(static_cast<uint32_t>(mEarliestPresentationTime));
        bitstr.write32Bits(static_cast<uint32_t>(mFirstOffset + reserveBytes));
    }
    else if (getVersion() == 1)
    {
        bitstr.write64Bits(mEarliestPresentationTime);
        bitstr.write64Bits(mFirstOffset + reserveBytes);
    }
    else
    {
        throw RuntimeError("SegmentIndexBox::writeBox() supports only 'sidx' version 0 or 1");
    }
    bitstr.write16Bits(0);                                          // reserved = 0
    bitstr.write16Bits(static_cast<uint16_t>(mReferences.size()));  // reference_count

    for (const auto& reference : mReferences)
    {
        bitstr.writeBits(reference.referenceType ? uint64_t(1) : uint64_t(0), 1);  // bit (1) reference_type
        bitstr.writeBits(uint64_t(0) | reference.referencedSize, 31);              // unsigned int(31) referenced_size
        bitstr.write32Bits(reference.subsegmentDuration);  // unsigned int(32) subsegment_duration
        bitstr.writeBits(reference.startsWithSAP ? uint64_t(1) : uint64_t(0), 1);  // bit (1) starts_with_SAP
        bitstr.writeBits(uint64_t(0) | reference.sapType, 3);                      // unsigned int(3) SAP_type
        bitstr.writeBits(uint64_t(0) | reference.sapDeltaTime, 28);                // unsigned int(28) SAP_delta_time
    }

    updateSize(bitstr);

    if (mReserveTotal != 0)
    {
        bitstr.write32Bits(reserveBytes);
        bitstr.write32Bits(FourCCInt("free").getUInt32());
        bitstr.write32Bits(0);

        for (uint32_t i = 1; i < mReserveTotal - mReferences.size(); i++)
        {
            bitstr.write32Bits(0);
            bitstr.write32Bits(0);
            bitstr.write32Bits(0);
        }
    }
}

void SegmentIndexBox::parseBox(BitStream& bitstr)
{
    parseFullBoxHeader(bitstr);
    mReferenceID = bitstr.read32Bits();
    mTimescale   = bitstr.read32Bits();
    if (getVersion() == 0)
    {
        mEarliestPresentationTime = bitstr.read32Bits();
        mFirstOffset              = bitstr.read32Bits();
    }
    else if (getVersion() == 1)
    {
        mEarliestPresentationTime = bitstr.read64Bits();
        mFirstOffset              = bitstr.read64Bits();
    }
    else
    {
        throw RuntimeError("SegmentIndexBox::parseBox() supports only 'sidx' version 0 or 1");
    }

    bitstr.read16Bits();                            // reserved = 0
    uint16_t referenceCount = bitstr.read16Bits();  // reference_count
    mReferences.clear();
    mReferences.reserve(referenceCount);
    for (uint16_t i = 0; i < referenceCount; i++)
    {
        Reference ref;
        ref.referenceType      = (bitstr.readBits(1) != 0);                 // bit (1) reference_type
        ref.referencedSize     = bitstr.readBits(31);                       // unsigned int(31) referenced_size
        ref.subsegmentDuration = bitstr.read32Bits();                       // unsigned int(32) subsegment_duration
        ref.startsWithSAP      = (bitstr.readBits(1) != 0);                 // bit (1) starts_with_SAP
        ref.sapType            = static_cast<uint8_t>(bitstr.readBits(3));  // unsigned int(3) SAP_type
        ref.sapDeltaTime       = bitstr.readBits(28);                       // unsigned int(28) SAP_delta_time
        mReferences.push_back(ref);
    }
}
