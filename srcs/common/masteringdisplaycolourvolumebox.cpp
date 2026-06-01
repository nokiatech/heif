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

#include "masteringdisplaycolourvolumebox.hpp"

#include <utility>

#include "bitstream.hpp"
#include "fourccint.hpp"

#include <stddef.h>

MasteringDisplayColourVolumeBox::MasteringDisplayColourVolumeBox()
    :Box("mdcv")
{
}

void MasteringDisplayColourVolumeBox::setDisplayPrimariesX(Vector<uint16_t> input){
    mDisplayPrimariesX = std::move(input);
}

Vector<uint16_t> MasteringDisplayColourVolumeBox::getDisplayPrimariesY() const{
    return mDisplayPrimariesY;
}

void MasteringDisplayColourVolumeBox::setDisplayPrimariesY(Vector<uint16_t> input){
    mDisplayPrimariesY = std::move(input);
}

Vector<uint16_t> MasteringDisplayColourVolumeBox::getDisplayPrimariesX() const{
    return mDisplayPrimariesX;
}

void MasteringDisplayColourVolumeBox::setWhitePointX(uint16_t value){
    mWhitePointX = value;
}

uint16_t MasteringDisplayColourVolumeBox::getWhitePointX() const{
    return mWhitePointX;
}

void MasteringDisplayColourVolumeBox::setWhitePointY(uint16_t value){
    mWhitePointY = value;
}

uint16_t MasteringDisplayColourVolumeBox::getWhitePointY() const{
    return mWhitePointY;
}

void MasteringDisplayColourVolumeBox::setWhitePoint(uint16_t wpX, uint16_t wpY){
    mWhitePointX = wpX;
    mWhitePointY = wpY;
}

std::pair<uint16_t, uint16_t> MasteringDisplayColourVolumeBox::getWhitePoint(){
    return {mWhitePointX, mWhitePointY};
}

void MasteringDisplayColourVolumeBox::setMaxDisplayMasteringLuminance(uint32_t value){
    mMaxDisplayMasteringLuminance = value;
}

uint32_t MasteringDisplayColourVolumeBox::getMaxDisplayMasteringLuminance() const{
    return mMaxDisplayMasteringLuminance;
}

void MasteringDisplayColourVolumeBox::setMinDisplayMasteringLuminance(uint32_t value){
    mMinDisplayMasteringLuminance = value;
}

uint32_t MasteringDisplayColourVolumeBox::getMinDisplayMasteringLuminance() const{
    return mMinDisplayMasteringLuminance;
}


void MasteringDisplayColourVolumeBox::writeBox(ISOBMFF::BitStream &output) const
{
    writeBoxHeader(output);
    for(size_t i=0;i<3;i++){
        output.write16Bits(mDisplayPrimariesX.at(i));
        output.write16Bits(mDisplayPrimariesY.at(i));
    }
    output.write16Bits(mWhitePointX);
    output.write16Bits(mWhitePointY);
    output.write32Bits(mMaxDisplayMasteringLuminance);
    output.write32Bits(mMinDisplayMasteringLuminance);
    updateSize(output);
}

void MasteringDisplayColourVolumeBox::parseBox(ISOBMFF::BitStream &input)
{
    parseBoxHeader(input);
    mDisplayPrimariesX.reserve(3);
    mDisplayPrimariesY.reserve(3);
    for(int i=0;i<3;i++){
        mDisplayPrimariesX.push_back(input.read16Bits());
        mDisplayPrimariesY.push_back(input.read16Bits());
    }
    mWhitePointX = input.read16Bits();
    mWhitePointY = input.read16Bits();
    mMaxDisplayMasteringLuminance = input.read32Bits();
    mMinDisplayMasteringLuminance = input.read32Bits();
}
