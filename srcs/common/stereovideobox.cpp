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

#include <cstdint>

#include "bitstream.hpp"
#include "log.hpp"
#include "stereovideobox.hpp"

const uint32_t SINGLE_VIEW_ALLOWED_MASK = 0x3;

StereoVideoBox::StereoVideoBox()
    : FullBox("stvi", 0, 0)
    , mSingleViewAllowed(StereoVideoBox::SingleViewAllowedType::None)
    , mStereoScheme(StereoVideoBox::StereoSchemeType::Iso14496)
{
}

void StereoVideoBox::setSingleViewAllowed(SingleViewAllowedType allowed)
{
    mSingleViewAllowed = allowed;
}

StereoVideoBox::SingleViewAllowedType StereoVideoBox::getSingleViewAllowed() const
{
    return mSingleViewAllowed;
}

void StereoVideoBox::setStereoScheme(StereoSchemeType scheme)
{
    mStereoScheme = scheme;
}

StereoVideoBox::StereoSchemeType StereoVideoBox::getStereoScheme() const
{
    return mStereoScheme;
}

void StereoVideoBox::setStereoIndicationType(StereoIndicationType indicationType)
{
    mStereoIndicationType = indicationType;
}

StereoVideoBox::StereoIndicationType StereoVideoBox::getStereoIndicationType() const
{
    return mStereoIndicationType;
}

void StereoVideoBox::writeBox(ISOBMFF::BitStream& bitstr) const
{
    writeFullBoxHeader(bitstr);
    bitstr.write32Bits((uint32_t) mSingleViewAllowed & SINGLE_VIEW_ALLOWED_MASK);
    bitstr.write32Bits((uint32_t) mStereoScheme);

    switch (mStereoScheme)
    {
    case StereoSchemeType::Iso13818:
    case StereoSchemeType::Iso14496:
        bitstr.write32Bits(4);  // length 4
        bitstr.write32Bits((uint32_t) mStereoIndicationType.valAsUint32);
        break;
    case StereoSchemeType::Iso23000:
        bitstr.write32Bits(2);
        bitstr.write8Bits((uint8_t) mStereoIndicationType.Iso23000.compositionType);
        bitstr.write8Bits(mStereoIndicationType.Iso23000.isLeftFirst ? 0x01 : 0x00);
        break;
    case StereoSchemeType::Povd:
        bitstr.write32Bits(2);
        bitstr.write8Bits((uint8_t) mStereoIndicationType.Povd.compositionType);
        bitstr.write8Bits(mStereoIndicationType.Povd.useQuincunxSampling ? 0x01 : 0x00);
        break;
    }

    updateSize(bitstr);
}

void StereoVideoBox::parseBox(ISOBMFF::BitStream& bitstr)
{
    parseFullBoxHeader(bitstr);

    mSingleViewAllowed                  = (SingleViewAllowedType)(bitstr.read32Bits() & SINGLE_VIEW_ALLOWED_MASK);
    mStereoScheme                       = (StereoSchemeType)(bitstr.read32Bits());
    uint32_t stereoIndicationTypeLength = bitstr.read32Bits();

    auto checkLen = [&](uint32_t len) {
        if (stereoIndicationTypeLength != len)
        {
            logInfo() << "Invalid length (" << stereoIndicationTypeLength
                      << ") for stvi stereo_indication_type data. For scheme_type " << (uint32_t) mStereoScheme
                      << " expected length is " << len << std::endl;
        }
    };

    switch (mStereoScheme)
    {
    case StereoSchemeType::Iso13818:
    case StereoSchemeType::Iso14496:
        mStereoIndicationType.valAsUint32 = bitstr.read32Bits();
        checkLen(4);
        break;
    case StereoSchemeType::Iso23000:
        mStereoIndicationType.Iso23000.compositionType = (Iso23000StereoscopicCompositionType) bitstr.read8Bits();
        mStereoIndicationType.Iso23000.isLeftFirst     = bitstr.read8Bits() & 0x01;
        checkLen(2);
        break;
    case StereoSchemeType::Povd:
        mStereoIndicationType.Povd.compositionType     = (PovdFrameCompositionType) bitstr.read8Bits();
        mStereoIndicationType.Povd.useQuincunxSampling = bitstr.read8Bits() & 0x01;
        checkLen(2);
        break;
    }
}
