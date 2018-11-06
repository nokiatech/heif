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

#include <heifreader.h>
#include <heifwriter.h>
#include "EntityGroup.h"
#include "Item.h"
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

    // disconnect from groups
    for (; !mGroups.empty();)
    {
        (*mGroups.begin())->removeItem(this);
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
const std::string& Item::getName() const
{
    return mName;
}
void Item::setName(const std::string& aName)
{
    mName = aName;
}
const std::string& Item::getContentType() const
{
    return mContentType;
}
HEIFPP::Result Item::setContentType(const std::string& aType)
{
    if (mContentType.empty())
    {
        mContentType = aType;
    }
    else
    {
        if (mContentType != aType)
        {
            //Error.
            return HEIFPP::Result::ALREADY_SET;
        }
    }
    return HEIFPP::Result::OK;
}
const std::string& Item::getContentEncoding() const
{
    return mContentEncoding;
}
HEIFPP::Result Item::setContentEncoding(const std::string& aType)
{
    if (mContentEncoding.empty())
    {
        mContentEncoding = aType;
    }
    return HEIFPP::Result::OK;
}

HEIF::ErrorCode Item::load(HEIF::Reader* aReader, const HEIF::ImageId& aId)
{
    HEIF::ErrorCode error = HEIF::ErrorCode::OK;

    HEIF::FourCC type;
    mId   = aId;
    error = aReader->getItemType(aId, type);
    if (HEIF::ErrorCode::OK != error)
        return error;
    HEIF_ASSERT(mType == type);
    const auto* i = mHeif->getItemInformation(aId);
    if (i)
    {
        mIsProtected = (i->features & HEIF::ItemFeatureEnum::IsProtected) != 0;
    }
    else
    {
        return HEIF::ErrorCode::MEDIA_PARSING_ERROR;
    }

    mName            = std::string(i->description.name.begin(), i->description.name.end());
    mContentType     = std::string(i->description.contentType.begin(), i->description.contentType.end());
    mContentEncoding = std::string(i->description.contentEncoding.begin(), i->description.contentEncoding.end());


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
    HEIF::ItemDescription d;
    if (mName.size())
    {
        d.name = HEIF::Array<char>(mName.size());
        for (size_t i = 0; i < d.name.size; i++)
        {
            d.name.elements[i] = mName[i];
        }
    }
    if (mContentType.size())
    {
        d.contentType = HEIF::Array<char>(mContentType.size());
        for (size_t i = 0; i < d.contentType.size; i++)
        {
            d.contentType.elements[i] = mContentType[i];
        }
    }
    if (mContentEncoding.size())
    {
        d.contentEncoding = HEIF::Array<char>(mContentEncoding.size());
        for (size_t i = 0; i < d.contentEncoding.size; i++)
        {
            d.contentEncoding.elements[i] = mContentEncoding[i];
        }
    }

    error = aWriter->setItemDescription(mId, d);
    return error;
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
std::uint32_t Item::propertyCount() const
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
bool Item::isEssential(const ItemProperty* aProperty) const
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
void Item::setEssential(const ItemProperty* aProperty, bool aEssential)
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
                // Allow only one.
                invalidProp = true;
                break;
            }
            case HEIF::ItemPropertyType::RAW:  ///< Property of an unrecognized/unknown type. It is accessible only as
                                               ///< raw data.
            {
                // Low level reader does not currently support these types and returns them as raw props.
                // Although 'lhv1' is not a valid image either.
                const RawProperty* raw  = static_cast<const RawProperty*>(it->first);
                const RawProperty* raw2 = static_cast<const RawProperty*>(aProp);
                if (raw->rawType() == raw2->rawType())
                {
                    if ((raw->rawType() == "lsel") ||  // Layer selection          (zero or one)
                        (raw->rawType() ==
                         "lhvC") ||  // Layered HEVC configuration item property   ('lhv1' items only, MUST have one)
                        (raw->rawType() ==
                         "oinf") ||  // Operating points information property      ('lhv1' items only, MUST have one)
                        (raw->rawType() ==
                         "tols"))  // Target output layer set property             ('lhv1' items only, MUST have one)
                    {
                        // allow only one.
                        invalidProp = true;
                    }
                    else if (raw->rawType() ==
                             "subs")  // Sub-sample item property (zero or more for HEVC item and 'avc1'.)
                    {
                        // Zero or more.
                    }
                    else
                    {
                        // really unknown. so allow multiples.
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
    // keep list ordered, descriptives first then transformatives.
    if (aProp->isTransformative())
    {
        mProps.push_back({aProp, aEssential});
        mTransformCount++;
    }
    else
    {
        // find first transform, insert before it
        auto it = mProps.begin();
        for (; it != mProps.end(); it++)
        {
            if (it->first->isTransformative())
            {
                break;
            }
        }
        mProps.insert(it, {aProp, aEssential});
    }
    aProp->link(this);
}


void Item::addToGroup(EntityGroup* aGroup)
{
    AddItemTo(mGroups, aGroup);
}
void Item::removeFromGroup(EntityGroup* aGroup)
{
    RemoveItemFrom(mGroups, aGroup);
}
std::uint32_t Item::getGroupCount() const
{
    return (std::uint32_t) mGroups.size();
}
EntityGroup* Item::getGroup(uint32_t aId)
{
    if (aId < mGroups.size())
    {
        return mGroups[aId];
    }
    return nullptr;
}
std::uint32_t Item::getGroupByTypeCount(const HEIF::FourCC& aType)
{
    std::uint32_t cnt = 0;
    for (auto grp : mGroups)
    {
        if (grp->getType() == aType)
        {
            cnt++;
        }
    }
    return cnt;
}
EntityGroup* Item::getGroupByType(const HEIF::FourCC& aType, std::uint32_t aId)
{
    std::uint32_t cnt = 0;
    for (auto grp : mGroups)
    {
        if (grp->getType() == aType)
        {
            if (aId == cnt)
                return grp;
            cnt++;
        }
    }
    return nullptr;
}
EntityGroup* Item::getGroupById(const HEIF::GroupId& aId)
{
    for (auto grp : mGroups)
    {
        if (grp->getId() == aId)
        {
            return grp;
        }
    }
    return nullptr;
}
