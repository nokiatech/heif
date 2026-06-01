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

#include "contentcolourvolumebox.hpp"

#include <utility>

#include "bitstream.hpp"
#include "fourccint.hpp"

#include <stddef.h>

ContentColourVolumeBox::ContentColourVolumeBox()
    :Box("cclv")
     
{
}

void ContentColourVolumeBox::setCcvPrimariesX(Vector<uint32_t> input){
    mCcvPrimariesPresent = true;
    mCcvPrimariesX = std::move(input);
}

Vector<uint32_t> ContentColourVolumeBox::getCcvPrimariesX() const{
    return mCcvPrimariesX;
}

void ContentColourVolumeBox::setCcvPrimariesY(Vector<uint32_t> input){
    mCcvPrimariesPresent = true;
    mCcvPrimariesY = std::move(input);
}

Vector<uint32_t> ContentColourVolumeBox::getCcvPrimariesY() const{
    return mCcvPrimariesY;
}

void ContentColourVolumeBox::setCcvMinLuminance(uint32_t value){
    mCcvMinLuminanceValuePresent = true;
    mCcvMinLuminanceValue = value;
}

uint32_t ContentColourVolumeBox::getCcvMinLuminance() const{
    return mCcvMinLuminanceValue;
}

void ContentColourVolumeBox::setCcvMaxLuminance(uint32_t value){
    mCcvMaxLuminanceValuePresent = true;
    mCcvMaxLuminanceValue = value;
}

uint32_t ContentColourVolumeBox::getCcvMaxLuminance() const{
    return mCcvMaxLuminanceValue;
}

void ContentColourVolumeBox::setCcvAvgLuminance(uint32_t value){
    mCcvAvgLuminanceValuePresent = true;
    mCcvAvgLuminanceValue = value;
}

uint32_t ContentColourVolumeBox::getCcvAvgLuminance() const{
    return mCcvAvgLuminanceValue;
}


void ContentColourVolumeBox::writeBox(ISOBMFF::BitStream &output) const
{
    writeBoxHeader(output);
    output.writeBits(0,1);  //reserved = 0; ccv_cancel_flag
    output.writeBits(0,1);  //reserved = 0; ccv_persitence_flag
    output.writeBits(mCcvPrimariesPresent,1);
    output.writeBits(mCcvMinLuminanceValuePresent,1);
    output.writeBits(mCcvMaxLuminanceValuePresent,1);
    output.writeBits(mCcvAvgLuminanceValuePresent,1);
    output.writeBits(0,2);  //reserved = 0;
    if(mCcvPrimariesPresent){
        for(size_t i=0;i<3;i++){
            output.write32Bits(mCcvPrimariesX.at(i));
            output.write32Bits(mCcvPrimariesY.at(i));
        }
    }
    if(mCcvMinLuminanceValuePresent){
        output.write32Bits(mCcvMinLuminanceValue);
    }
    if(mCcvMaxLuminanceValuePresent){
        output.write32Bits(mCcvMaxLuminanceValue);
    }
    if(mCcvAvgLuminanceValuePresent){
        output.write32Bits(mCcvAvgLuminanceValue);
    }
    updateSize(output);
}

void ContentColourVolumeBox::parseBox(ISOBMFF::BitStream &input)
{
    parseBoxHeader(input);
    input.readBits(1); //Reserved 0 ccv_cancel_flag
    input.readBits(1); //Reserved 0 ccv_persistence_flag
    mCcvPrimariesPresent = input.readBits(1);
    mCcvMinLuminanceValuePresent = input.readBits(1);
    mCcvMaxLuminanceValuePresent = input.readBits(1);
    mCcvAvgLuminanceValuePresent = input.readBits(1);

    input.readBits(2);  //reserved 2 bits

    if(mCcvPrimariesPresent){
        mCcvPrimariesX.reserve(3);
        mCcvPrimariesY.reserve(3);
        for(int i=0;i<3;i++){
            mCcvPrimariesX.push_back(input.read32Bits());
            mCcvPrimariesY.push_back(input.read32Bits());
        }
    }
    if(mCcvMinLuminanceValuePresent){
        mCcvMinLuminanceValue = input.read32Bits();
    }
    if(mCcvMaxLuminanceValuePresent){
        mCcvMaxLuminanceValue = input.read32Bits();
    }
    if(mCcvAvgLuminanceValuePresent){
        mCcvAvgLuminanceValue = input.read32Bits();
    }
}
