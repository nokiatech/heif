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

#include "channellayoutbox.hpp"
#include <stdexcept>

#define CHANNEL_STRUCTURED 1u
#define OBJECT_STRUCTURED 2u

ChannelLayoutBox::ChannelLayoutBox()
    : FullBox("chnl", 0, 0)
    , mStreamStructure(0)
    , mDefinedLayout(0)
    , mOmittedChannelsMap(0)
    , mObjectCount(0)
    , mChannelCount(0)
    , mChannelLayouts()
{
}

ChannelLayoutBox::ChannelLayoutBox(const ChannelLayoutBox& box)
    : FullBox(box.getType(), box.getVersion(), box.getFlags())
    , mStreamStructure(box.mStreamStructure)
    , mDefinedLayout(box.mDefinedLayout)
    , mOmittedChannelsMap(box.mOmittedChannelsMap)
    , mObjectCount(box.mObjectCount)
    , mChannelCount(box.mChannelCount)
    , mChannelLayouts(box.mChannelLayouts)
{
}

const Vector<ChannelLayoutBox::ChannelLayout>& ChannelLayoutBox::getChannelLayouts() const
{
    return mChannelLayouts;
}

void ChannelLayoutBox::addChannelLayout(ChannelLayout& channelLayout)
{
    mChannelLayouts.push_back(channelLayout);
    // set other member variables to mirror fact that mChannelLayouts is present
    mStreamStructure |= CHANNEL_STRUCTURED;  // if channellayouts are present then stream is structured.
    mDefinedLayout      = 0;                 // if hannellayouts are present then layout is defined.
    mOmittedChannelsMap = 0;
}

std::uint8_t ChannelLayoutBox::getStreamStructure() const
{
    return mStreamStructure;
}

std::uint8_t ChannelLayoutBox::getDefinedLayout() const
{
    return mDefinedLayout;
}

void ChannelLayoutBox::setDefinedLayout(std::uint8_t definedLayout)
{
    mDefinedLayout = definedLayout;
    // set other member variables to mirror fact that mDefinedLayout is present
    if (mDefinedLayout)
    {
        mChannelLayouts.clear();
    }
    mStreamStructure |= CHANNEL_STRUCTURED;
}

std::uint64_t ChannelLayoutBox::getOmittedChannelsMap() const
{
    return mOmittedChannelsMap;
}

void ChannelLayoutBox::setOmittedChannelsMap(std::uint64_t omittedChannelsMap)
{
    mOmittedChannelsMap = omittedChannelsMap;
    // set other member variables to mirror fact that mOmittedChannelsMap is present
    mChannelLayouts.clear();
    mStreamStructure |= CHANNEL_STRUCTURED;
}

std::uint8_t ChannelLayoutBox::getObjectCount() const
{
    return mObjectCount;
}

void ChannelLayoutBox::setObjectCount(std::uint8_t objectCount)
{
    mObjectCount = objectCount;
    mStreamStructure |= OBJECT_STRUCTURED;
}

std::uint16_t ChannelLayoutBox::getChannelCount() const
{
    return mChannelCount;
}

void ChannelLayoutBox::setChannelCount(std::uint16_t channelCount)
{
    mChannelCount = channelCount;
}

void ChannelLayoutBox::writeBox(BitStream& bitstr) const
{
    writeFullBoxHeader(bitstr);

    bitstr.write8Bits(mStreamStructure);
    if (mStreamStructure & CHANNEL_STRUCTURED)  // channels
    {
        bitstr.write8Bits(mDefinedLayout);
        if (mDefinedLayout == 0)
        {
            if (mChannelLayouts.size() != mChannelCount)
            {
                throw RuntimeError("ChannelLayoutBox mChannelCount doesn't match mChannelLayout.size()");
            }

            for (std::uint16_t i = 0; i < mChannelLayouts.size(); i++)
            {
                bitstr.write8Bits(mChannelLayouts.at(i).speakerPosition);
                if (mChannelLayouts[i].speakerPosition == 126)
                {
                    bitstr.write16Bits(static_cast<std::uint16_t>(mChannelLayouts.at(i).azimuth));
                    bitstr.write8Bits(static_cast<std::uint8_t>(mChannelLayouts.at(i).elevation));
                }
            }
        }
        else
        {
            bitstr.write64Bits(mOmittedChannelsMap);
        }
    }
    else if (mStreamStructure & OBJECT_STRUCTURED)  // objects
    {
        bitstr.write8Bits(mObjectCount);
    }

    updateSize(bitstr);
}

void ChannelLayoutBox::parseBox(BitStream& bitstr)
{
    parseFullBoxHeader(bitstr);
    mStreamStructure = bitstr.read8Bits();

    if (mStreamStructure & CHANNEL_STRUCTURED)  // Stream carries channels.
    {
        mDefinedLayout = bitstr.read8Bits();
        if (mDefinedLayout == 0)
        {
            for (std::uint16_t i = 1; i <= mChannelCount; i++)
            {
                ChannelLayout channelPosition;
                channelPosition.speakerPosition = bitstr.read8Bits();
                channelPosition.azimuth         = 0;
                channelPosition.elevation       = 0;
                if (channelPosition.speakerPosition == 126)  // explicit position
                {
                    channelPosition.azimuth   = static_cast<std::int16_t>(bitstr.read16Bits());
                    channelPosition.elevation = static_cast<std::int8_t>(bitstr.read8Bits());
                }
                mChannelLayouts.push_back(channelPosition);
            }
        }
        else
        {
            mOmittedChannelsMap = bitstr.read64Bits();
        }
    }

    if (mStreamStructure & OBJECT_STRUCTURED)  // Stream carries objects
    {
        mObjectCount = bitstr.read8Bits();
    }
}
