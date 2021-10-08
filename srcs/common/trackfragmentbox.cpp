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

#include "trackfragmentbox.hpp"

#include "log.hpp"

TrackFragmentBox::TrackFragmentBox(Vector<MOVIEFRAGMENTS::SampleDefaults>& sampleDefaults)
    : Box("traf")
    , mTrackFragmentHeaderBox()
    , mSampleDefaults(sampleDefaults)
    , mTrackRunBoxes()
    , mTrackFragmentDecodeTimeBox()
{
}

TrackFragmentHeaderBox& TrackFragmentBox::getTrackFragmentHeaderBox()
{
    return mTrackFragmentHeaderBox;
}

void TrackFragmentBox::addTrackRunBox(UniquePtr<TrackRunBox> trackRunBox)
{
    mTrackRunBoxes.push_back(std::move(trackRunBox));
}

Vector<TrackRunBox*> TrackFragmentBox::getTrackRunBoxes()
{
    Vector<TrackRunBox*> trackRunBoxes;
    for (auto& trackRuns : mTrackRunBoxes)
    {
        trackRunBoxes.push_back(trackRuns.get());
    }
    return trackRunBoxes;
}

void TrackFragmentBox::setTrackFragmentDecodeTimeBox(
    UniquePtr<TrackFragmentBaseMediaDecodeTimeBox> trackFragmentDecodeTimeBox)
{
    mTrackFragmentDecodeTimeBox = std::move(trackFragmentDecodeTimeBox);
}

TrackFragmentBaseMediaDecodeTimeBox* TrackFragmentBox::getTrackFragmentBaseMediaDecodeTimeBox()
{
    return mTrackFragmentDecodeTimeBox.get();
}

void TrackFragmentBox::writeBox(BitStream& bitstr) const
{
    writeBoxHeader(bitstr);
    mTrackFragmentHeaderBox.writeBox(bitstr);

    if (mTrackFragmentDecodeTimeBox)
    {
        mTrackFragmentDecodeTimeBox->writeBox(bitstr);
    }
    for (auto& trackRuns : mTrackRunBoxes)
    {
        trackRuns->writeBox(bitstr);
    }
    updateSize(bitstr);
}

void TrackFragmentBox::parseBox(BitStream& bitstr)
{
    parseBoxHeader(bitstr);

    bool foundTfhd = false;

    while (bitstr.numBytesLeft() > 0)
    {
        FourCCInt boxType;
        BitStream subBitstr = bitstr.readSubBoxBitStream(boxType);

        if (boxType == "tfhd")
        {
            if (foundTfhd)
            {
                throw RuntimeError("TrackFragmentBox: exactly one tfhd expected; found more than one");
            }

            mTrackFragmentHeaderBox.parseBox(subBitstr);

            bool foundDefault                      = false;
            uint32_t defaultSampleDescriptionIndex = 0;
            for (const auto& sampleDefault : mSampleDefaults)
            {
                if (sampleDefault.trackId == mTrackFragmentHeaderBox.getTrackId())
                {
                    defaultSampleDescriptionIndex = sampleDefault.defaultSampleDescriptionIndex;
                    foundDefault                  = true;
                    break;
                }
            }
            if (!foundDefault)
            {
                throw RuntimeError("TrackFragmentBox: default sample description index not found");
            }

            if ((mTrackFragmentHeaderBox.getFlags() & TrackFragmentHeaderBox::SampleDescriptionIndexPresent) == 0)
            {
                mTrackFragmentHeaderBox.setSampleDescriptionIndex(defaultSampleDescriptionIndex);
            }
            foundTfhd = true;
        }
        else if (boxType == "tfdt")
        {
            UniquePtr<TrackFragmentBaseMediaDecodeTimeBox> trackFragmentDecodeTimeBox(
                CUSTOM_NEW(TrackFragmentBaseMediaDecodeTimeBox, ()));
            trackFragmentDecodeTimeBox->parseBox(subBitstr);
            mTrackFragmentDecodeTimeBox = std::move(trackFragmentDecodeTimeBox);
        }
        else if (boxType == "trun")
        {
            MOVIEFRAGMENTS::SampleDefaults sampleDefaults{};
            bool defaultsFound = false;
            for (const auto& sampleDefault : mSampleDefaults)
            {
                if (sampleDefault.trackId == mTrackFragmentHeaderBox.getTrackId())
                {
                    sampleDefaults.trackId                       = sampleDefault.trackId;
                    sampleDefaults.defaultSampleDescriptionIndex = sampleDefault.defaultSampleDescriptionIndex;
                    sampleDefaults.defaultSampleDuration         = sampleDefault.defaultSampleDuration;
                    sampleDefaults.defaultSampleSize             = sampleDefault.defaultSampleSize;
                    sampleDefaults.defaultSampleFlags            = sampleDefault.defaultSampleFlags;
                    defaultsFound                                = true;
                    break;
                }
            }
            if ((mTrackFragmentHeaderBox.getFlags() & TrackFragmentHeaderBox::DefaultSampleDurationPresent) != 0)
            {
                sampleDefaults.defaultSampleDuration = mTrackFragmentHeaderBox.getDefaultSampleDuration();
            }
            if ((mTrackFragmentHeaderBox.getFlags() & TrackFragmentHeaderBox::DefaultSampleSizePresent) != 0)
            {
                sampleDefaults.defaultSampleSize = mTrackFragmentHeaderBox.getDefaultSampleSize();
            }
            if ((mTrackFragmentHeaderBox.getFlags() & TrackFragmentHeaderBox::DefaultSampleFlagsPresent) != 0)
            {
                sampleDefaults.defaultSampleFlags = mTrackFragmentHeaderBox.getDefaultSampleFlags();
            }
            UniquePtr<TrackRunBox> trackRunBox(CUSTOM_NEW(TrackRunBox, ()));
            if (defaultsFound)
            {
                trackRunBox->setSampleDefaults(sampleDefaults);
            }

            trackRunBox->parseBox(subBitstr);
            mTrackRunBoxes.push_back(std::move(trackRunBox));
        }
        else
        {
            logWarning() << "Skipping an unsupported box '" << boxType.getString() << "' inside TrackFragmentBox."
                         << std::endl;
        }
    }

    if (!foundTfhd)
    {
        throw RuntimeError("TrackFragmentBox: tfhd box missing (mandatory)");
    }
}
