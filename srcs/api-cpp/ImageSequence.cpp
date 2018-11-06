/*
 * This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2018 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its
 * subsidiaries. All rights are reserved. Copying, including reproducing, storing, adapting or translating, any or all
 * of this material requires the prior written consent of Nokia.
 */

#include "ImageSequence.h"
#include <cstring>
#include "VideoSample.h"
#include "heifreader.h"
#include "heifwriter.h"

using namespace HEIFPP;


ImageSequence::ImageSequence(Heif* aHeif)
    : VideoTrack(aHeif)
    , mHasAux(false)
    , mHasClap(false)
    , mHasCodingConstraints(false)
    , mAuxProperty{}
    , mClapProperty{0, 0, 0, 0, 0, 0, 0, 0}
    , mCodingConstraints{}

{
    mHandler = HEIF::FourCC("pict");
}
ImageSequence::~ImageSequence()
{
}

const HEIF::AuxiliaryType* ImageSequence::aux() const
{
    if (mHasAux)
        return &mAuxProperty;
    return nullptr;
}
const HEIF::CleanAperture* ImageSequence::clap() const
{
    if (mHasClap)
        return &mClapProperty;
    return nullptr;
}

bool ImageSequence::getAllRefPicsIntra() const
{
    return mCodingConstraints.allRefPicsIntra;
}
bool ImageSequence::getIntraPredUsed() const
{
    return mCodingConstraints.intraPredUsed;
}
std::uint8_t ImageSequence::getMaxRefPerPic() const
{
    return mCodingConstraints.maxRefPerPic;
}

void ImageSequence::setAllRefPicsIntra(bool aArg)
{
    mHasCodingConstraints = true;
    mFeatures |= HEIF::TrackFeatureEnum::Feature::HasCodingConstraints;
    mCodingConstraints.allRefPicsIntra = aArg;
}
void ImageSequence::setIntraPredUsed(bool aArg)
{
    mHasCodingConstraints = true;
    mFeatures |= HEIF::TrackFeatureEnum::Feature::HasCodingConstraints;
    mCodingConstraints.intraPredUsed = aArg;
}
void ImageSequence::setMaxRefPerPic(std::uint8_t aArg)
{
    mHasCodingConstraints = true;
    mFeatures |= HEIF::TrackFeatureEnum::Feature::HasCodingConstraints;
    mCodingConstraints.maxRefPerPic = aArg;
}


HEIF::ErrorCode ImageSequence::load(HEIF::Reader* aReader, const HEIF::SequenceId& aId)
{
    HEIF::ErrorCode ret = VideoTrack::load(aReader, aId);
    // NOTE: Even though technically it is possible for each sample to contain these properties,
    //      There "should" be only one clap/aux/codingconstraint per track.
    //      so to support files created by others we need to :
    //      1. scan through all samples (ie. the sample descriptions used in the track)
    //      2. see if codingconstraint/aux/clap properties are set. and store them.
    //      3. and warn user if the data changes mid-track....
    auto info = mHeif->getTrackInformation(aId);
    HEIF::AuxiliaryType a;
    HEIF::CleanAperture c;
    std::uint32_t asd = 0, csd = 0;
    bool firstSample      = true;
    mHasCodingConstraints = (info->features & HEIF::TrackFeatureEnum::HasCodingConstraints) != 0;

    for (const auto& at : info->sampleProperties)
    {
        if (mHasCodingConstraints)
        {
            if (firstSample)
            {
                mCodingConstraints = at.codingConstraints;
            }
            else
            {
                // sanity check. we cant write tracks which have changing coding constraints.
                if ((mCodingConstraints.allRefPicsIntra != at.codingConstraints.allRefPicsIntra) ||
                    (mCodingConstraints.intraPredUsed != at.codingConstraints.intraPredUsed) ||
                    (mCodingConstraints.maxRefPerPic != at.codingConstraints.maxRefPerPic))
                {
                    // Warning! coding constraints change mid-track
                }
            }
        }
        if (at.hasAuxi)
        {
            aReader->getProperty(aId, at.sampleDescriptionIndex, a);
            if (mHasAux)
            {
                // verify..
                if ((a.auxType.size != mAuxProperty.auxType.size) ||
                    (0 != std::memcmp(a.auxType.elements, mAuxProperty.auxType.elements, a.auxType.size)))
                {
                    // Warning! aux property changes mid-track
                }
            }
            else
            {
                if (!firstSample)
                {
                    // Warning! property was not defined in the first sample!
                }
                mAuxProperty = a;
                asd          = at.sampleDescriptionIndex;
                mHasAux      = true;
            }
        }
        if (at.hasClap)
        {
            aReader->getProperty(aId, at.sampleDescriptionIndex, c);
            if (mHasClap)
            {
                if (0 != std::memcmp(&c, &mClapProperty, sizeof(c)))
                {
                    // Warning! clap property changes mid-track
                }
            }
            else
            {
                if (!firstSample)
                {
                    // Warning! property was not defined in the first sample!
                }
                mClapProperty = c;
                csd           = at.sampleDescriptionIndex;
                mHasClap      = true;
            }
        }
        firstSample = false;
    }
    if (csd != asd)
    {
        // Warning! properties were defined in different samples descriptions!
    }
    return ret;
}
HEIF::ErrorCode ImageSequence::save(HEIF::Writer* aWriter)
{
    HEIF::Rational tb={1,mTimeScale};
    HEIF::ErrorCode err=HEIF::ErrorCode::OK;
    // see the comment in load about codingconstraints/aux/clap..
    err = aWriter->addImageSequence(tb, mCodingConstraints, mId);
    if (HEIF::ErrorCode::OK!=err)
    {
        return err;
    }
    // see the comment in load about codingconstraints/aux/clap..
    for (std::uint32_t i = 0; i < mIsAuxiliaryTo.size(); i++)
    {
        auto ref = mIsAuxiliaryTo[0].first;
        if (ref->getId() == Heif::InvalidSequence)
        {
            err=ref->save(aWriter);
            if (HEIF::ErrorCode::OK!=err)
            {
                return err;
            }
        }
        err=aWriter->addAuxiliaryReference(mAuxProperty, mId, ref->getId());
        if (HEIF::ErrorCode::OK!=err)
        {
            return err;
        }
    }

    if (mHasClap)
    {
        err = aWriter->addProperty(mClapProperty, mId);
        if (HEIF::ErrorCode::OK!=err)
        {
            return err;
        }
    }

    err = VideoTrack::save(aWriter);
    return err;
}
