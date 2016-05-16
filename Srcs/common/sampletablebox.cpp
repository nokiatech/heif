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

#include "sampletablebox.hpp"
#include "log.hpp"

using namespace std;

SampleTableBox::SampleTableBox() :
    Box("stbl"),
    mSampleDescriptionBox(),
    mTimeToSampleBox(),
    mSampleToChunkBox(),
    mChunkOffsetBox(),
    mSampleSizeBox(),
    mCompositionToDecodeBox(nullptr),
    mSampleGroupDescriptionBox(nullptr)
{
}

SampleDescriptionBox& SampleTableBox::getSampleDescriptionBox()
{
    return mSampleDescriptionBox;
}

TimeToSampleBox& SampleTableBox::getTimeToSampleBox()
{
    return mTimeToSampleBox;
}

void SampleTableBox::setCompositionOffsetBox(const CompositionOffsetBox& compositionOffsetBox)
{
    if (mCompositionOffsetBox == nullptr)
    {
        mCompositionOffsetBox = std::make_shared<CompositionOffsetBox>(compositionOffsetBox);
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
        mCompositionToDecodeBox = std::make_shared<CompositionToDecodeBox>(compositionToDecodeBox);
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

SampleToChunkBox& SampleTableBox::getSampleToChunkBox()
{
    return mSampleToChunkBox;
}


ChunkOffsetBox& SampleTableBox::getChunkOffsetBox()
{
    return mChunkOffsetBox;
}

SampleSizeBox& SampleTableBox::getSampleSizeBox()
{
    return mSampleSizeBox;
}

void SampleTableBox::setSampleGroupDescriptionBox(std::unique_ptr<SampleGroupDescriptionBox> sgpd)
{
    mSampleGroupDescriptionBox = std::move(sgpd);
}

SampleToGroupBox& SampleTableBox::getSampleToGroupBox()
{
    mSampleToGroupBox.resize(mSampleToGroupBox.size() + 1);
    return mSampleToGroupBox.back();
}


const std::vector<SampleToGroupBox>& SampleTableBox::getSampleToGroupBoxes() const
{
    return mSampleToGroupBox;
}

const SampleGroupDescriptionBox* SampleTableBox::getSampleGroupDescriptionBox(const std::string& groupingType) const
{
    if (mSampleGroupDescriptionBox)
    {
        if (mSampleGroupDescriptionBox->getGroupingType() == groupingType)
        {
            return mSampleGroupDescriptionBox.get();
        }
    }

    throw std::runtime_error("SampleGroupDescriptionBox with groupingType=" + groupingType + " not found.");
}

void SampleTableBox::setSyncSampleBox(const SyncSampleBox& syncSampleBox)
{
    if (mSyncSampleBox == nullptr)
    {
        mSyncSampleBox = std::make_shared<SyncSampleBox>(syncSampleBox);
    }
    else
    {
        *mSyncSampleBox = syncSampleBox;
    }
}

void SampleTableBox::writeBox(BitStream& bitstr)
{
    // Write box headers
    writeBoxHeader(bitstr);

    // Write other boxes contained in the movie box
    mSampleDescriptionBox.writeBox(bitstr);
    mTimeToSampleBox.writeBox(bitstr);
    mSampleToChunkBox.writeBox(bitstr);
    mChunkOffsetBox.writeBox(bitstr);
    mSampleSizeBox.writeBox(bitstr);

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

    if (mSampleGroupDescriptionBox)
    {
        mSampleGroupDescriptionBox->writeBox(bitstr);
    }

    if (mSampleToGroupBox.empty() == false)
    {
        for (auto sbgp : mSampleToGroupBox)
        {
            sbgp.writeBox(bitstr);
        }
    }

    // Update the size
    updateSize(bitstr);
}

void SampleTableBox::parseBox(BitStream& bitstr)
{
    //  First parse the box header
    parseBoxHeader(bitstr);

    // if there a data available in the file
    while (bitstr.numBytesLeft() > 0)
    {
        // Extract contained box bitstream and type
        std::string boxType;
        BitStream subBitstr = bitstr.readSubBoxBitStream(boxType);

        // Handle this box based on the type
        if (boxType == "stsd")
        {
            mSampleDescriptionBox.parseBox(subBitstr);
        }
        else if (boxType == "stco" || boxType == "co64") // 'co64' is the 64-bit version
        {
            mChunkOffsetBox.parseBox(subBitstr);
        }
        else if (boxType == "stsz")
        {
            mSampleSizeBox.parseBox(subBitstr);
        }
        else if (boxType == "stts")
        {
            mTimeToSampleBox.parseBox(subBitstr);
        }
        else if (boxType == "stsc")
        {
            mSampleToChunkBox.parseBox(subBitstr);
        }
        else if (boxType == "stss")
        {
            mSyncSampleBox = std::make_shared<SyncSampleBox>();
            mSyncSampleBox->parseBox(subBitstr);
        }
        else if (boxType == "sgpd")
        {
            auto sgdb = new SampleGroupDescriptionBox;
            sgdb->parseBox(subBitstr);
            mSampleGroupDescriptionBox.reset(sgdb);
        }
        else if (boxType == "sbgp")
        {
            SampleToGroupBox sampleToGroupBox;
            sampleToGroupBox.parseBox(subBitstr);
            mSampleToGroupBox.push_back(move(sampleToGroupBox));
        }
        else if (boxType == "cslg")
        {
            mCompositionToDecodeBox = std::make_shared<CompositionToDecodeBox>();
            mCompositionToDecodeBox->parseBox(subBitstr);
        }
        else if (boxType == "ctts")
        {
            mCompositionOffsetBox = std::make_shared<CompositionOffsetBox>();
            mCompositionOffsetBox->parseBox(subBitstr);
        }
        else
        {
            logWarning() << "Skipping unknown box of type '" << boxType << "' inside SampleTableBox" << endl;
        }
    }

    // We need to update stsc decoded presentation of chunk entries.
    mSampleToChunkBox.decodeEntries(mChunkOffsetBox.getChunkOffsets().size());
}
