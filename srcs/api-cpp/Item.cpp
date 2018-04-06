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

#include "Item.h"
#include <heifreader.h>
#include <heifwriter.h>
#include "ItemProperty.h"
#include "MimeItem.h"
#include "RawProperty.h"

using namespace HEIFPP;

Item::Item(Heif* aHeif, const HEIF::FourCC& aType, bool aIsImageItem)
    : mHeif(aHeif)
    , mId(Heif::InvalidItem)
    , mType(aType)
    , mIsProtected(false)
    , mIsImageItem(aIsImageItem)
    , mContext(nullptr)
    , mFirstTransform(0)
    , mTransformCount(0)
{
    mHeif->addItem(this);
}
Heif* Item::getHeif()
{
    return mHeif;
}
const Heif* Item::getHeif() const
{
    return mHeif;
}
Item::~Item()
{
    for (; !mProps.empty();)
    {
        removeProperty(mProps.begin()->first);
    }
    mHeif->removeItem(this);
}

void Item::setContext(const void* aContext)
{
    mContext = aContext;
}
const void* Item::getContext() const
{
    return mContext;
}

const HEIF::FourCC& Item::getType() const
{
    return mType;
}
bool Item::isImageItem() const
{
    return mIsImageItem;
}
bool Item::isTileImageItem() const
{
    // TODO: Tile images are not supported yet.
    return false;
}
bool Item::isMetadataItem() const
{
    return (isExifItem() || isMimeItem());
}
bool Item::isExifItem() const
{
    if (mType == HEIF::FourCC("Exif"))
    {
        return true;
    }
    return false;
}
bool Item::isMimeItem() const
{
    if (mType == HEIF::FourCC("mime"))
    {
        return true;
    }
    return false;
}
bool Item::isXMPItem() const
{
    if (isMimeItem())
    {
        const MimeItem* mime = static_cast<const MimeItem*>(this);
        return (mime->getContentType() == "application/rdf+xml");
    }
    return false;
}
bool Item::isMPEG7Item() const
{
    if (isMimeItem())
    {
        // TODO: Technically should do more verification.
        const MimeItem* mime = static_cast<const MimeItem*>(this);
        return (mime->getContentType() == "text/xml");
    }
    return false;
}

bool Item::isProtected() const
{
    return mIsProtected;
}

void Item::setId(const HEIF::ImageId& id)
{
    mId = id;
}
const HEIF::ImageId& Item::getId() const
{
    return mId;
}

HEIF::ErrorCode Item::load(HEIF::Reader* aReader, const HEIF::ImageId& aId)
{
    HEIF::ErrorCode error;
    HEIF::FourCC type;
    mId   = aId;
    error = aReader->getItemType(aId, type);
    if (HEIF::ErrorCode::OK != error)
        return error;
    HEIF_ASSERT(mType == type);
    const auto* i = mHeif->getItemInformation(aId);  // not all items have this. some have only ImageInformation, and others have both..
    if (i)
    {
        mIsProtected = (bool) (i->features & HEIF::ItemFeatureEnum::IsProtected);
    }

    const auto* i2 = mHeif->getImageInformation(aId);  // not all items have this. some have only ItemInformation, and others have both..
    if (i2)
    {
        if (i)
        {
            HEIF_ASSERT(mIsProtected == (bool) (i2->features & HEIF::ImageFeatureEnum::IsProtected));
        }
        mIsProtected = (bool) (i2->features & HEIF::ImageFeatureEnum::IsProtected);
    }
    HEIF::Array<HEIF::ItemPropertyInfo> propertyInfos;

    error = aReader->getItemProperties(aId, propertyInfos);
    if (HEIF::ErrorCode::OK != error)
        return error;
    for (uint32_t indx = 0; indx < propertyInfos.size; ++indx)
    {
        const HEIF::ItemPropertyInfo& p = propertyInfos[indx];
        ItemProperty* prop              = mHeif->constructItemProperty(aReader, p, error);
        if (HEIF::ErrorCode::OK != error)
        {
            return error;
        }
        if (prop)
        {
            addProperty(prop, p.essential);
        }
    }
    return HEIF::ErrorCode::OK;
}
HEIF::ErrorCode Item::save(HEIF::Writer* aWriter)
{
    HEIF::ErrorCode error;
    // add properties..
    for (const auto& it : mProps)
    {
        if (it.first)
        {
            if ((it.first) && (it.first->getId() == Heif::InvalidProperty))
            {
                error = it.first->save(aWriter);
                if (HEIF::ErrorCode::OK != error)
                    return error;
            }
            error = aWriter->associateProperty(mId, it.first->getId(), it.second);
            if (HEIF::ErrorCode::OK != error)
                return error;
        }
    }

    return HEIF::ErrorCode::OK;
}

void Item::removeProperty(ItemProperty* aProp)
{
    if (aProp == nullptr)
        return;
    for (auto it = mProps.begin(); it != mProps.end(); it++)
    {
        if (it->first == aProp)
        {
            mProps.erase(it);
            aProp->unlink(this);
            if (aProp->isTransformative())
            {
                HEIF_ASSERT(mTransformCount > 0);
                mTransformCount--;
            }
            break;
        }
    }
}
uint32_t Item::propertyCount() const
{
    return (uint32_t) mProps.size();
}

bool Item::isEssential(uint32_t aId) const
{
    if (aId < mProps.size())
    {
        return mProps[aId].second;
    }
    return false;
}
bool Item::isEssential(ItemProperty* aProperty) const
{
    // find matching property from list
    for (const auto& it : mProps)
    {
        if (it.first == aProperty)
        {
            return it.second;
        }
    }
    return false;
}
void Item::setEssential(uint32_t aId, bool aEssential)
{
    if (aId < mProps.size())
    {
        mProps[aId].second = aEssential;
    }
}
void Item::setEssential(ItemProperty* aProperty, bool aEssential)
{
    // find matching property from list
    for (auto& it : mProps)
    {
        if (it.first == aProperty)
        {
            it.second = aEssential;
        }
    }
}

ItemProperty* Item::getProperty(uint32_t aId)
{
    if (aId < mProps.size())
    {
        return mProps[aId].first;
    }
    return nullptr;
}
const ItemProperty* Item::getProperty(uint32_t aId) const
{
    if (aId < mProps.size())
    {
        return mProps[aId].first;
    }
    return nullptr;
}

void Item::addProperty(ItemProperty* aProp, bool aEssential)
{
    if (aProp == nullptr)
    {
        return;
    }
    auto type        = aProp->getType();
    bool invalidProp = false;
    switch (type)
    {
    case HEIF::ItemPropertyType::INVALID:
    {
        return;
    }
    case HEIF::ItemPropertyType::ISPE:  ///< Image spatial extents
    case HEIF::ItemPropertyType::JPGC:  ///< JPEG configuration
    case HEIF::ItemPropertyType::AVCC:  ///< AVC configuration
    case HEIF::ItemPropertyType::HVCC:  ///< HEVC configuration
    {
        // These properties should not be set with addProperty.
        invalidProp = true;
        break;
    }

    case HEIF::ItemPropertyType::RAW:
    case HEIF::ItemPropertyType::AUXC:
    case HEIF::ItemPropertyType::CLAP:
    case HEIF::ItemPropertyType::COLR:
    case HEIF::ItemPropertyType::IMIR:
    case HEIF::ItemPropertyType::IROT:
    case HEIF::ItemPropertyType::PASP:
    case HEIF::ItemPropertyType::PIXI:
    case HEIF::ItemPropertyType::RLOC:
    default:
    {
        // Allow all other property types.
        invalidProp = false;
        break;
    }
    }

    for (auto it = mProps.begin(); it != mProps.end(); it++)
    {
        if (it->first == aProp)
        {
            // This property has already been associated with this image.
            invalidProp = true;
            break;
        }
        if (it->first->getType() == type)
        {
            bool end = true;
            // yes it exists already.
            switch (type)
            {
            case HEIF::ItemPropertyType::AUXC:  ///< Image properties for auxiliary images
            case HEIF::ItemPropertyType::CLAP:  ///< Clean aperture (crop)
            case HEIF::ItemPropertyType::COLR:  ///< Colour information
            case HEIF::ItemPropertyType::IMIR:  ///< Image mirror
            case HEIF::ItemPropertyType::IROT:  ///< Image rotation
            case HEIF::ItemPropertyType::PASP:  ///< Pixel aspect ratio
            case HEIF::ItemPropertyType::PIXI:  ///< Pixel information
            case HEIF::ItemPropertyType::RLOC:  ///< Relative location
            {
                //Allow only one.
                invalidProp = true;
                break;
            }
            case HEIF::ItemPropertyType::RAW:  ///< Property of an unrecognized/unknown type. It is accessible only as raw data.
            {
                //Low level reader does not currently support these types and returns them as raw props.
                //Although 'lhv1' is not a valid image either.
                RawProperty* raw  = static_cast<RawProperty*>(it->first);
                RawProperty* raw2 = static_cast<RawProperty*>(aProp);
                if (raw->rawType() == raw2->rawType())
                {
                    if ((raw->rawType() == "lsel") ||  //Layer selection                               (zero or one)
                        (raw->rawType() == "lhvC") ||  // Layered HEVC configuration item property     ('lhv1' items only, MUST have one)
                        (raw->rawType() == "oinf") ||  // Operating points information property        ('lhv1' items only, MUST have one)
                        (raw->rawType() == "tols"))    // Target output layer set property             ('lhv1' items only, MUST have one)
                    {
                        //allow only one.
                        invalidProp = true;
                    }
                    else if (raw->rawType() == "subs")  // Sub-sample item property                    (zero or more for HEVC item and 'avc1'.)
                    {
                        // Zero or more.
                    }
                    else
                    {
                        //really unknown. so allow multiples.
                    }
                }
                else
                {
                    end = false;
                }
                break;
            }

            case HEIF::ItemPropertyType::AVCC:
            case HEIF::ItemPropertyType::HVCC:
            case HEIF::ItemPropertyType::JPGC:
            case HEIF::ItemPropertyType::ISPE:
            case HEIF::ItemPropertyType::INVALID:
            {
                return;
            }
            default:
            {
                // Okay.
                invalidProp = false;
                break;
            }
            }
            if (end)
            {
                break;
            }
        }
    }
    if (invalidProp)
    {
        return;
    }

    // check type.
    if (aProp->isTransformative())
    {
        if (mTransformCount == 0)
            mFirstTransform = (uint32_t) mProps.size();
        mProps.push_back({aProp, aEssential});
        mTransformCount++;
    }
    else
    {
        // descriptive properties
        if (mTransformCount == 0)
        {
            // No transforms, so add to end
            mProps.push_back({aProp, aEssential});
        }
        else
        {
            // Insert before first transform.
            mProps.insert(mProps.begin() + (int64_t) mFirstTransform, {aProp, aEssential});
            mFirstTransform++;
        }
    }
    aProp->link(this);
}
