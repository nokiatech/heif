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

#include "ImageItem.h"
#include <heifreader.h>
#include <heifwriter.h>
#include "CodedImageItem.h"
#include "DerivedImageItem.h"
#include "DescriptiveProperty.h"
#include "GridImageItem.h"
#include "IdentityImageItem.h"
#include "MetaItem.h"
#include "OverlayImageItem.h"
#include "RawProperty.h"
#include "TransformativeProperty.h"

using namespace HEIFPP;

ImageItem::ImageItem(Heif* aHeif, const HEIF::FourCC& aType, bool aDerived, bool aCoded)
    : Item(aHeif, aType, true)
    , mIsHidden(false)
    , mIsDerived(aDerived)
    , mIsCoded(aCoded)
    , mWidth(0)
    , mHeight(0)
{
}
ImageItem::~ImageItem()
{
    // disconnect all links "to".
    // Disconnect all metadatas
    for (; !mMetaItems.empty();)
    {
        MetaItem* meta = mMetaItems[0];
        if (meta)
        {
            removeMetadata(meta);
        }
    }

    // Disconnect all thumbnails
    for (; !mThumbnailImages.empty();)
    {
        ImageItem* image = mThumbnailImages[0];
        if (image)
        {
            removeThumbnail(image);
        }
    }
    // Disconnect all auxiliary images
    for (; !mAuxImages.empty();)
    {
        ImageItem* image = mAuxImages[0];
        if (image)
        {
            removeAuxImage(image);
        }
    }

    // disconnect all links "from".
    // Remove this(thumbnail) from all master images.
    for (; !mIsThumbnailTo.empty();)
    {
        std::pair<ImageItem*, std::uint32_t>& p = mIsThumbnailTo[0];
        ImageItem* image                        = p.first;
        if (image)
        {
            image->removeThumbnail(this);
        }
    }
    // Remove this(auxiliary) from all master images.
    for (; !mIsAuxiliaryTo.empty();)
    {
        std::pair<ImageItem*, std::uint32_t>& p = mIsAuxiliaryTo[0];
        ImageItem* image                        = p.first;
        if (image)
        {
            image->removeAuxImage(this);
        }
    }
    // Remove this(base) from all master images.
    for (; !mIsBaseImageTo.empty();)
    {
        std::pair<CodedImageItem*, std::uint32_t>& p = mIsBaseImageTo[0];
        CodedImageItem* image                        = p.first;
        if (image)
        {
            image->removeBaseImage(this);
        }
    }
    // Remove this(source) from all master images.
    for (; !mIsSourceImageTo.empty();)
    {
        std::pair<DerivedImageItem*, std::uint32_t>& p = mIsSourceImageTo[0];
        DerivedImageItem* image                        = p.first;

        if (image)
        {
            image->removeImage(this);
        }
    }
}

ItemProperty* ImageItem::getFirstPropertyOfType(HEIF::ItemPropertyType aType) const
{
    for (auto it = mProps.begin(); it != mProps.end(); it++)
    {
        if (it->first->getType() == aType)
        {
            return (it->first);
        }
    }
    return nullptr;
}

PixelAspectRatioProperty* ImageItem::pixelAspectRatio()
{
    return static_cast<PixelAspectRatioProperty*>(getFirstPropertyOfType(HEIF::ItemPropertyType::PASP));
}
ColourInformationProperty* ImageItem::colourInformation()
{
    return static_cast<ColourInformationProperty*>(getFirstPropertyOfType(HEIF::ItemPropertyType::COLR));
}
PixelInformationProperty* ImageItem::pixelInformation()
{
    return static_cast<PixelInformationProperty*>(getFirstPropertyOfType(HEIF::ItemPropertyType::PIXI));
}
RelativeLocationProperty* ImageItem::relativeLocation()
{
    return static_cast<RelativeLocationProperty*>(getFirstPropertyOfType(HEIF::ItemPropertyType::RLOC));
}
AuxiliaryProperty* ImageItem::aux()
{
    return static_cast<AuxiliaryProperty*>(getFirstPropertyOfType(HEIF::ItemPropertyType::AUXC));
}

const PixelAspectRatioProperty* ImageItem::pixelAspectRatio() const
{
    return static_cast<const PixelAspectRatioProperty*>(getFirstPropertyOfType(HEIF::ItemPropertyType::PASP));
}
const ColourInformationProperty* ImageItem::colourInformation() const
{
    return static_cast<const ColourInformationProperty*>(getFirstPropertyOfType(HEIF::ItemPropertyType::COLR));
}
const PixelInformationProperty* ImageItem::pixelInformation() const
{
    return static_cast<const PixelInformationProperty*>(getFirstPropertyOfType(HEIF::ItemPropertyType::PIXI));
}
const RelativeLocationProperty* ImageItem::relativeLocation() const
{
    return static_cast<const RelativeLocationProperty*>(getFirstPropertyOfType(HEIF::ItemPropertyType::RLOC));
}
const AuxiliaryProperty* ImageItem::aux() const
{
    return static_cast<const AuxiliaryProperty*>(getFirstPropertyOfType(HEIF::ItemPropertyType::AUXC));
}

void ImageItem::setSize(std::uint32_t aWidth, std::uint32_t aHeight)
{
    mWidth  = aWidth;
    mHeight = aHeight;
}
std::uint32_t ImageItem::width() const
{
    return mWidth;
}

std::uint32_t ImageItem::height() const
{
    return mHeight;
}

std::uint32_t ImageItem::transformativePropertyCount() const
{
    return mTransformCount;
}
TransformativeProperty* ImageItem::getTransformativeProperty(std::uint32_t aId)
{
    std::uint32_t index = 0;
    auto it             = mProps.begin();
    for (; it != mProps.end(); it++)
    {
        if (it->first->isTransformative())
        {
            if (aId == index)
            {
                return static_cast<TransformativeProperty*>(it->first);
            }
            index++;
        }
    }
    return nullptr;
}
const TransformativeProperty* ImageItem::getTransformativeProperty(std::uint32_t aId) const
{
    std::uint32_t index = 0;
    auto it             = mProps.begin();
    for (; it != mProps.end(); it++)
    {
        if (it->first->isTransformative())
        {
            if (aId == index)
            {
                return static_cast<const TransformativeProperty*>(it->first);
            }
            index++;
        }
    }
    return nullptr;
}

void ImageItem::setHidden(bool aHidden)
{
    mIsHidden = aHidden;
}
bool ImageItem::isHidden() const
{
    return mIsHidden;
}
bool ImageItem::isThumbnailImage() const
{
    return mIsThumbnailTo.size() > 0;
}
bool ImageItem::isAuxiliaryImage() const
{
    return mIsAuxiliaryTo.size() > 0;
}
bool ImageItem::isMasterImage() const
{
    return !(isThumbnailImage() || isAuxiliaryImage());
}
bool ImageItem::isSourceImage() const
{
    return mIsSourceImageTo.size() > 0;
}
bool ImageItem::isDerivedImage() const
{
    return mIsDerived;
}

bool ImageItem::isBaseImage() const
{
    return mIsBaseImageTo.size() > 0;
}
bool ImageItem::isPreDerivedImage() const
{
    if (isCodedImage())
    {
        const CodedImageItem* img = static_cast<const CodedImageItem*>(this);
        if (img->getBaseImageCount() > 0)
            return true;
    }
    return false;
}

bool ImageItem::isPrimaryImage() const
{
    return (mHeif->getPrimaryItem() == this);
}

bool ImageItem::isCoverImage() const
{
    // based on how the low-level api creates the flags.
    return isPrimaryImage();
}

bool ImageItem::HasLinkedThumbnails() const
{
    return (getThumbnailCount() > 0);
}
bool ImageItem::hasLinkedAuxiliaryImage() const
{
    return (getAuxCount() > 0);
}
bool ImageItem::hasLinkedDerivedImage() const
{
    return isSourceImage();
}
bool ImageItem::hasLinkedPreComputedDerivedImage() const
{
    return isBaseImage();
}
/*
bool ImageItem::hasLinkedTiles() const
{
    return getTileCount()>0;
}
*/
bool ImageItem::hasLinkedMetadata() const
{
    return (getMetadataCount() > 0);
}

bool ImageItem::isCodedImage() const
{
    return mIsCoded;
}
HEIF::ErrorCode ImageItem::save(HEIF::Writer* aWriter)
{
    HEIF::ErrorCode error;
    error = aWriter->setImageHidden(mId, isHidden());
    if (HEIF::ErrorCode::OK != error)
        return error;

    // save metadata.
    for (MetaItem* item : mMetaItems)
    {
        if (item == nullptr)
        {
            return HEIF::ErrorCode::INVALID_ITEM_ID;
        }
        if (item->getId() == Heif::InvalidItem)
        {
            error = item->save(aWriter);
            if (HEIF::ErrorCode::OK != error)
                return error;
        }
        error = aWriter->addMetadataItemReference(item->getId().get(), mId);
        if (HEIF::ErrorCode::OK != error)
            return error;
    }

    // save all thumbs..
    for (ImageItem* image : mThumbnailImages)
    {
        if (image == nullptr)
        {
            return HEIF::ErrorCode::INVALID_ITEM_ID;
        }
        if (image->getId() == Heif::InvalidItem)
        {
            error = image->save(aWriter);
            if (HEIF::ErrorCode::OK != error)
                return error;
        }
        error = aWriter->addThumbnail(image->getId(), mId);
        if (HEIF::ErrorCode::OK != error)
            return error;
    }

    // save all auxiliarys..
    for (ImageItem* image : mAuxImages)
    {
        if (image == nullptr)
        {
            return HEIF::ErrorCode::INVALID_ITEM_ID;
        }
        if (image->getId() == Heif::InvalidItem)
        {
            error = image->save(aWriter);
            if (HEIF::ErrorCode::OK != error)
                return error;
        }
        error = aWriter->addAuxiliaryReference(image->getId(), mId);
        if (HEIF::ErrorCode::OK != error)
            return error;
    }

    return Item::save(aWriter);
}

HEIF::ErrorCode ImageItem::load(HEIF::Reader* aReader, const HEIF::ImageId& aId)
{
    HEIF::ErrorCode error;
    error = Item::load(aReader, aId);
    if (HEIF::ErrorCode::OK != error)
        return error;
    error = aReader->getWidth(aId, mWidth);
    if (HEIF::ErrorCode::OK != error)
        return error;
    error = aReader->getHeight(aId, mHeight);
    if (HEIF::ErrorCode::OK != error)
        return error;

    HEIF::Array<HEIF::ImageId> ids;
    {
        const auto* i = mHeif->getItemInformation(aId);
        HEIF_ASSERT(i);  // ImageItems MUST have this.
        mIsHidden = (i->features & HEIF::ItemFeatureEnum::IsHiddenImage) != 0;

        error = aReader->getReferencedToItemListByType(aId, "thmb", ids);
        if (HEIF::ErrorCode::OK != error)
            return error;
        if (ids.size > 0)
        {
            mThumbnailImages.reserve(ids.size);
            for (const auto& thumbId : ids)
            {
                ImageItem* image = mHeif->constructImageItem(aReader, thumbId, error);
                if (HEIF::ErrorCode::OK != error)
                {
                    return error;
                }
                addThumbnail(image);
            }
        }

        error = aReader->getReferencedToItemListByType(aId, "auxl", ids);
        if (HEIF::ErrorCode::OK != error)
            return error;
        if (ids.size > 0)
        {
            mAuxImages.reserve(ids.size);
            for (const auto& auxId : ids)
            {
                ImageItem* image = mHeif->constructImageItem(aReader, auxId, error);
                if (HEIF::ErrorCode::OK != error)
                {
                    return error;
                }
                addAuxImage(image);
            }
        }

        error = aReader->getReferencedToItemListByType(aId, "cdsc", ids);
        if (HEIF::ErrorCode::OK != error)
            return error;
        if (ids.size > 0)
        {
            mMetaItems.reserve(ids.size);
            for (const auto& metaId : ids)
            {
                MetaItem* meta = mHeif->constructMetaItem(aReader, metaId, error);
                if (HEIF::ErrorCode::OK != error)
                {
                    return error;
                }
                addMetadata(meta);
            }
        }
    }
    return HEIF::ErrorCode::OK;
}

std::uint32_t ImageItem::getThumbnailCount() const
{
    return (uint32_t) mThumbnailImages.size();
}
ImageItem* ImageItem::getThumbnail(uint32_t aId)
{
    if (aId < mThumbnailImages.size())
    {
        return mThumbnailImages[aId];
    }
    return nullptr;
}
const ImageItem* ImageItem::getThumbnail(uint32_t aId) const
{
    if (aId < mThumbnailImages.size())
    {
        return mThumbnailImages[aId];
    }
    return nullptr;
}
void ImageItem::addThumbnail(ImageItem* aImage)
{
    if (aImage)
    {
        aImage->mIsThumbnailTo.addLink(this);
        mThumbnailImages.push_back(aImage);
    }
}
void ImageItem::removeThumbnail(ImageItem* aImage)
{
    for (auto it = mThumbnailImages.begin(); it != mThumbnailImages.end();)
    {
        if (aImage == (*it))
        {
            if (aImage)
            {
                if (!aImage->mIsThumbnailTo.removeLink(this))
                {
                    HEIF_ASSERT(false);
                }
            }
            it = mThumbnailImages.erase(it);
        }
        else
        {
            ++it;
        }
    }
}


std::uint32_t ImageItem::getAuxCount() const
{
    return (uint32_t) mAuxImages.size();
}
ImageItem* ImageItem::getAux(uint32_t aId)
{
    if (aId < mAuxImages.size())
    {
        return mAuxImages[aId];
    }
    return nullptr;
}
const ImageItem* ImageItem::getAux(uint32_t aId) const
{
    if (aId < mAuxImages.size())
    {
        return mAuxImages[aId];
    }
    return nullptr;
}
void ImageItem::addAuxImage(ImageItem* aImage)
{
    if (aImage)
    {
        aImage->mIsAuxiliaryTo.addLink(this);
        mAuxImages.push_back(aImage);
    }
}
void ImageItem::removeAuxImage(ImageItem* aImage)
{
    for (auto it = mAuxImages.begin(); it != mAuxImages.end();)
    {
        if (aImage == (*it))
        {
            if (aImage)
            {
                if (!aImage->mIsAuxiliaryTo.removeLink(this))
                {
                    HEIF_ASSERT(false);
                }
            }
            it = mAuxImages.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

std::uint32_t ImageItem::getMetadataCount() const
{
    return (uint32_t) mMetaItems.size();
}
MetaItem* ImageItem::getMetadata(uint32_t aId)
{
    if (aId < mMetaItems.size())
    {
        return mMetaItems[aId];
    }
    return nullptr;
}
const MetaItem* ImageItem::getMetadata(uint32_t aId) const
{
    if (aId < mMetaItems.size())
    {
        return mMetaItems[aId];
    }
    return nullptr;
}
void ImageItem::addMetadata(MetaItem* aItem)
{
    if (aItem == nullptr)
        return;

    if (AddItemTo(mMetaItems, aItem))
    {
        aItem->link(this);
    }
}
void ImageItem::removeMetadata(MetaItem* aItem)
{
    if (aItem == nullptr)
        return;
    if (RemoveItemFrom(mMetaItems, aItem))
    {
        aItem->unlink(this);
    }
}


void ImageItem::addSourceLink(ImageItem* aSource, DerivedImageItem* aTarget)
{
    HEIF_ASSERT(aSource);
    HEIF_ASSERT(aTarget);
    aSource->mIsSourceImageTo.addLink(aTarget);
}
bool ImageItem::removeSourceLink(ImageItem* aSource, DerivedImageItem* aTarget)
{
    HEIF_ASSERT(aSource);
    HEIF_ASSERT(aTarget);
    return aSource->mIsSourceImageTo.removeLink(aTarget);
}
void ImageItem::addBaseLink(ImageItem* aSource, CodedImageItem* aTarget)
{
    HEIF_ASSERT(aSource);
    HEIF_ASSERT(aTarget);
    aSource->mIsBaseImageTo.addLink(aTarget);
}
bool ImageItem::removeBaseLink(ImageItem* aSource, CodedImageItem* aTarget)
{
    HEIF_ASSERT(aSource);
    HEIF_ASSERT(aTarget);
    return aSource->mIsBaseImageTo.removeLink(aTarget);
}
