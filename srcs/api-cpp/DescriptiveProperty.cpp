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

#include "DescriptiveProperty.h"
#include <heifreader.h>
#include <heifwriter.h>
#include <cstring>

using namespace HEIFPP;

DescriptiveProperty::DescriptiveProperty(Heif* aHeif, const HEIF::ItemPropertyType& aProp, const HEIF::FourCC& aRawType)
    : ItemProperty(aHeif, aProp, aRawType, false){};
HEIF::ErrorCode DescriptiveProperty::load(HEIF::Reader* aReader, const HEIF::PropertyId& aId)
{
    return ItemProperty::load(aReader, aId);
}

PixelAspectRatioProperty::PixelAspectRatioProperty(Heif* aHeif)
    : DescriptiveProperty(aHeif, HEIF::ItemPropertyType::PASP,"pasp")
    , mPixelAspectRatio{}
{
}
HEIF::ErrorCode PixelAspectRatioProperty::load(HEIF::Reader* aReader, const HEIF::PropertyId& aId)
{
    HEIF::ErrorCode error;
    error = DescriptiveProperty::load(aReader, aId);
    if (HEIF::ErrorCode::OK == error)
    {
        error = aReader->getProperty(aId, mPixelAspectRatio);
    }
    return error;
};
HEIF::ErrorCode PixelAspectRatioProperty::save(HEIF::Writer* aWriter)
{
    HEIF::PropertyId newId;
    HEIF::ErrorCode error;
    error = aWriter->addProperty(mPixelAspectRatio, newId);
    if (HEIF::ErrorCode::OK == error)
    {
        setId(newId);
    }
    return error;
}

ColourInformationProperty::ColourInformationProperty(Heif* aHeif)
    : DescriptiveProperty(aHeif, HEIF::ItemPropertyType::COLR,"colr")
    , mColourInformation{}
{
}
HEIF::ErrorCode ColourInformationProperty::load(HEIF::Reader* aReader, const HEIF::PropertyId& aId)
{
    HEIF::ErrorCode error;
    error = DescriptiveProperty::load(aReader, aId);
    if (HEIF::ErrorCode::OK == error)
    {
        error = aReader->getProperty(aId, mColourInformation);
    }
    return error;
};
HEIF::ErrorCode ColourInformationProperty::save(HEIF::Writer* aWriter)
{
    HEIF::PropertyId newId;
    HEIF::ErrorCode error;
    error = aWriter->addProperty(mColourInformation, newId);
    if (HEIF::ErrorCode::OK == error)
    {
        setId(newId);
    }
    return error;
}

PixelInformationProperty::PixelInformationProperty(Heif* aHeif)
    : DescriptiveProperty(aHeif, HEIF::ItemPropertyType::PIXI,"pixi")
    , mPixelInformation{0}
{
}
HEIF::ErrorCode PixelInformationProperty::load(HEIF::Reader* aReader, const HEIF::PropertyId& aId)
{
    HEIF::ErrorCode error;
    error = DescriptiveProperty::load(aReader, aId);
    if (HEIF::ErrorCode::OK == error)
    {
        error = aReader->getProperty(aId, mPixelInformation);
    }
    return error;
};
HEIF::ErrorCode PixelInformationProperty::save(HEIF::Writer* aWriter)
{
    HEIF::PropertyId newId;
    HEIF::ErrorCode error;
    error = aWriter->addProperty(mPixelInformation, newId);
    if (HEIF::ErrorCode::OK == error)
    {
        setId(newId);
    }
    return error;
}

RelativeLocationProperty::RelativeLocationProperty(Heif* aHeif)
    : DescriptiveProperty(aHeif, HEIF::ItemPropertyType::RLOC,"rloc")
    , mRelativeLocation{}
{
}
HEIF::ErrorCode RelativeLocationProperty::load(HEIF::Reader* aReader, const HEIF::PropertyId& aId)
{
    HEIF::ErrorCode error;
    error = DescriptiveProperty::load(aReader, aId);
    if (HEIF::ErrorCode::OK == error)
    {
        error = aReader->getProperty(aId, mRelativeLocation);
    }
    return error;
}
HEIF::ErrorCode RelativeLocationProperty::save(HEIF::Writer* aWriter)
{
    HEIF::PropertyId newId;
    HEIF::ErrorCode error;
    error = aWriter->addProperty(mRelativeLocation, newId);
    if (HEIF::ErrorCode::OK == error)
    {
        setId(newId);
    }
    return error;
}

AuxiliaryProperty::AuxiliaryProperty(Heif* aHeif)
    : DescriptiveProperty(aHeif, HEIF::ItemPropertyType::AUXC,"auxc")
{
}
HEIF::ErrorCode AuxiliaryProperty::load(HEIF::Reader* aReader, const HEIF::PropertyId& aId)
{
    HEIF::ErrorCode error;
    error = DescriptiveProperty::load(aReader, aId);
    if (HEIF::ErrorCode::OK == error)
    {
        HEIF::AuxiliaryType p;
        error = aReader->getProperty(aId, p);
        if (HEIF::ErrorCode::OK == error)
        {
            mAuxType = std::string(p.auxType.begin(), p.auxType.end());
            mSubType.assign(p.subType.begin(), p.subType.end());
        }
    }
    return error;
}

HEIF::ErrorCode AuxiliaryProperty::save(HEIF::Writer* aWriter)
{
    HEIF::ErrorCode error = HEIF::ErrorCode::OK;
    HEIF::AuxiliaryType p;

    if (mAuxType.length() > 0)
    {
        p.auxType = HEIF::Array<char>(mAuxType.length());
        std::memcpy(p.auxType.elements, mAuxType.data(), p.auxType.size);
    }
    if (mSubType.size() > 0)
    {
        p.subType = HEIF::Array<uint8_t>(mSubType.size());
        std::memcpy(p.subType.elements, mSubType.data(), p.subType.size);
    }
    HEIF::PropertyId newId;
    error = aWriter->addProperty(p, newId);
    if (HEIF::ErrorCode::OK == error)
    {
        setId(newId);
    }
    return error;
};

const std::string& AuxiliaryProperty::auxType()
{
    return mAuxType;
}
void AuxiliaryProperty::auxType(const std::string& aType)
{
    mAuxType = aType;
}
const std::vector<uint8_t>& AuxiliaryProperty::subType()
{
    return mSubType;
}
void AuxiliaryProperty::subType(const std::vector<uint8_t>& aType)
{
    mSubType = aType;
}
