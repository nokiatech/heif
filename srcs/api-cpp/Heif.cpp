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

#include "Heif.h"
#include <heifreader.h>
#include <heifstreaminterface.h>
#include <heifwriter.h>
#include "AVCCodedImageItem.h"
#include "DescriptiveProperty.h"
#include "ExifItem.h"
#include "GridImageItem.h"
#include "HEVCCodedImageItem.h"
#include "IdentityImageItem.h"
#include "Item.h"
#include "JPEGCodedImageItem.h"
#include "MPEG7Item.h"
#include "OverlayImageItem.h"
#include "RawProperty.h"
#include "TransformativeProperty.h"
#include "XMPItem.h"

using namespace HEIFPP;
const HEIF::ImageId Heif::InvalidItem((uint32_t) 0);
const HEIF::PropertyId Heif::InvalidProperty((uint32_t) 0);
const HEIF::DecoderConfigId Heif::InvalidDecoderConfig((uint32_t) 0);
const HEIF::MediaDataId Heif::InvalidMediaData((uint32_t) 0);
#define ISPE_AS_RAW_PROPERTY 0
#define DECODER_CONFIG_AS_RAW_PROPERTY 0


Heif::Heif()
    : mFileinfo{}
    , mMajorBrand()
    , mMinorVersion(0)
    , mCompatibleBrands()
    , mItems()
    , mItemsOfType()
    , mProperties()
    , mDecoderConfigs()
    , mPrimaryItem(nullptr)
    , mItemsLoad()
    , mPropertiesLoad()
    , mDecoderConfigsLoad()
    , mContext(nullptr)
{
}

Heif::~Heif()
{
    reset();
}

void Heif::reset()
{
    for (; !mItems.empty();)
    {
        auto it = mItems.begin();
        delete (*it);
    }
    for (; !mProperties.empty();)
    {
        auto it = mProperties.begin();
        delete (*it);
    }
    for (; !mDecoderConfigs.empty();)
    {
        auto it = mDecoderConfigs.begin();
        delete (*it);
    }
    mCompatibleBrands.clear();
    mMajorBrand  = HEIF::FourCC();
    mPrimaryItem = nullptr;
    mFileinfo    = HEIF::FileInformation();
}

/** Custom user data can be bound to objects. */
void Heif::setContext(const void* aContext)
{
    mContext = aContext;
}

const void* Heif::getContext() const
{
    return mContext;
}

Result Heif::save(const char* fileName)
{
    HEIF::Writer* writer;
    if (mMajorBrand == HEIF::FourCC())
        return Result::BRANDS_NOT_SET;
    if (mPrimaryItem == nullptr)
        return Result::PRIMARY_ITEM_NOT_SET;
    if (mPrimaryItem->isHidden())
        return Result::HIDDEN_PRIMARY_ITEM;

    HEIF::ErrorCode error;
    writer = HEIF::Writer::Create();
    HEIF::OutputConfig output;
    output.fileName         = fileName;
    output.majorBrand       = mMajorBrand;
    output.compatibleBrands = HEIF::Array<HEIF::FourCC>(mCompatibleBrands.size());
    output.progressiveFile  = false;
    for (uint32_t i = 0; i < mCompatibleBrands.size(); i++)
    {
        output.compatibleBrands[i] = mCompatibleBrands[i];
    }
    error = writer->initialize(output);
    if (HEIF::ErrorCode::OK == error)
    {
        // Invalidate all id's since writer will create new ones.
        for (auto* item : mItems)
        {
            // set an "invalid" id.
            item->setId(InvalidItem);
        }
        for (auto* property : mProperties)
        {
            // set an "invalid" id.
            property->setId(InvalidProperty);
        }
        for (auto* config : mDecoderConfigs)
        {
            // set an "invalid" id.
            config->setId(InvalidDecoderConfig);
        }
        for (auto* item : mItems)
        {
            if (item->getId() == Heif::InvalidItem)
            {
                error = item->save(writer);
                if (HEIF::ErrorCode::OK != error)
                {
                    break;
                }
            }
        }
        if (HEIF::ErrorCode::OK == error)
        {
            error = writer->setPrimaryItem(mPrimaryItem->getId());
        }

        HEIF::ErrorCode finalizeError = writer->finalize();

        if (HEIF::ErrorCode::OK == error)
        {
            error = finalizeError;
        }
    }
    HEIF::Writer::Destroy(writer);
    return convertErrorCode(error);
}
const HEIF::ItemInformation* Heif::getItemInformation(const HEIF::ImageId& aItemId) const
{
    for (const auto& i : mFileinfo.rootMetaBoxInformation.itemInformations)
    {
        if (i.itemId == aItemId)
        {
            return &i;
        }
    }
    return nullptr;
}
const HEIF::ImageInformation* Heif::getImageInformation(const HEIF::ImageId& aItemId) const
{
    for (const auto& i : mFileinfo.rootMetaBoxInformation.imageInformations)
    {
        if (i.itemId == aItemId)
        {
            return &i;
        }
    }
    return nullptr;
}
Result Heif::load(const char* fileName)
{
    HEIF::Reader* reader;
    if (mMajorBrand != HEIF::FourCC())
        return Result::ALREADY_INITIALIZED;
    if (!mCompatibleBrands.empty())
        return Result::ALREADY_INITIALIZED;
    if (!mItems.empty())
        return Result::ALREADY_INITIALIZED;
    if (!mItemsOfType.empty())
        return Result::ALREADY_INITIALIZED;
    if (!mProperties.empty())
        return Result::ALREADY_INITIALIZED;
    if (!mDecoderConfigs.empty())
        return Result::ALREADY_INITIALIZED;
    if (mPrimaryItem)
        return Result::ALREADY_INITIALIZED;

    reader                = HEIF::Reader::Create();
    HEIF::ErrorCode error = reader->initialize(fileName);
    if (HEIF::ErrorCode::OK == error)
    {
        error = load(reader);
    }
    mItemsLoad.clear();
    mPropertiesLoad.clear();
    mDecoderConfigsLoad.clear();
    if (HEIF::ErrorCode::OK != error)
    {
        // clean up on error.
        reset();
    }

    // TODO: leave the reader object alive, and on-demand read item datas...
    HEIF::Reader::Destroy(reader);
    reader = nullptr;

    return convertErrorCode(error);
}

Result Heif::load(HEIF::StreamInterface* aStream)
{
    HEIF::Reader* reader;
    if (mMajorBrand != HEIF::FourCC())
        return Result::ALREADY_INITIALIZED;
    if (!mCompatibleBrands.empty())
        return Result::ALREADY_INITIALIZED;
    if (!mItems.empty())
        return Result::ALREADY_INITIALIZED;
    if (!mItemsOfType.empty())
        return Result::ALREADY_INITIALIZED;
    if (!mProperties.empty())
        return Result::ALREADY_INITIALIZED;
    if (!mDecoderConfigs.empty())
        return Result::ALREADY_INITIALIZED;
    if (mPrimaryItem)
        return Result::ALREADY_INITIALIZED;

    reader                = HEIF::Reader::Create();
    HEIF::ErrorCode error = reader->initialize(aStream);
    if (HEIF::ErrorCode::OK == error)
    {
        error = load(reader);
    }
    mItemsLoad.clear();
    mPropertiesLoad.clear();
    mDecoderConfigsLoad.clear();
    if (HEIF::ErrorCode::OK != error)
    {
        // clean up on error.
        reset();
    }

    // TODO: leave the reader object alive, and on-demand read item datas...
    HEIF::Reader::Destroy(reader);
    reader = nullptr;


    return convertErrorCode(error);
}

HEIF::ErrorCode Heif::load(HEIF::Reader* aReader)
{
    HEIF::ErrorCode error;
    error = aReader->getMajorBrand(mMajorBrand);
    if (HEIF::ErrorCode::OK == error)
    {
        error = aReader->getMinorVersion(mMinorVersion);
        if (HEIF::ErrorCode::OK == error)
        {
            HEIF::Array<HEIF::FourCC> brands;
            error = aReader->getCompatibleBrands(brands);
            if (HEIF::ErrorCode::OK == error)
            {
                mCompatibleBrands.reserve(brands.size);
                for (const auto& b : brands)
                {
                    addCompatibleBrand(b);
                }
                error = aReader->getFileInformation(mFileinfo);
                if (HEIF::ErrorCode::OK == error)
                {
                    HEIF::ImageId prim = 0;  // should be invalid?
                    error              = aReader->getPrimaryItem(prim);
                    if (HEIF::ErrorCode::OK != error)
                    {
                        return error;
                    }

                    for (const auto& i : mFileinfo.rootMetaBoxInformation.imageInformations)
                    {
                        ImageItem* image = static_cast<ImageItem*>(constructItem(aReader, i.itemId, error));
                        if (HEIF::ErrorCode::OK != error)
                        {
                            return error;
                        }
                        if (i.itemId == prim)
                        {
                            setPrimaryItem(image);
                        }
                    }
                    for (const auto& i : mFileinfo.rootMetaBoxInformation.itemInformations)
                    {
                        constructItem(aReader, i.itemId, error);
                        if (HEIF::ErrorCode::OK != error)
                        {
                            return error;
                        }
                    }
                }
            }
        }
    }
    return error;
}
const HEIF::FourCC& Heif::getMajorBrand() const
{
    return mMajorBrand;
}
void Heif::setMajorBrand(const HEIF::FourCC& aBrand)
{
    mMajorBrand = aBrand;
}
uint32_t Heif::getMinorVersion() const
{
    return mMinorVersion;
}
void Heif::setMinorVersion(uint32_t aVersion)
{
    mMinorVersion = aVersion;
}
uint32_t Heif::compatibleBrands() const
{
    return (uint32_t) mCompatibleBrands.size();
}
const HEIF::FourCC& Heif::getCompatibleBrand(uint32_t aId) const
{
    if (aId < mCompatibleBrands.size())
    {
        return mCompatibleBrands[aId];
    }
    static HEIF::FourCC aInvalidFourCC((uint32_t) 0);
    return aInvalidFourCC;
}

void Heif::addCompatibleBrand(const HEIF::FourCC& brand)
{
    if (!AddItemTo(mCompatibleBrands, brand))
    {
        // Tried to add a duplicate compatible brand.
        HEIF_ASSERT(false);
    }
}
void Heif::removeCompatibleBrand(uint32_t aId)
{
    if (aId < mCompatibleBrands.size())
    {
        mCompatibleBrands.erase(mCompatibleBrands.begin() + (int32_t) aId);
    }
}
void Heif::removeCompatibleBrand(const HEIF::FourCC& brand)
{
    if (!RemoveItemFrom(mCompatibleBrands, brand))
    {
        // tried to remove brand that was not added.
        HEIF_ASSERT(false);
    }
}

Item* Heif::constructItem(HEIF::Reader* aReader, const HEIF::ImageId& aItemId, HEIF::ErrorCode& aErrorCode)
{
    auto it = mItemsLoad.find(aItemId);
    if (it == mItemsLoad.end())
    {
        HEIF::FourCC type;
        aErrorCode = aReader->getItemType(aItemId, type);
        if (HEIF::ErrorCode::OK != aErrorCode)
        {
            return NULL;
        }
        Item* item = nullptr;
        if (type == HEIF::FourCC("avc1"))
        {
            item = new AVCCodedImageItem(this);
        }
        else if (type == HEIF::FourCC("hvc1"))
        {
            item = new HEVCCodedImageItem(this);
        }
        else if (type == HEIF::FourCC("jpeg"))
        {
            item = new JPEGCodedImageItem(this);
        }
        else if (type == HEIF::FourCC("iden"))
        {
            item = new Identity(this);
        }
        else if (type == HEIF::FourCC("iovl"))
        {
            item = new Overlay(this);
        }
        else if (type == HEIF::FourCC("grid"))
        {
            item = new Grid(this);
        }
        else if (type == HEIF::FourCC("Exif"))
        {
            item = new ExifItem(this);
        }
        else if (type == HEIF::FourCC("mime"))
        {
            const HEIF::ItemInformation* info = getItemInformation(aItemId);
            // TODO: when reader exposes the item content-type, use that.
            if (info->features & HEIF::ItemFeatureEnum::Feature::IsXMPItem)
            {
                item = new XMPItem(this);
            }
            else if (info->features & HEIF::ItemFeatureEnum::Feature::IsMPEG7Item)
            {
                // TODO: should actually check the content schema for "urn:mpeg:mpeg7:schema:2001" etcetc.
                item = new MPEG7Item(this);
            }
            else
            {
                // Creates a generic mime item.
                item = new MimeItem(this);
            }
        }
        if (item)
        {
            item->setId(aItemId);
            mItemsLoad[aItemId] = item;
            aErrorCode          = item->load(aReader, aItemId);
            return item;
        }
#ifdef FAIL_ON_UNKNOWN_ITEM
        aErrorCode = HEIF::ErrorCode::MEDIA_PARSING_ERROR;
#endif
        // invalid state.
        return nullptr;
    }
    aErrorCode = HEIF::ErrorCode::OK;
    HEIF_ASSERT(it->second != nullptr);
    return it->second;
}
ItemProperty* Heif::constructItemProperty(HEIF::Reader* aReader,
                                          const HEIF::ItemPropertyInfo& aItemInfo,
                                          HEIF::ErrorCode& aErrorCode)
{
    // method is valid only during load
    auto it = mPropertiesLoad.find(aItemInfo.index);
    if (it == mPropertiesLoad.end())
    {
        ItemProperty* p = nullptr;
        switch (aItemInfo.type)
        {
        case HEIF::ItemPropertyType::CLAP:
        {
            p = new CleanApertureProperty(this);
            break;
        }
        case HEIF::ItemPropertyType::IROT:
        {
            p = new RotateProperty(this);
            break;
        }
        case HEIF::ItemPropertyType::IMIR:
        {
            p = new MirrorProperty(this);
            break;
        }
        case HEIF::ItemPropertyType::PASP:
        {
            p = new PixelAspectRatioProperty(this);
            break;
        }
        case HEIF::ItemPropertyType::COLR:
        {
            p = new ColourInformationProperty(this);
            break;
        }
        case HEIF::ItemPropertyType::PIXI:
        {
            p = new PixelInformationProperty(this);
            break;
        }
        case HEIF::ItemPropertyType::RLOC:
        {
            p = new RelativeLocationProperty(this);
            break;
        }
        case HEIF::ItemPropertyType::AUXC:
        {
            p = new AuxProperty(this);
            break;
        }
        case HEIF::ItemPropertyType::ISPE:
        {
            // not accessible directly, use HEIF::Reader::getWidth/HEIF::Reader::getHeight
#if ISPE_AS_RAW_PROPERTY
            // so construct as RawProperty..
            p = new RawProperty(this);
#else
            // ignore the property
            aErrorCode = HEIF::ErrorCode::OK;
            return nullptr;
#endif
            break;
        }
        case HEIF::ItemPropertyType::HVCC:
        case HEIF::ItemPropertyType::AVCC:
        case HEIF::ItemPropertyType::JPGC:
        {
            // not accessible directly, use HEIF::Reader::getDecoderParameterSets?
            // so construct as raw..
#if DECODER_CONFIG_AS_RAW_PROPERTY
            // so construct as RawProperty..
            p = new RawProperty(this);
#else
            // ignore the property
            aErrorCode = HEIF::ErrorCode::OK;
            return nullptr;
#endif
            break;
        }
        case HEIF::ItemPropertyType::INVALID:
        {
            aErrorCode = HEIF::ErrorCode::MEDIA_PARSING_ERROR;
            return nullptr;
        }
        case HEIF::ItemPropertyType::RAW:
        default:
        {
            // construct as raw..
            p = new RawProperty(this);
            break;
        }
        }
        if (p)
        {
            p->setId(aItemInfo.index);
            mPropertiesLoad[aItemInfo.index] = p;
            aErrorCode                       = p->load(aReader, aItemInfo.index);
            return p;
        }
#ifdef FAIL_ON_UNKNOWN_PROPERTY
        aErrorCode = HEIF::ErrorCode::MEDIA_PARSING_ERROR;
#endif
        return nullptr;
    }
    aErrorCode = HEIF::ErrorCode::OK;
    HEIF_ASSERT(it->second != nullptr);
    return it->second;
}
void Heif::addProperty(ItemProperty* aItemProperty)
{
    aItemProperty->setId(((uint32_t) mProperties.size()) + 1);
    if (!AddItemTo(mProperties, aItemProperty))
    {
        // Tried to add already added property.
        HEIF_ASSERT(false);
    }
}
void Heif::addItem(Item* aItem)
{
    aItem->setId(((uint32_t) mItems.size()) + 1);
    if (!AddItemTo(mItems, aItem))
    {
        // Tried to add an already added item
        HEIF_ASSERT(false);
        return;
    }
    if (!AddItemTo(mItemsOfType[aItem->getType()], aItem))
    {
        // Tried to add an already added item
        HEIF_ASSERT(false);
    }
}

uint32_t Heif::getPropertyCount() const
{
    return (uint32_t) mProperties.size();
}

ItemProperty* Heif::getProperty(uint32_t aIndex)
{
    if (aIndex < mProperties.size())
    {
        return mProperties[aIndex];
    }
    return nullptr;
}
const ItemProperty* Heif::getProperty(uint32_t aIndex) const
{
    if (aIndex < mProperties.size())
    {
        return mProperties[aIndex];
    }
    return nullptr;
}

void Heif::removeProperty(ItemProperty* aProperty)
{
    if (!RemoveItemFrom(mProperties, aProperty))
    {
        // tried to remove property that was not added
        HEIF_ASSERT(false);
    }
}
void Heif::removeItem(Item* aItem)
{
    // check if primary
    if (mPrimaryItem == aItem)
    {
        mPrimaryItem = nullptr;
    }
    // firstly remove it from the all items list.
    if (!RemoveItemFrom(mItems, aItem))
    {
        // Tried to remove an item that was not added!
        HEIF_ASSERT(false);
    }

    // secondly remove the item from the mItemsOfType lists..
    auto tmp = mItemsOfType.find(aItem->getType());
    if (tmp != mItemsOfType.end())
    {
        if (!RemoveItemFrom(tmp->second, aItem))
        {
            // Tried to remove an item that was not added!
            HEIF_ASSERT(false);
        }
        // and cleanup the type map if it was the last of it's type
        if (tmp->second.empty())
        {
            mItemsOfType.erase(tmp);
        }
    }
}

uint32_t Heif::getItemsOfTypeCount(const HEIF::FourCC& aType) const
{
    const auto& it = mItemsOfType.find(aType);
    if (it != mItemsOfType.end())
    {
        return (uint32_t) it->second.size();
    }
    return 0;
}

Item* Heif::getItemOfType(const HEIF::FourCC& aType, uint32_t aId)
{
    const auto& it = mItemsOfType.find(aType);
    if (it != mItemsOfType.end())
    {
        if (aId < it->second.size())
        {
            return it->second[aId];
        }
    }
    return nullptr;
}
const Item* Heif::getItemOfType(const HEIF::FourCC& aType, uint32_t aId) const
{
    const auto& it = mItemsOfType.find(aType);
    if (it != mItemsOfType.end())
    {
        if (aId < it->second.size())
        {
            return it->second[aId];
        }
    }
    return nullptr;
}

ImageItem* Heif::getPrimaryItem()
{
    return mPrimaryItem;
}
const ImageItem* Heif::getPrimaryItem() const
{
    return mPrimaryItem;
}

void Heif::setPrimaryItem(ImageItem* aPrimary)
{
    mPrimaryItem = aPrimary;
}

uint32_t Heif::getMasterImageCount() const
{
    uint32_t cnt = 0;
    for (const auto& it : mItems)
    {
        if ((it->isImageItem()) && ((static_cast<ImageItem*>(it))->isMasterImage()))
        {
            ++cnt;
        }
    }
    return cnt;
}

ImageItem* Heif::getMasterImage(uint32_t aId)
{
    uint32_t cnt = 0;
    for (const auto& it : mItems)
    {
        if ((it->isImageItem()) && ((static_cast<ImageItem*>(it))->isMasterImage()))
        {
            if (cnt == aId)
                return static_cast<ImageItem*>(it);
            ++cnt;
        }
    }
    return nullptr;
}

const ImageItem* Heif::getMasterImage(uint32_t aId) const
{
    uint32_t cnt = 0;
    for (const auto& it : mItems)
    {
        if ((it->isImageItem()) && ((static_cast<ImageItem*>(it))->isMasterImage()))
        {
            if (cnt == aId)
                return static_cast<ImageItem*>(it);
            ++cnt;
        }
    }
    return nullptr;
}

uint32_t Heif::getItemCount() const
{
    uint32_t cnt = 0;
    for (const auto& it : mItems)
    {
        if ((!it->isImageItem()) || (!(static_cast<ImageItem*>(it))->isCodedImage()))
        {
            ++cnt;
        }
    }
    return cnt;
}

Item* Heif::getItem(uint32_t aId)
{
    uint32_t cnt = 0;
    for (const auto& it : mItems)
    {
        if ((!it->isImageItem()) || (!(static_cast<ImageItem*>(it))->isCodedImage()))
        {
            if (cnt == aId)
                return it;
            ++cnt;
        }
    }
    return nullptr;
}

const Item* Heif::getItem(uint32_t aId) const
{
    uint32_t cnt = 0;
    for (const auto& it : mItems)
    {
        if ((!it->isImageItem()) || (!(static_cast<ImageItem*>(it))->isCodedImage()))
        {
            if (cnt == aId)
                return it;
            ++cnt;
        }
    }
    return nullptr;
}

uint32_t Heif::getImageCount() const
{
    uint32_t cnt = 0;
    for (const auto& it : mItems)
    {
        if (it->isImageItem())
        {
            ++cnt;
        }
    }
    return cnt;
}

ImageItem* Heif::getImage(uint32_t aId)
{
    uint32_t cnt = 0;
    for (const auto& it : mItems)
    {
        if (it->isImageItem())
        {
            if (cnt == aId)
                return static_cast<ImageItem*>(it);
            ++cnt;
        }
    }
    return nullptr;
}

const ImageItem* Heif::getImage(uint32_t aId) const
{
    uint32_t cnt = 0;
    for (const auto& it : mItems)
    {
        if (it->isImageItem())
        {
            if (cnt == aId)
                return static_cast<ImageItem*>(it);
            ++cnt;
        }
    }
    return nullptr;
}


bool Heif::hasSingleImage()
{
    return getMasterImageCount() == 1;
}
bool Heif::hasImageCollection()
{
    return getMasterImageCount() > 1;
}


DecoderConfiguration* Heif::constructDecoderConfig(HEIF::Reader* aReader,
                                                   const HEIF::MediaFormat& aFormat,
                                                   const HEIF::ImageId& aItemId,
                                                   HEIF::ErrorCode& aErrorCode)
{
    // method is only valid during load
    HEIF::DecoderConfiguration cfg;
    aReader->getDecoderParameterSets(aItemId, cfg);

    auto it = mDecoderConfigsLoad.find(cfg.decoderConfigId);
    if ((it == mDecoderConfigsLoad.end()) || (it->second == nullptr))
    {
        DecoderConfiguration* config = nullptr;
        if (aFormat == HEIF::MediaFormat::AVC)
        {
            config = new AVCDecoderConfiguration(this);
        }
#if 0
        else if (aFormat == HEIF::MediaFormat::JPEG)
        {
            config = new JPEGDecoderConfiguration(this);
        }
#endif
        else if (aFormat == HEIF::MediaFormat::HEVC)
        {
            config = new HEVCDecoderConfiguration(this);
        }

        if (config)
        {
            aErrorCode = config->setConfig(cfg.decoderSpecificInfo);
            if (HEIF::ErrorCode::OK == aErrorCode)
            {
                config->setId(cfg.decoderConfigId);  // save the original id.
                mDecoderConfigsLoad[cfg.decoderConfigId] = config;
            }
            return config;
        }
#ifdef FAIL_ON_UNKNOWN_CONFIGTYPE
        aErrorCode = HEIF::ErrorCode::MEDIA_PARSING_ERROR;
#endif
        return nullptr;
    }
    aErrorCode = HEIF::ErrorCode::OK;
    return it->second;
}

uint32_t Heif::getDecoderConfigCount() const
{
    return (uint32_t) mDecoderConfigs.size();
}

DecoderConfiguration* Heif::getDecoderConfig(uint32_t aId)
{
    if (aId < mDecoderConfigs.size())
    {
        return mDecoderConfigs[aId];
    }
    return nullptr;
}
const DecoderConfiguration* Heif::getDecoderConfig(uint32_t aId) const
{
    if (aId < mDecoderConfigs.size())
    {
        return mDecoderConfigs[aId];
    }
    return nullptr;
}

void Heif::addDecoderConfig(DecoderConfiguration* aItem)
{
    aItem->setId(((uint32_t) mDecoderConfigs.size()) + 1);
    if (!AddItemTo(mDecoderConfigs, aItem))
    {
        // Tried to add a already added DecoderConfiguration
        HEIF_ASSERT(false);
    }
}

void Heif::removeDecoderConfig(DecoderConfiguration* aDecoderConfig)
{
    if (!RemoveItemFrom(mDecoderConfigs, aDecoderConfig))
    {
        // Tried to remove a non added DecoderConfiguration
        HEIF_ASSERT(false);
    }
}
