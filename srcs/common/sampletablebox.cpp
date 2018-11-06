/* This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2018 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its
 * subsidiaries. All rights are reserved.
 *
 * Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior
 * written consent of Nokia.
 */

#include "sampletablebox.hpp"
#include "log.hpp"
#include "smallvector.hpp"

using namespace std;

SampleTableBox::SampleTableBox()
    : Box("stbl")
    , mSampleDescriptionBox()
    , mTimeToSampleBox()
    , mSampleToChunkBox()
    , mChunkOffsetBox()
    , mSampleSizeBox()
    , mCompositionToDecodeBox(nullptr)
    , mSampleGroupDescriptionBoxes()
    , mSampleToGroupBoxes()
    , mHasSyncSampleBox(false)
{
}

const SampleDescriptionBox& SampleTableBox::getSampleDescriptionBox() const
{
    return mSampleDescriptionBox;
}

SampleDescriptionBox& SampleTableBox::getSampleDescriptionBox()
{
    return mSampleDescriptionBox;
}

const TimeToSampleBox& SampleTableBox::getTimeToSampleBox() const
{
    return mTimeToSampleBox;
}

TimeToSampleBox& SampleTableBox::getTimeToSampleBox()
{
    return mTimeToSampleBox;
}

void SampleTableBox::setCompositionOffsetBox(const CompositionOffsetBox& compositionOffsetBox)
{
    if (mCompositionOffsetBox == nullptr)
    {
        mCompositionOffsetBox = makeCustomShared<CompositionOffsetBox>(compositionOffsetBox);
    }
    else
    {
        *mCompositionOffsetBox = compositionOffsetBox;
    }
}

std::shared_ptr<const CompositionOffsetBox> SampleTableBox::getCompositionOffsetBox() const
{
    return mCompositionOffsetBox;
}

void SampleTableBox::setCompositionToDecodeBox(const CompositionToDecodeBox& compositionToDecodeBox)
{
    if (mCompositionToDecodeBox == nullptr)
    {
        mCompositionToDecodeBox = makeCustomShared<CompositionToDecodeBox>(compositionToDecodeBox);
    }
    else
    {
        *mCompositionToDecodeBox = compositionToDecodeBox;
    }
}

std::shared_ptr<const CompositionToDecodeBox> SampleTableBox::getCompositionToDecodeBox() const
{
    return mCompositionToDecodeBox;
}

const SampleToChunkBox& SampleTableBox::getSampleToChunkBox() const
{
    return mSampleToChunkBox;
}

SampleToChunkBox& SampleTableBox::getSampleToChunkBox()
{
    return mSampleToChunkBox;
}

const ChunkOffsetBox& SampleTableBox::getChunkOffsetBox() const
{
    return mChunkOffsetBox;
}

ChunkOffsetBox& SampleTableBox::getChunkOffsetBox()
{
    return mChunkOffsetBox;
}

const SampleSizeBox& SampleTableBox::getSampleSizeBox() const
{
    return mSampleSizeBox;
}

SampleSizeBox& SampleTableBox::getSampleSizeBox()
{
    return mSampleSizeBox;
}

void SampleTableBox::addSampleGroupDescriptionBox(UniquePtr<SampleGroupDescriptionBox> sgpd)
{
    mSampleGroupDescriptionBoxes.push_back(std::move(sgpd));
}

SampleToGroupBox& SampleTableBox::getSampleToGroupBox()
{
    mSampleToGroupBoxes.resize(mSampleToGroupBoxes.size() + 1);
    return mSampleToGroupBoxes.back();
}

const Vector<SampleToGroupBox>& SampleTableBox::getSampleToGroupBoxes() const
{
    return mSampleToGroupBoxes;
}

const SampleGroupDescriptionBox* SampleTableBox::getSampleGroupDescriptionBox(FourCCInt groupingType) const
{
    if (mSampleGroupDescriptionBoxes.size())
    {
        for (const auto& sgpd : mSampleGroupDescriptionBoxes)
        {
            if (sgpd->getGroupingType() == groupingType)
            {
                return sgpd.get();
            }
        }
    }

    return nullptr;
}

void SampleTableBox::setSyncSampleBox(const SyncSampleBox& syncSampleBox)
{
    if (mSyncSampleBox == nullptr)
    {
        mSyncSampleBox = makeCustomShared<SyncSampleBox>(syncSampleBox);
    }
    else
    {
        *mSyncSampleBox = syncSampleBox;
    }
    mHasSyncSampleBox = true;
}

bool SampleTableBox::hasSyncSampleBox() const
{
    return mHasSyncSampleBox;
}

std::shared_ptr<const SyncSampleBox> SampleTableBox::getSyncSampleBox() const
{
    return mSyncSampleBox;
}

void SampleTableBox::writeBox(ISOBMFF::BitStream& bitstr) const
{
    // Write box headers
    writeBoxHeader(bitstr);

    // Write other boxes contained in the movie box
    mSampleDescriptionBox.writeBox(bitstr);
    mSampleSizeBox.writeBox(bitstr);
    mTimeToSampleBox.writeBox(bitstr);
    mSampleToChunkBox.writeBox(bitstr);
    mChunkOffsetBox.writeBox(bitstr);

    if (mSyncSampleBox != nullptr)
    {
        mSyncSampleBox->writeBox(bitstr);
    }

    if (mCompositionOffsetBox != nullptr)
    {
        mCompositionOffsetBox->writeBox(bitstr);
    }

    if (mCompositionToDecodeBox != nullptr)
    {
        mCompositionToDecodeBox->writeBox(bitstr);
    }

    if (mSampleGroupDescriptionBoxes.size())
    {
        for (auto& sgpd : mSampleGroupDescriptionBoxes)
        {
            sgpd->writeBox(bitstr);
        }
    }

    if (mSampleToGroupBoxes.size())
    {
        for (auto& sbgp : mSampleToGroupBoxes)
        {
            sbgp.writeBox(bitstr);
        }
    }

    // Update the size
    updateSize(bitstr);
}

void SampleTableBox::parseBox(ISOBMFF::BitStream& bitstr)
{
    //  First parse the box header
    parseBoxHeader(bitstr);

    int64_t sampleCountMax = -1;
    int64_t absoluteSampleCountMax =
        MP4VR_ABSOLUTE_MAX_SAMPLE_COUNT;  // 4 194 304  (more than day worth of 48hz samples)

    // if there a data available in the file
    while (bitstr.numBytesLeft() > 0)
    {
        // Extract contained box bitstream and type
        FourCCInt boxType;
        BitStream subBitstr = bitstr.readSubBoxBitStream(boxType);

        // Handle this box based on the type
        if (boxType == "stsd")
        {
            mSampleDescriptionBox.parseBox(subBitstr);
        }
        else if (boxType == "stco" || boxType == "co64")  // 'co64' is the 64-bit version
        {
            mChunkOffsetBox.parseBox(subBitstr);
        }
        else if (boxType == "stsz")
        {
            mSampleSizeBox.parseBox(subBitstr);
            uint32_t sampleCount = mSampleSizeBox.getSampleCount();
            if (sampleCountMax == -1)
            {
                if (sampleCount > absoluteSampleCountMax)
                {
                    throw RuntimeError("Over max sample counts from stsz to rest of sample table");
                }
                sampleCountMax = static_cast<int64_t>(sampleCount);
            }
            else if (sampleCount != sampleCountMax)
            {
                throw RuntimeError("Non-matching sample counts from stsz to rest of sample table");
            }
        }
        else if (boxType == "stts")
        {
            mTimeToSampleBox.parseBox(subBitstr);
            uint32_t sampleCount = static_cast<uint32_t>(mTimeToSampleBox.getSampleCount());
            if (sampleCountMax == -1)
            {
                if (sampleCount > absoluteSampleCountMax)
                {
                    throw RuntimeError("Over max sample counts from stts to rest of sample table");
                }
                sampleCountMax = static_cast<int64_t>(sampleCount);
            }
            else if (sampleCount != sampleCountMax)
            {
                throw RuntimeError("Non-matching sample counts from stts to rest of sample table");
            }
        }
        else if (boxType == "stsc")
        {
            if (sampleCountMax != -1)
            {
                mSampleToChunkBox.setSampleCountMaxSafety(sampleCountMax);
            }
            mSampleToChunkBox.parseBox(subBitstr);
        }
        else if (boxType == "stss")
        {
            mSyncSampleBox = makeCustomShared<SyncSampleBox>();
            if (sampleCountMax != -1)
            {
                mSyncSampleBox->setSampleCountMaxSafety(sampleCountMax);
            }
            mSyncSampleBox->parseBox(subBitstr);
            mHasSyncSampleBox = true;
        }
        else if (boxType == "sgpd")
        {
            UniquePtr<SampleGroupDescriptionBox> sgpd(CUSTOM_NEW(SampleGroupDescriptionBox, ()));
            sgpd->parseBox(subBitstr);
            mSampleGroupDescriptionBoxes.push_back(move(sgpd));
        }
        else if (boxType == "sbgp")
        {
            SampleToGroupBox sampleToGroupBox;
            sampleToGroupBox.parseBox(subBitstr);
            uint32_t sampleCount = static_cast<uint32_t>(sampleToGroupBox.getNumberOfSamples());
            if (sampleCountMax == -1)
            {
                if (sampleCount > absoluteSampleCountMax)
                {
                    throw RuntimeError("Over max sample counts from sbgp to rest of sample table");
                }
                // we can't update sampleCountMax here as sbgp can have less samples than total.
            }
            else if (sampleCount > sampleCountMax)
            {  // special case, there can be less samples defined in this box, but not more
                throw RuntimeError("Non-matching sample counts from sbgp to rest of sample table");
            }
            mSampleToGroupBoxes.push_back(move(sampleToGroupBox));
        }
        else if (boxType == "cslg")
        {
            mCompositionToDecodeBox = makeCustomShared<CompositionToDecodeBox>();
            mCompositionToDecodeBox->parseBox(subBitstr);
        }
        else if (boxType == "ctts")
        {
            mCompositionOffsetBox = makeCustomShared<CompositionOffsetBox>();
            mCompositionOffsetBox->parseBox(subBitstr);
            uint32_t sampleCount = static_cast<uint32_t>(mCompositionOffsetBox->getSampleCount());
            if (sampleCountMax == -1)
            {
                if (sampleCount > absoluteSampleCountMax)
                {
                    throw RuntimeError("Over max sample counts from ctts to rest of sample table");
                }
                sampleCountMax = static_cast<int64_t>(sampleCount);
            }
            else if (sampleCount != sampleCountMax)
            {
                throw RuntimeError("Non-matching sample counts from ctts to rest of sample table");
            }
        }
        else
        {
            logWarning() << "Skipping unknown box of type '" << boxType.getString() << "' inside SampleTableBox" << endl;
        }
    }

    if (sampleCountMax == -1)
    {
        throw RuntimeError("SampleToTableBox does not determine number of samples");
    }
    else
    {
        SmallVector<uint32_t, 2> sizes;
        sizes.push_back(mTimeToSampleBox.getSampleCount());
        sizes.push_back(mSampleSizeBox.getSampleCount());

        auto lowerBound = mSampleToChunkBox.getSampleCountLowerBound(
            static_cast<unsigned int>(mChunkOffsetBox.getChunkOffsets().size()));
        auto referenceSize = sizes[0];
        for (size_t c = 0; c < sizes.size(); ++c)
        {
            if (sizes[c] != referenceSize || sizes[c] < lowerBound)
            {
                throw RuntimeError("SampleToTableBox contains boxes with mismatching sample counts");
            }
        }

        // reset it here in case the order of boxes didn't allow it to
        // be set on time for mSampleToGroupBox parsing
        mSampleToChunkBox.setSampleCountMaxSafety(sampleCountMax);

        // we need to update stsc decoded presentation of chunk entries.
        mSampleToChunkBox.decodeEntries(static_cast<unsigned int>(mChunkOffsetBox.getChunkOffsets().size()));
    }
}

void SampleTableBox::resetSamples()
{
    mTimeToSampleBox  = TimeToSampleBox();
    mSampleToChunkBox = SampleToChunkBox();
    mChunkOffsetBox   = ChunkOffsetBox();
    mSampleSizeBox    = SampleSizeBox();
    mSyncSampleBox.reset();
    mCompositionOffsetBox.reset();
    mCompositionToDecodeBox.reset();
    mSampleGroupDescriptionBoxes.clear();
    mSampleToGroupBoxes.clear();
}
