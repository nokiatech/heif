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

#include "tonemapimage.hpp"

#include <stdexcept>
#include <string>

GainMapChannel::GainMapChannel()
     
= default;

void GainMapChannel::read(BitStream& bitstr){
    gainMapMinNumerator = static_cast<int32_t>(bitstr.readBits(32));
    gainMapMinDenominator = bitstr.readBits(32);
    gainMapMaxNumerator = static_cast<int32_t>(bitstr.readBits(32));
    gainMapMaxDenominator = bitstr.readBits(32);
    gammaNumerator = bitstr.readBits(32);
    gammaDenominator = bitstr.readBits(32);
    baseOffsetNumerator = static_cast<int32_t>(bitstr.readBits(32));
    baseOffsetDenominator = bitstr.readBits(32);
    alternateOffsetNumerator = static_cast<int32_t>(bitstr.readBits(32));
    alternateOffsetDenominator = bitstr.readBits(32);
}

void GainMapChannel::write(BitStream& bitstr) const
{
    bitstr.write32Bits(static_cast<uint32_t>(gainMapMinNumerator));
    bitstr.write32Bits(gainMapMinDenominator);
    bitstr.write32Bits(static_cast<uint32_t>(gainMapMaxNumerator));
    bitstr.write32Bits(gainMapMaxDenominator);
    bitstr.write32Bits(gammaNumerator);
    bitstr.write32Bits(gammaDenominator);
    bitstr.write32Bits(static_cast<uint32_t>(baseOffsetNumerator));
    bitstr.write32Bits(baseOffsetDenominator);
    bitstr.write32Bits(static_cast<uint32_t>(alternateOffsetNumerator));
    bitstr.write32Bits(alternateOffsetDenominator);
}

GainMapMetadata::GainMapMetadata()
    : mVersion{0, 0}
    , 
     mChannels()
{
}

void GainMapMetadata::read(BitStream& bitstr){

    mVersion.minimumVersion = static_cast<uint16_t>(bitstr.readBits(16));
    mVersion.writerVersion = static_cast<uint16_t>(bitstr.readBits(16));

    if(mVersion.minimumVersion == 0){
        mIsMultichannel = bitstr.readBits(1) != 0;
        mUseBaseColourSpace = bitstr.readBits(1) != 0;

        bitstr.readBits(6);  //bit(6) reserved

        mChannelCount = (mIsMultichannel) * 2 + 1;

        mBaseHdrHeadroomNumerator = bitstr.readBits(32);
        mBaseHdrHeadroomDenominator = bitstr.readBits(32);
        mAlternateHdrHeadroomNumerator = bitstr.readBits(32);
        mAlternateHdrHeadroomDenominator = bitstr.readBits(32);

        // NOLINTNEXTLINE(bugprone-too-small-loop-variable): mChannelCount is set to 1 or 3 above, so uint8_t comfortably bounds it.
        for (uint8_t i = 0; i < mChannelCount; ++i){
            GainMapChannel channel;
            channel.read(bitstr);
            addChannel(channel);
        }
    }
}

void GainMapMetadata::write(BitStream& bitstr) const
{
    bitstr.write16Bits(mVersion.minimumVersion);
    bitstr.write16Bits(mVersion.writerVersion);

    if(mVersion.minimumVersion == 0){
        bitstr.writeBits(mIsMultichannel ? 1 : 0, 1);
        bitstr.writeBits(mUseBaseColourSpace ? 1 : 0, 1);
        bitstr.writeBits(0, 6);  //bit(6) reserved

        bitstr.write32Bits(mBaseHdrHeadroomNumerator);
        bitstr.write32Bits(mBaseHdrHeadroomDenominator);
        bitstr.write32Bits(mAlternateHdrHeadroomNumerator);
        bitstr.write32Bits(mAlternateHdrHeadroomDenominator);

        for (const auto& channel : mChannels){
            channel.write(bitstr);
        }
    }else{
        throw std::runtime_error("Unsupported GainMapMetadata version for writing: " + std::to_string(static_cast<int>(mVersion.minimumVersion)));
    }
}

const GainMapVersion GainMapMetadata::getVersion() const
{
    return mVersion;
}

bool GainMapMetadata::isMultichannel() const
{
    return mIsMultichannel;
}

bool GainMapMetadata::useBaseColourSpace() const
{
    return mUseBaseColourSpace;
}

uint32_t GainMapMetadata::getBaseHdrHeadroomNumerator() const
{
    return mBaseHdrHeadroomNumerator;
}

uint32_t GainMapMetadata::getBaseHdrHeadroomDenominator() const
{
    return mBaseHdrHeadroomDenominator;
}

uint32_t GainMapMetadata::getAlternateHdrHeadroomNumerator() const
{
    return mAlternateHdrHeadroomNumerator;
}

uint32_t GainMapMetadata::getAlternateHdrHeadroomDenominator() const
{
    return mAlternateHdrHeadroomDenominator;
}

const Vector<GainMapChannel>& GainMapMetadata::getChannels() const
{
    return mChannels;
}

void GainMapMetadata::setVersion(const GainMapVersion& version)
{
    mVersion = version;
}

void GainMapMetadata::setIsMultichannel(bool isMultichannel)
{
    mIsMultichannel = isMultichannel;
}

void GainMapMetadata::setUseBaseColourSpace(bool useBaseColourSpace)
{
    mUseBaseColourSpace = useBaseColourSpace;
}

void GainMapMetadata::setBaseHdrHeadroom(uint32_t numerator, uint32_t denominator)
{
    mBaseHdrHeadroomNumerator = numerator;
    mBaseHdrHeadroomDenominator = denominator;
}

void GainMapMetadata::setAlternateHdrHeadroom(uint32_t numerator, uint32_t denominator)
{
    mAlternateHdrHeadroomNumerator = numerator;
    mAlternateHdrHeadroomDenominator = denominator;
}

void GainMapMetadata::addChannel(const GainMapChannel& channel)
{
    mChannels.push_back(channel);
}

void GainMapMetadata::clearChannels()
{
    mChannels.clear();
}

ToneMapImage::ToneMapImage()
    : 
     mGainMapMetadata()
{
}

void ToneMapImage::read(BitStream& bitstr)
{
    mVersion = static_cast<uint8_t>(bitstr.readBits(8));
    mGainMapMetadata.read(bitstr);
}

void ToneMapImage::write(BitStream& bitstr) const
{
    bitstr.writeBits(mVersion, 8);
    mGainMapMetadata.write(bitstr);
}

uint8_t ToneMapImage::getVersion() const
{
    return mVersion;
}

const GainMapMetadata& ToneMapImage::getGainMapMetadata() const
{
    return mGainMapMetadata;
}

GainMapMetadata& ToneMapImage::getGainMapMetadata()
{
    return mGainMapMetadata;
}

void ToneMapImage::setVersion(uint8_t version)
{
    mVersion = version;
}

void ToneMapImage::setGainMapMetadata(const GainMapMetadata& metadata)
{
    mGainMapMetadata = metadata;
}


