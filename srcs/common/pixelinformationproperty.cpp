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

#include "pixelinformationproperty.hpp"

#include <stddef.h>
#include <utility>
#include <vector>

#include "bitstream.hpp"
#include "fourccint.hpp"

PixelInformationProperty::PixelInformationProperty()
    : FullBox("pixi", 0, 0)
{
}

const Vector<std::uint8_t>& PixelInformationProperty::getBitsPerChannels() const
{
    return mBitsPerChannel;
}

void PixelInformationProperty::setBitsPerChannels(const Vector<std::uint8_t>& bitsPerChannel)
{
    mBitsPerChannel = bitsPerChannel;
}
void PixelInformationProperty::setNumChannels(uint8_t numc)
{
    mNumChannels = numc;
}

const uint8_t &PixelInformationProperty::getNumChannels() const
{
    return mNumChannels;
}

void PixelInformationProperty::setComponentFormat(const uint8_t& idc, uint8_t componentformat)
{
    mChannelData[idc].ComponentFormat = componentformat;
}

uint8_t PixelInformationProperty::getComponentFormat(const uint8_t& idc) const
{
    return mChannelData.at(idc).ComponentFormat;
}

void PixelInformationProperty::setSubSamplingFlag(const uint8_t& idc,bool ssamplingflag)
{
    mChannelData[idc].SubSamplingFlag = ssamplingflag;
}

bool PixelInformationProperty::getSubSamplingFlag(const uint8_t& idc) const
{
    return mChannelData.at(idc).SubSamplingFlag;
}

void PixelInformationProperty::setChannelLabelFlag(const uint8_t& idc, bool clabelflag)
{
    mChannelData[idc].ChannelLabelFlag = clabelflag;
}

bool PixelInformationProperty::getChannelLabelFlag(const uint8_t& idc) const
{
    return mChannelData.at(idc).ChannelLabelFlag;
}

void PixelInformationProperty::setSubSamplingType(const uint8_t& idc, uint8_t ssamplingtype)
{
    mChannelData[idc].SubSamplingType = ssamplingtype;
}

uint8_t PixelInformationProperty::getSubSamplingType(const uint8_t& idc) const
{
    return mChannelData.at(idc).SubSamplingType;
}

void PixelInformationProperty::setSubSamplingLocation(const uint8_t& idc, uint8_t ssamplingloc)
{
    mChannelData[idc].SubSamplingLocation = ssamplingloc;
}

uint8_t PixelInformationProperty::getSubSamplingLocation(const uint8_t& idc) const
{
    return mChannelData.at(idc).SubSamplingLocation;
}

void PixelInformationProperty::setChannelLabel(const uint8_t& idc, std::string clabel)
{
    mChannelData[idc].ChannelLabel = std::move(clabel);
}

std::string PixelInformationProperty::getChannelLabel(const uint8_t& idc) const
{
    return mChannelData.at(idc).ChannelLabel;
}

void PixelInformationProperty::setChannelData(const std::map<uint8_t, ChannelInfo>& chaninfo)
{
    mChannelData = chaninfo;
}

const std::map<uint8_t, PixelInformationProperty::ChannelInfo> PixelInformationProperty::getChannelData() const
{
    return mChannelData;
}

/// @todo ensure logic, maybe add documentation (would also be cleaner with YUV4XX enum type)
const std::pair<bool,bool> PixelInformationProperty::getChromaCenterdness(const uint8_t& idc) const{
    std::pair<bool,bool> subsamp;
    if(mChannelData.at(idc).SubSamplingType == 0){ subsamp = {1, 1};}
    else if(mChannelData.at(idc).SubSamplingType==1){
        if(mChannelData.at(idc).SubSamplingLocation & 1 ){subsamp = {0, 1};}
        else{subsamp = {1,1};}
    }
    else if(mChannelData.at(idc).SubSamplingType==2){
        if(mChannelData.at(idc).SubSamplingLocation == 0){subsamp = {1,0};}
        else if(mChannelData.at(idc).SubSamplingLocation == 1){subsamp = {0,0};}
        else if(mChannelData.at(idc).SubSamplingLocation == 2){subsamp = {1,1};}
        else if(mChannelData.at(idc).SubSamplingLocation == 3){subsamp = {0,1};}
        else if(mChannelData.at(idc).SubSamplingLocation == 4){subsamp = {1,0};}
        else if(mChannelData.at(idc).SubSamplingLocation == 5){subsamp = {0,0};}
    }
    else if(mChannelData.at(idc).SubSamplingType==3){
        if(mChannelData.at(idc).SubSamplingLocation & 1){subsamp = {0,1};}
        else{subsamp = {1,1};}
    }
    else if(mChannelData.at(idc).SubSamplingType==4){
        if(mChannelData.at(idc).SubSamplingLocation == 0){subsamp = {1,0};}
        else if(mChannelData.at(idc).SubSamplingLocation == 1){subsamp = {1,0};}
        else if(mChannelData.at(idc).SubSamplingLocation == 2){subsamp = {1,1};}
        else if(mChannelData.at(idc).SubSamplingLocation == 3){subsamp = {1,1};}
        else if(mChannelData.at(idc).SubSamplingLocation == 4){subsamp = {1,0};}
        else if(mChannelData.at(idc).SubSamplingLocation == 5){subsamp = {1,0};}
    }

    return subsamp;
}

void PixelInformationProperty::writeBox(BitStream& output) const
{
    writeFullBoxHeader(output);
    output.write8Bits(mNumChannels);
    for (const auto bitsPerChannel : mBitsPerChannel)
    {
        output.write8Bits(bitsPerChannel);
    }
    if(getFlags() != 0){
        for(auto const& channel : mChannelData){
            output.writeBits(channel.first,3); // channel IDC
            output.writeBits(0, 1); // Reserved 0 Bit
            output.writeBits(channel.second.ComponentFormat, 2);
            output.writeBits(channel.second.SubSamplingFlag, 1);
            output.writeBits(channel.second.ChannelLabelFlag, 1);
            if(channel.second.SubSamplingFlag){
                output.writeBits(channel.second.SubSamplingType, 4);
                output.writeBits(channel.second.SubSamplingLocation, 4);
            }
            if(channel.second.ChannelLabelFlag){
                output.writeZeroTerminatedString(String(channel.second.ChannelLabel));
            }
        }
    }
    updateSize(output);
}

void PixelInformationProperty::parseBox(BitStream& input)
{
    mBitsPerChannel.clear();
    parseFullBoxHeader(input);
    mNumChannels = input.read8Bits();
    mBitsPerChannel.reserve(mNumChannels);
    for (unsigned int i = 0; i < mNumChannels; i++){
        mBitsPerChannel.push_back(input.read8Bits());
    }
    if(getFlags() != 0){
        for(size_t i=0;i<mNumChannels;i++){
            PixelInformationProperty::ChannelInfo chan_info;
            uint8_t channelIdc = static_cast<uint8_t>(input.readBits(3));
            input.readBits(1);  //reserved = 0;
            chan_info.ComponentFormat = static_cast<uint8_t>(input.readBits(2));
            chan_info.SubSamplingFlag=input.readBits(1);
            chan_info.ChannelLabelFlag=input.readBits(1);
            if(chan_info.SubSamplingFlag){
                chan_info.SubSamplingType=static_cast<uint8_t>(input.readBits(4));
                chan_info.SubSamplingLocation=static_cast<uint8_t>(input.readBits(4));
            }
            if(chan_info.ChannelLabelFlag){
                String readStr;
                input.readZeroTerminatedString(readStr);
                chan_info.ChannelLabel=std::string(readStr);
            }

            mChannelData.emplace(channelIdc,chan_info);
        }
    }
}
