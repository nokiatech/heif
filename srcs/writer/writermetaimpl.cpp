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

#include <algorithm>
#include <cstring>
#include <memory>

#include "heiferrorinfo.h"

#include "accessibilitytext.hpp"
#include "auxiliarytypeproperty.hpp"
#include "avcconfigurationbox.hpp"
#include "cleanaperturebox.hpp"
#include "colourinformationbox.hpp"
#include "creationtimeinformation.hpp"
#include "customallocator.hpp"
#include "hevcconfigurationbox.hpp"
#include "imagegrid.hpp"
#include "imagemirror.hpp"
#include "imageoverlay.hpp"
#include "imagerelativelocationproperty.hpp"
#include "imagerotation.hpp"
#include "imagescaling.hpp"
#include "imagespatialextentsproperty.hpp"
#include "jpegconfigurationbox.hpp"
#include "layerselectorproperty.hpp"
#include "modificationtimeinformation.hpp"
#include "pixelaspectratiobox.hpp"
#include "pixelinformationproperty.hpp"
#include "rawpropertybox.hpp"
#include "requiredreferencetypesproperty.hpp"
#include "subsampleinformationproperty.hpp"
#include "targetolsproperty.hpp"
#include "operatingpointsinformationproperty.hpp"
#include "timeutility.hpp"
#include "userdescriptionproperty.hpp"
#include "writerimpl.hpp"
#include "ambientviewingenvironmentbox.hpp"
#include "avccommondefs.hpp"
#include "avcdecoderconfigrecord.hpp"
#include "bitstream.hpp"
#include "contentcolourvolumebox.hpp"
#include "contentlightlevelbox.hpp"
#include "filetypebox.hpp"
#include "fourccint.hpp"
#include "heifcommondatatypes.h"
#include "heifwriterdatatypes.h"
#include "hevccommondefs.hpp"
#include "hevcdecoderconfigrecord.hpp"
#include "idgenerators.hpp"
#include "iteminfobox.hpp"
#include "itempropertiesbox.hpp"
#include "log.hpp"
#include "masteringdisplaycolourvolumebox.hpp"
#include "metabox.hpp"
#include "nominaldiffusewhitebox.hpp"
#include "referenceviewingenvironmentbox.hpp"
#include "vvccommondefs.hpp"
#include "vvcconfigurationbox.hpp"
#include "vvcdecoderconfigrecord.hpp"
#include "writerdatatypesinternal.hpp"
#include <cstdint>
#include <map>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

using namespace std;

namespace HEIF
{
    namespace
    {
        ErrorCode createAvcDecoderConfigurationRecord(const Array<DecoderSpecificInfo>& nalUnits,
                                                      AvcDecoderConfigurationRecord& configRecord)
        {
            bool spsFound = false;
            bool ppsFound = false;

            for (auto& nalUnit : nalUnits)
            {
                const auto nalVector = vectorize(nalUnit.decSpecInfoData);

                if (nalUnit.decSpecInfoType == DecoderSpecInfoType::AVC_PPS && !ppsFound)
                {
                    ppsFound = true;
                    configRecord.addNalUnit(nalVector, AvcNalUnitType::PPS);
                }
                else if (nalUnit.decSpecInfoType == DecoderSpecInfoType::AVC_SPS && !spsFound)
                {
                    spsFound = true;
                    configRecord.addNalUnit(nalVector, AvcNalUnitType::SPS);
                    if (configRecord.makeConfigFromSPS(nalVector) == false)
                    {
                        return HEIF_FAIL(ErrorCode::DECODER_CONFIGURATION_ERROR,
                                         ErrorOrigin::Writer_DecoderConfig,
                                         "createAvcDecoderConfigurationRecord: makeConfigFromSPS rejected SPS NAL");
                    }
                }
                else
                {
                    return HEIF_FAIL(ErrorCode::DECODER_CONFIGURATION_ERROR,
                                     ErrorOrigin::Writer_DecoderConfig,
                                     "createAvcDecoderConfigurationRecord: unexpected/duplicate NAL unit type for AVC");
                }
            }

            if (!spsFound || !ppsFound)
            {
                return HEIF_FAIL(ErrorCode::DECODER_CONFIGURATION_ERROR,
                                 ErrorOrigin::Writer_DecoderConfig,
                                 "createAvcDecoderConfigurationRecord: missing SPS and/or PPS NAL units");
            }

            return ErrorCode::OK;
        }

        ErrorCode createHevcDecoderConfigurationRecord(const Array<DecoderSpecificInfo>& nalUnits,
                                                       HevcDecoderConfigurationRecord& configRecord)
        {
            bool spsFound = false;
            bool ppsFound = false;
            bool vpsFound = false;

            for (auto& nalUnit : nalUnits)
            {
                const auto nalVector = vectorize(nalUnit.decSpecInfoData);

                if (nalUnit.decSpecInfoType == DecoderSpecInfoType::HEVC_PPS && !ppsFound)
                {
                    ppsFound = true;
                    configRecord.addNalUnit(nalVector, HevcNalUnitType::PPS, true);
                }
                else if (nalUnit.decSpecInfoType == DecoderSpecInfoType::HEVC_VPS && !vpsFound)
                {
                    vpsFound = true;
                    configRecord.addNalUnit(nalVector, HevcNalUnitType::VPS, true);
                }
                else if (nalUnit.decSpecInfoType == DecoderSpecInfoType::HEVC_SPS && !spsFound)
                {
                    spsFound = true;
                    configRecord.addNalUnit(nalVector, HevcNalUnitType::SPS, true);
                    if (configRecord.makeConfigFromSPS(nalVector) == false)
                    {
                        return HEIF_FAIL(ErrorCode::DECODER_CONFIGURATION_ERROR,
                                         ErrorOrigin::Writer_DecoderConfig,
                                         "createHevcDecoderConfigurationRecord: makeConfigFromSPS rejected SPS NAL");
                    }
                }
                else
                {
                    return HEIF_FAIL(ErrorCode::DECODER_CONFIGURATION_ERROR,
                                     ErrorOrigin::Writer_DecoderConfig,
                                     "createHevcDecoderConfigurationRecord: unexpected/duplicate NAL unit type for HEVC");
                }
            }

            if (!spsFound || !ppsFound || !vpsFound)
            {
                return HEIF_FAIL(ErrorCode::DECODER_CONFIGURATION_ERROR,
                                 ErrorOrigin::Writer_DecoderConfig,
                                 "createHevcDecoderConfigurationRecord: missing SPS/PPS/VPS NAL units");
            }

            return ErrorCode::OK;
        }

        ErrorCode createVVCDecoderConfigurationRecord(const Array<DecoderSpecificInfo>& nalUnits,
                                                       VvcDecoderConfigurationRecord& configRecord){

            bool spsFound = false; bool ppsFound = false; bool vpsFound = false;
            for (auto& nalUnit : nalUnits)
            {
                const auto nalVector = vectorize(nalUnit.decSpecInfoData);
                if(nalVector.size() > 0){
                    if (nalUnit.decSpecInfoType == DecoderSpecInfoType::VVC_PPS && !ppsFound){
                        ppsFound = true;
                        configRecord.addNalUnit(nalVector, VvcNalUnitType::PPS_NUT, true);
                    }
                    else if (nalUnit.decSpecInfoType == DecoderSpecInfoType::VVC_VPS && !vpsFound){
                        vpsFound = true;
                        configRecord.addNalUnit(nalVector, VvcNalUnitType::VPS_NUT, true);
                    }
                    else if (nalUnit.decSpecInfoType == DecoderSpecInfoType::VVC_SPS && !spsFound){
                        spsFound = true;
                        configRecord.addNalUnit(nalVector, VvcNalUnitType::SPS_NUT, true);
                        if (!configRecord.makeConfigFromSPS(nalVector)){
                            return HEIF_FAIL(ErrorCode::DECODER_CONFIGURATION_ERROR,
                                             ErrorOrigin::Writer_DecoderConfig,
                                             "createVVCDecoderConfigurationRecord: makeConfigFromSPS rejected SPS NAL");
                        }
                    }
                    else if (nalUnit.decSpecInfoType == DecoderSpecInfoType::VVC_APS_PREFIX){
                        configRecord.addNalUnit(nalVector, VvcNalUnitType::PREFIX_APS_NUT, true);
                    }
                    else if (nalUnit.decSpecInfoType == DecoderSpecInfoType::VVC_APS_SUFFIX){
                        configRecord.addNalUnit(nalVector, VvcNalUnitType::SUFFIX_APS_NUT, true);
                    }
                    else
                    {
                        return HEIF_FAIL(ErrorCode::DECODER_CONFIGURATION_ERROR,
                                         ErrorOrigin::Writer_DecoderConfig,
                                         "createVVCDecoderConfigurationRecord: unexpected/duplicate NAL unit type for VVC");
                    }
                }
            }

            //Spec defines that at least SPS and PPS are needed
            if (!spsFound || !ppsFound)
            {
                return HEIF_FAIL(ErrorCode::DECODER_CONFIGURATION_ERROR,
                                 ErrorOrigin::Writer_DecoderConfig,
                                 "createVVCDecoderConfigurationRecord: missing SPS and/or PPS NAL units");
            }

            return ErrorCode::OK;
        }


        String charArrayToString(const Array<char>& input)
        {
            return String(input.begin(), input.end());
        }
    }  // anonymous namespace

    /* *************************************************************** */
    /* ********************* Writer API  methods ********************* */
    /* *************************************************************** */

    ErrorCode WriterImpl::setPrimaryItem(const ImageId& imageId)
    {
        HEIF_ENTRY_TRY
        {
        if (mState != State::WRITING)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Writer_State,
                             "WriterImpl::setPrimaryItem: called before initialize()");
        }

        if (!checkImageIds({imageId}))
        {
            return HEIF_FAIL(ErrorCode::INVALID_ITEM_ID,
                             ErrorOrigin::Writer_BoxBuild,
                             "WriterImpl::setPrimaryItem: item id not found");
        }

        if (mImageCollection.images[imageId].isHidden)
        {
            return HEIF_FAIL(ErrorCode::HIDDEN_PRIMARY_ITEM,
                             ErrorOrigin::Writer_State,
                             "WriterImpl::setPrimaryItem: primary item is hidden, operation not allowed");
        }

        mMetaBox.setPrimaryItem(imageId.get());
        mPrimaryItemSet = true;

        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::setPrimaryItem: exception escaped firewall")
    }

    ErrorCode WriterImpl::addImage(const MediaDataId& aMediaDataId, ImageId& aImageId)
    {
        HEIF_ENTRY_TRY
        {
        if (mState != State::WRITING)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Writer_State,
                             "WriterImpl::addImage(ImageId): called before initialize()");
        }

        if (mMediaData.count(aMediaDataId) == 0)
        {
            return HEIF_FAIL(ErrorCode::INVALID_MEDIADATA_ID,
                             ErrorOrigin::Writer_BoxBuild,
                             "WriterImpl::addImage(ImageId): media-data id not found");
        }

        const MediaData& mediaData = mMediaData.at(aMediaDataId);

        aImageId = Context::getValue();

        ImageCollection::Image newImage;
        newImage.imageId                  = aImageId;
        newImage.decoderConfigId          = mediaData.decoderConfigId;
        mImageCollection.images[aImageId] = newImage;

        struct FormatNames
        {
            FourCCInt type;
            String name;
        };
        const map<MediaFormat, FormatNames> formatMapping = {{MediaFormat::AVC, {FourCCInt("avc1"), ""}},
                                                             {MediaFormat::HEVC, {FourCCInt("hvc1"), ""}},
                                                             {MediaFormat::VVC, {FourCCInt("vvc1"), ""}},
                                                             {MediaFormat::JPEG, {FourCCInt("jpeg"), ""}}};

        const FormatNames& format = formatMapping.at(mediaData.mediaFormat);

        mMetaBox.addItem(newImage.imageId.get(), format.type, format.name);
        mMetaBox.addIloc(newImage.imageId.get(), mediaData.offset, mediaData.size, 0);

        if ((mediaData.mediaFormat == MediaFormat::AVC) || (mediaData.mediaFormat == MediaFormat::HEVC) || (mediaData.mediaFormat == MediaFormat::VVC))
        {
            uint16_t configPropertyIndex;
            ErrorCode error = getConfigIndex(mediaData.decoderConfigId, configPropertyIndex);
            if (error != ErrorCode::OK)
            {
                return error;
            }
            const auto imageSize         = mDecoderConfigIndexToSize[configPropertyIndex];
            const auto width             = imageSize.width;
            const auto height            = imageSize.height;
            const auto ispePropertyIndex = getIspeIndex(width, height);
            mMetaBox.addProperty(configPropertyIndex, {aImageId.get()}, true);
            mMetaBox.addProperty(ispePropertyIndex, {aImageId.get()}, false);
        }
        else if (mediaData.mediaFormat == MediaFormat::JPEG)
        {
            if (mediaData.decoderConfigId.get())
            {
                uint16_t configPropertyIndex;
                if (getConfigIndex(mediaData.decoderConfigId, configPropertyIndex) == ErrorCode::OK)
                {
                    mMetaBox.addProperty(configPropertyIndex, {aImageId.get()}, true);
                }
            }

            const ImageSize& size        = mJpegDimensions.at(aMediaDataId);
            const auto ispePropertyIndex = getIspeIndex(size.width, size.height);
            mMetaBox.addProperty(ispePropertyIndex, {aImageId.get()}, false);
        }

        if (!mInitialMdat && !mWriteMini) //mini files doesn't include mif1 in ftyp
        {
            mFileTypeBox.addCompatibleBrand("mif1");
        }

        if (mWriteItemCreationTimes)
        {
            addCreationTimeInformation(aImageId);
        }

        return ErrorCode::OK;
        }
        HEIF_CATCH_AND_RETURN(ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::addImage(MediaDataId): exception escaped")
    }

    ErrorCode WriterImpl::addImage(const MediaDataId& mediaDataId,
                                   const Array<ImageId>& referenceImageIds,
                                   ImageId& imageId)
    {
        HEIF_ENTRY_TRY
        {
        if (!checkImageIds({referenceImageIds}))
        {
            return HEIF_FAIL(ErrorCode::INVALID_ITEM_ID,
                             ErrorOrigin::Writer_BoxBuild,
                             "WriterImpl::addImage(Arra): item id not found");
        }

        auto error = addImage(mediaDataId, imageId);
        if (error != ErrorCode::OK)
        {
            return error;
        }

        for (const auto& referenceId : referenceImageIds)
        {
            mMetaBox.addItemReference("pred", imageId.get(), referenceId.get());
        }

        // If not yet done, create a Required reference types property containing value 'pred'.
        if (mPredRrefPropertyId == 0)
        {
            RequiredReferenceTypes rref;
            rref.referenceTypes = {"pred"};
            error               = addProperty(rref, mPredRrefPropertyId);
            if (error != ErrorCode::OK)
            {
                return error;
            }

            addCompatibleBrandCombination({"pred", "heic"});
            addCompatibleBrand("mif2");
        }

        error = associateProperty(imageId, mPredRrefPropertyId, true);

        if (mWriteItemCreationTimes)
        {
            addCreationTimeInformation(imageId);
        }

        return error;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::addImage(Arra): exception escaped firewall")
    }

    ErrorCode WriterImpl::setItemDescription(const ImageId& imageId, const ItemDescription& itemDescription)
    {
        HEIF_ENTRY_TRY
        {
        if (mState != State::WRITING)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Writer_State,
                             "WriterImpl::setItemDescription(ItemDescription): called before initialize()");
        }

        if (!checkImageIds({imageId}) && (!checkMetadataIds(imageId.get())))
        {
            return HEIF_FAIL(ErrorCode::INVALID_ITEM_ID,
                             ErrorOrigin::Writer_BoxBuild,
                             "WriterImpl::setItemDescription(ItemDescription): item id not found");
        }

        ItemInfoEntry& itemInfo = mMetaBox.getItemInfoBox().getItemById(imageId.get());
        itemInfo.setItemName({itemDescription.name.begin(), itemDescription.name.end()});
        itemInfo.setContentType({itemDescription.contentType.begin(), itemDescription.contentType.end()});
        itemInfo.setContentEncoding({itemDescription.contentEncoding.begin(), itemDescription.contentEncoding.end()});
        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::setItemDescription(ItemDescription): exception escaped firewall")
    }

    ErrorCode WriterImpl::setImageHidden(const ImageId& imageId, const bool hidden)
    {
        HEIF_ENTRY_TRY
        {
        if (mState != State::WRITING)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Writer_State,
                             "WriterImpl::setImageHidden: called before initialize()");
        }

        if (!checkImageIds({imageId.get()}))
        {
            return HEIF_FAIL(ErrorCode::INVALID_ITEM_ID,
                             ErrorOrigin::Writer_BoxBuild,
                             "WriterImpl::setImageHidden: item id not found");
        }

        mMetaBox.setImageHidden(imageId.get(), hidden);
        mImageCollection.images[imageId].isHidden = hidden;

        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::setImageHidden: exception escaped firewall")
    }

    ErrorCode WriterImpl::addAuxiliaryReference(const ImageId& fromImageId, const ImageId& toImageId)
    {
        HEIF_ENTRY_TRY
        {
        if (mState != State::WRITING)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Writer_State,
                             "WriterImpl::addAuxiliaryReference: called before initialize()");
        }

        if (!checkImageIds({fromImageId, toImageId}))
        {
            return HEIF_FAIL(ErrorCode::INVALID_ITEM_ID,
                             ErrorOrigin::Writer_BoxBuild,
                             "WriterImpl::addAuxiliaryReference: item id not found");
        }

        mMetaBox.addItemReference("auxl", fromImageId.get(), toImageId.get());
        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::addAuxiliaryReference: exception escaped firewall")
    }


    ErrorCode WriterImpl::addThumbnail(const ImageId& thumbImageId, const ImageId& masterImageId)
    {
        HEIF_ENTRY_TRY
        {
        if (mState != State::WRITING)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Writer_State,
                             "WriterImpl::addThumbnail: called before initialize()");
        }

        if (!checkImageIds({thumbImageId, masterImageId}))
        {
            return HEIF_FAIL(ErrorCode::INVALID_ITEM_ID,
                             ErrorOrigin::Writer_BoxBuild,
                             "WriterImpl::addThumbnail: item id not found");
        }

        mMetaBox.addItemReference("thmb", thumbImageId.get(), masterImageId.get());

        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::addThumbnail: exception escaped firewall")
    }

    ErrorCode WriterImpl::finalizeMetaBox()
    {
        HEIF_TRY
        {
        mMetaBox.setHandlerType("pict");

        // If primary item was not set, default to the first non-hidden image.
        if (!mPrimaryItemSet)
        {
            for (const auto& image : mImageCollection.images)
            {
                if (!image.second.isHidden)
                {
                    setPrimaryItem(image.second.imageId);
                    mPrimaryItemSet = true;
                    break;
                }
            }
        }

        for (const auto& entityGroup : mEntityGroups)
        {
            const EntityGroup& group = entityGroup.second;

            Vector<std::uint32_t> itemIds;
            for (auto entity : group.entities)
            {
                itemIds.push_back(entity.id);
            }

            for (const auto& property : group.descriptiveProperties)
            {
                mMetaBox.addProperty(static_cast<uint16_t>(property.propertyId.get()), {group.id.get()},
                                     property.essential);
            }
            for (const auto& property : group.transformativeProperties)
            {
                mMetaBox.addProperty(static_cast<uint16_t>(property.propertyId.get()), {group.id.get()},
                                     property.essential);
            }


            mMetaBox.addEntityGrouping(FourCCInt(group.type.value), entityGroup.first.get(), itemIds);
        }

        for (const auto& entry : mImageCollection.images)
        {
            const ImageCollection::Image& image = entry.second;
            for (const auto& property : image.descriptiveProperties)
            {
                mMetaBox.addProperty(static_cast<uint16_t>(property.propertyId.get()), {image.imageId.get()},
                                     property.essential);
            }
            for (const auto& property : image.transformativeProperties)
            {
                mMetaBox.addProperty(static_cast<uint16_t>(property.propertyId.get()), {image.imageId.get()},
                                     property.essential);
            }
        }

        ///@todo Validity checks?
        // if (!mPrimaryItemSet) still, in case all items were hidden.
        // if (mImageCollection.images.size() == 0)

        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::finalizeMetaBox: exception escaped firewall")
    }

    ErrorCode WriterImpl::addProperty(const CleanAperture& clap, PropertyId& propertyId)
    {
        HEIF_ENTRY_TRY
        {
        if (mState != State::WRITING)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Writer_State,
                             "WriterImpl::addProperty(CleanAperture): called before initialize()");
        }

        auto clapBox = makeCustomShared<::CleanApertureBox>();
        ::CleanApertureBox::Fraction value;
        value.numerator   = clap.heightN;
        value.denominator = clap.heightD;
        clapBox->setHeight(value);
        value.numerator   = clap.widthN;
        value.denominator = clap.widthD;
        clapBox->setWidth(value);
        value.numerator   = static_cast<uint32_t>(clap.horizontalOffsetN);
        value.denominator = clap.horizontalOffsetD;
        clapBox->setHorizOffset(value);
        value.numerator   = static_cast<uint32_t>(clap.verticalOffsetN);
        value.denominator = clap.verticalOffsetD;
        clapBox->setVertOffset(value);

        PropertyInformation info;
        info.isTransformative   = true;
        propertyId              = PropertyId(mMetaBox.addProperty(static_pointer_cast<Box>(clapBox), {}, false));
        mProperties[propertyId] = info;

        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::addProperty(CleanAperture): exception escaped firewall")
    }

    ErrorCode WriterImpl::addProperty(const Mirror& imir, PropertyId& propertyId)
    {
        HEIF_ENTRY_TRY
        {
        if (mState != State::WRITING)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Writer_State,
                             "WriterImpl::addProperty(Mirror): called before initialize()");
        }

        auto imirBox = makeCustomShared<ImageMirror>();
        imirBox->setHorizontalAxis(imir.horizontalAxis);

        PropertyInformation info;
        info.isTransformative   = true;
        propertyId              = PropertyId(mMetaBox.addProperty(static_pointer_cast<Box>(imirBox), {}, false));
        mProperties[propertyId] = info;

        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::addProperty(Mirror): exception escaped firewall")
    }

    ErrorCode WriterImpl::addProperty(const Rotate& irot, PropertyId& propertyId)
    {
        HEIF_ENTRY_TRY
        {
        if (mState != State::WRITING)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Writer_State,
                             "WriterImpl::addProperty(Rotate): called before initialize()");
        }

        auto irotBox = makeCustomShared<ImageRotation>();
        irotBox->setAngle(irot.angle);

        PropertyInformation info;
        info.isTransformative   = true;
        propertyId              = PropertyId(mMetaBox.addProperty(static_pointer_cast<Box>(irotBox), {}, false));
        mProperties[propertyId] = info;

        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::addProperty(Rotate): exception escaped firewall")
    }

    ErrorCode WriterImpl::addProperty(const Scale& iscl, PropertyId& propertyId)
    {
        HEIF_ENTRY_TRY
        {
        if (mState != State::WRITING)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Writer_State,
                             "WriterImpl::addProperty(Scale): called before initialize()");
        }

        if ((iscl.targetWidthD == 0) || (iscl.targetWidthN == 0) || (iscl.targetHeightD == 0) ||
            (iscl.targetHeightN == 0))
        {
            return HEIF_FAIL(ErrorCode::INVALID_FUNCTION_PARAMETER,
                             ErrorOrigin::Writer_Validation,
                             "WriterImpl::addProperty(Scale): invalid argument");
        }

        auto isclBox = makeCustomShared<ImageScaling>();
        ::ImageScaling::Fraction value;
        value.numerator   = iscl.targetWidthN;
        value.denominator = iscl.targetWidthD;
        isclBox->setWidth(value);
        value.numerator   = iscl.targetHeightN;
        value.denominator = iscl.targetHeightD;
        isclBox->setHeight(value);

        PropertyInformation info;
        info.isTransformative   = true;
        propertyId              = PropertyId(mMetaBox.addProperty(static_pointer_cast<Box>(isclBox), {}, false));
        mProperties[propertyId] = info;

        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::addProperty(Scale): exception escaped firewall")
    }

    ErrorCode WriterImpl::addProperty(const RelativeLocation& rloc, PropertyId& propertyId)
    {
        HEIF_ENTRY_TRY
        {
        if (mState != State::WRITING)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Writer_State,
                             "WriterImpl::addProperty(RelativeLocation): called before initialize()");
        }

        auto rlocBox = makeCustomShared<ImageRelativeLocationProperty>();
        rlocBox->setHorizontalOffset(rloc.horizontalOffset);
        rlocBox->setVerticalOffset(rloc.verticalOffset);

        PropertyInformation info;
        info.isTransformative   = false;
        propertyId              = PropertyId(mMetaBox.addProperty(static_pointer_cast<Box>(rlocBox), {}, false));
        mProperties[propertyId] = info;

        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::addProperty(RelativeLocation): exception escaped firewall")
    }

    ErrorCode WriterImpl::addProperty(const LayerSelector& lsel, PropertyId& propertyId)
    {
        HEIF_ENTRY_TRY
        {
        if (mState != State::WRITING)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Writer_State,
                             "WriterImpl::addProperty(LayerSelector): called before initialize()");
        }

        auto lselBox = makeCustomShared<LayerSelectorProperty>();
        lselBox->setLayerId(lsel.layerId);

        PropertyInformation info;
        info.isTransformative   = false;
        propertyId              = PropertyId(mMetaBox.addProperty(static_pointer_cast<Box>(lselBox), {}, false));
        mProperties[propertyId] = info;

        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::addProperty(LayerSelector): exception escaped firewall")
    }

    ErrorCode WriterImpl::addProperty(const TargetOls& tols, PropertyId& propertyId)
    {
        HEIF_ENTRY_TRY
        {
        if (mState != State::WRITING)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Writer_State,
                             "WriterImpl::addProperty(TargetOls): called before initialize()");
        }

        auto tolsBox = makeCustomShared<TargetOlsProperty>();
        tolsBox->setOlsIndex(tols.targetOlsIndex);

        PropertyInformation info;
        info.isTransformative   = false;
        propertyId              = PropertyId(mMetaBox.addProperty(static_pointer_cast<Box>(tolsBox), {}, false));
        mProperties[propertyId] = info;

        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::addProperty(TargetOls): exception escaped firewall")
    }

    ErrorCode WriterImpl::addProperty(const OperatingPointsInfo& oinf, PropertyId& propertyId)
    {
        HEIF_ENTRY_TRY
        {
        if (mState != State::WRITING)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Writer_State,
                             "WriterImpl::addProperty(OperatingPointsInfo): called before initialize()");
        }

        auto oinfBox = makeCustomShared<OperatingPointsInformationProperty>();
        oinfBox->setScalabilityMask(oinf.scalabilityMask);

        Vector<OperatingPointsInformationProperty::ProfileTierLevel> ptls;
        ptls.reserve(oinf.profileTierLevels.size);
        for (std::size_t i = 0; i < oinf.profileTierLevels.size; ++i)
        {
            OperatingPointsInformationProperty::ProfileTierLevel ptl{};
            ptl.generalProfileSpace              = oinf.profileTierLevels[i].generalProfileSpace;
            ptl.generalTierFlag                  = oinf.profileTierLevels[i].generalTierFlag;
            ptl.generalProfileIdc                = oinf.profileTierLevels[i].generalProfileIdc;
            ptl.generalProfileCompatibilityFlags = oinf.profileTierLevels[i].generalProfileCompatibilityFlags;
            ptl.generalConstraintIndicatorFlags  = oinf.profileTierLevels[i].generalConstraintIndicatorFlags;
            ptl.generalLevelIdc                  = oinf.profileTierLevels[i].generalLevelIdc;
            ptls.push_back(ptl);
        }
        oinfBox->setProfileTierLevels(ptls);

        Vector<OperatingPointsInformationProperty::OperatingPoint> ops;
        ops.reserve(oinf.operatingPoints.size);
        for (std::size_t i = 0; i < oinf.operatingPoints.size; ++i)
        {
            const auto& src = oinf.operatingPoints[i];
            OperatingPointsInformationProperty::OperatingPoint op{};
            op.outputLayerSetIdx = src.outputLayerSetIdx;
            op.maxTemporalId     = src.maxTemporalId;
            op.layers.reserve(src.layers.size);
            for (std::size_t j = 0; j < src.layers.size; ++j)
            {
                OperatingPointsInformationProperty::OperatingPointLayer layer{};
                layer.ptlIdx                 = src.layers[j].ptlIdx;
                layer.layerId                = src.layers[j].layerId;
                layer.isOutputLayer          = src.layers[j].isOutputLayer != 0;
                layer.isAlternateOutputLayer = src.layers[j].isAlternateOutputLayer != 0;
                op.layers.push_back(layer);
            }
            op.minPicWidth        = src.minPicWidth;
            op.minPicHeight       = src.minPicHeight;
            op.maxPicWidth        = src.maxPicWidth;
            op.maxPicHeight       = src.maxPicHeight;
            op.maxChromaFormat    = src.maxChromaFormat;
            op.maxBitDepthMinus8  = src.maxBitDepthMinus8;
            op.frameRateInfoFlag  = src.frameRateInfoFlag != 0;
            op.bitRateInfoFlag    = src.bitRateInfoFlag != 0;
            op.avgFrameRate       = src.avgFrameRate;
            op.constantFrameRate  = src.constantFrameRate;
            op.maxBitRate         = src.maxBitRate;
            op.avgBitRate         = src.avgBitRate;
            ops.push_back(op);
        }
        oinfBox->setOperatingPoints(ops);

        Vector<OperatingPointsInformationProperty::LayerInfo> layers;
        layers.reserve(oinf.layers.size);
        for (std::size_t i = 0; i < oinf.layers.size; ++i)
        {
            OperatingPointsInformationProperty::LayerInfo layer{};
            layer.dependentLayerId = oinf.layers[i].dependentLayerId;
            layer.dependsOnLayerIds.reserve(oinf.layers[i].dependsOnLayerIds.size);
            for (std::size_t j = 0; j < oinf.layers[i].dependsOnLayerIds.size; ++j)
            {
                layer.dependsOnLayerIds.push_back(oinf.layers[i].dependsOnLayerIds[j]);
            }
            layer.dimensionIdentifiers.reserve(oinf.layers[i].dimensionIdentifiers.size);
            for (std::size_t j = 0; j < oinf.layers[i].dimensionIdentifiers.size; ++j)
            {
                layer.dimensionIdentifiers.push_back(oinf.layers[i].dimensionIdentifiers[j]);
            }
            layers.push_back(layer);
        }
        oinfBox->setLayers(layers);

        PropertyInformation info;
        info.isTransformative   = false;
        propertyId              = PropertyId(mMetaBox.addProperty(static_pointer_cast<Box>(oinfBox), {}, false));
        mProperties[propertyId] = info;

        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::addProperty(OperatingPointsInfo): exception escaped firewall")
    }

    ErrorCode WriterImpl::addProperty(const SubSampleInfo& subs, PropertyId& propertyId)
    {
        HEIF_ENTRY_TRY
        {
        if (mState != State::WRITING)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Writer_State,
                             "WriterImpl::addProperty(SubSampleInfo): called before initialize()");
        }

        auto subsBox = makeCustomShared<SubSampleInformationProperty>();
        Vector<SubSampleInformationProperty::SubSampleEntry> entries;
        entries.reserve(subs.subSamples.size);
        for (uint64_t i = 0; i < subs.subSamples.size; ++i)
        {
            SubSampleInformationProperty::SubSampleEntry e{};
            e.subsampleSize           = subs.subSamples[i].subsampleSize;
            e.subsamplePriority       = subs.subSamples[i].subsamplePriority;
            e.discardable             = subs.subSamples[i].discardable;
            e.codecSpecificParameters = subs.subSamples[i].codecSpecificParameters;
            entries.push_back(e);
        }
        subsBox->setSubSamples(entries);

        PropertyInformation info;
        info.isTransformative   = false;
        propertyId              = PropertyId(mMetaBox.addProperty(static_pointer_cast<Box>(subsBox), {}, false));
        mProperties[propertyId] = info;

        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::addProperty(SubSampleInfo): exception escaped firewall")
    }

    ErrorCode WriterImpl::addProperty(const PixelAspectRatio& pasp, PropertyId& propertyId)
    {
        HEIF_ENTRY_TRY
        {
        if (mState != State::WRITING)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Writer_State,
                             "WriterImpl::addProperty(PixelAspectRatio): called before initialize()");
        }

        auto paspBox = makeCustomShared<PixelAspectRatioBox>();

        paspBox->setRelativeWidth(pasp.relativeWidth);
        paspBox->setRelativeHeight(pasp.relativeHeight);

        PropertyInformation info;
        info.isTransformative   = false;
        propertyId              = PropertyId(mMetaBox.addProperty(static_pointer_cast<Box>(paspBox), {}, false));
        mProperties[propertyId] = info;

        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::addProperty(PixelAspectRatio): exception escaped firewall")
    }
    ErrorCode WriterImpl::addProperty(const PixelInformation& pixi, PropertyId& propertyId)
    {
        HEIF_ENTRY_TRY
        {
        if (mState != State::WRITING)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Writer_State,
                             "WriterImpl::addProperty(PixelInformation): called before initialize()");
        }

        auto pixiBox = makeCustomShared<PixelInformationProperty>();

        pixiBox->setNumChannels(pixi.numChannels);

        Vector<std::uint8_t> bitsPerChannelVector;
        for (const auto bitsPerChannel : pixi.bitsPerChannel)
        {
            bitsPerChannelVector.push_back(bitsPerChannel);
        }
        pixiBox->setBitsPerChannels(bitsPerChannelVector);

        std::map<uint8_t, PixelInformationProperty::ChannelInfo> channelDataMap;
        for(size_t i = 0; i < pixi.ChannelData.size; ++i){
            const auto& entry = pixi.ChannelData[i];
            PixelInformationProperty::ChannelInfo outputChannelInfo;
            outputChannelInfo.ComponentFormat = entry.info.ComponentFormat;
            outputChannelInfo.SubSamplingFlag = entry.info.SubSamplingFlag;
            outputChannelInfo.ChannelLabelFlag = entry.info.ChannelLabelFlag;
            outputChannelInfo.SubSamplingType = entry.info.SubSamplingType;
            outputChannelInfo.SubSamplingLocation = entry.info.SubSamplingLocation;
            outputChannelInfo.ChannelLabel = std::string(entry.info.ChannelLabel.begin(), entry.info.ChannelLabel.end());

            channelDataMap.emplace(entry.channelIdc, outputChannelInfo);
        }
        pixiBox->setChannelData(channelDataMap);

        PropertyInformation info;
        info.isTransformative   = false;
        propertyId              = PropertyId(mMetaBox.addProperty(static_pointer_cast<Box>(pixiBox), {}, false));
        mProperties[propertyId] = info;

        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::addProperty(PixelInformation): exception escaped firewall")
    }

    ErrorCode WriterImpl::addProperty(const ColourInformation& colr, PropertyId& propertyId)
    {
        HEIF_ENTRY_TRY
        {
        if (mState != State::WRITING)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Writer_State,
                             "WriterImpl::addProperty(ColourInformation): called before initialize()");
        }

        auto colrBox = makeCustomShared<ColourInformationBox>();

        if ((colr.colourType == "nclx") || (colr.colourType == "rICC") || (colr.colourType == "prof"))
        {
            colrBox->setColourType(colr.colourType.value);
        }
        else
        {
            return HEIF_FAIL(ErrorCode::INVALID_FUNCTION_PARAMETER,
                             ErrorOrigin::Writer_Validation,
                             "WriterImpl::addProperty(ColourInformation): invalid argument");
        }

        colrBox->setColourPrimaries(colr.colourPrimaries);
        colrBox->setTransferCharacteristics(colr.transferCharacteristics);
        colrBox->setMatrixCoefficients(colr.matrixCoefficients);
        colrBox->setFullRangeFlag(colr.fullRangeFlag);
        Vector<std::uint8_t> iccProfileVector;
        for (const auto byte : colr.iccProfile)
        {
            iccProfileVector.push_back(byte);
        }
        colrBox->setIccProfile(iccProfileVector);

        PropertyInformation info;
        info.isTransformative   = false;
        propertyId              = PropertyId(mMetaBox.addProperty(static_pointer_cast<Box>(colrBox), {}, false));
        mProperties[propertyId] = info;

        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::addProperty(ColourInformation): exception escaped firewall")
    }

    ErrorCode WriterImpl::addProperty(const AuxiliaryType& auxC, PropertyId& propertyId)
    {
        HEIF_ENTRY_TRY
        {
        if (mState != State::WRITING)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Writer_State,
                             "WriterImpl::addProperty(AuxiliaryType): called before initialize()");
        }

        auto auxCBox = makeCustomShared<AuxiliaryTypeProperty>();

        String typeUrn(auxC.auxType.begin(), auxC.auxType.end());
        auxCBox->setAuxType(typeUrn);
        if (auxC.subType.size)
        {
            auxCBox->setAuxSubType(vectorize(auxC.subType));
        }

        PropertyInformation info;
        info.isTransformative   = false;
        propertyId              = PropertyId(mMetaBox.addProperty(static_pointer_cast<Box>(auxCBox), {}, false));
        mProperties[propertyId] = info;

        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::addProperty(AuxiliaryType): exception escaped firewall")
    }

    ErrorCode WriterImpl::addProperty(const RequiredReferenceTypes& rref, PropertyId& propertyId)
    {
        HEIF_ENTRY_TRY
        {
        if (mState != State::WRITING)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Writer_State,
                             "WriterImpl::addProperty(RequiredReferenceTypes): called before initialize()");
        }

        auto rrefBox = makeCustomShared<RequiredReferenceTypesProperty>();

        for (const auto& type : rref.referenceTypes)
        {
            rrefBox->addReferenceType(type.value);
        }

        PropertyInformation info;
        info.isTransformative   = false;
        propertyId              = PropertyId(mMetaBox.addProperty(static_pointer_cast<Box>(rrefBox), {}, true));
        mProperties[propertyId] = info;

        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::addProperty(RequiredReferenceTypes): exception escaped firewall")
    }

    ErrorCode WriterImpl::addProperty(const UserDescription& udes, PropertyId& propertyId)
    {
        HEIF_ENTRY_TRY
        {
        if (mState != State::WRITING)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Writer_State,
                             "WriterImpl::addProperty(UserDescription): called before initialize()");
        }

        auto udesProperty = makeCustomShared<UserDescriptionProperty>();

        udesProperty->setLang(charArrayToString(udes.lang));
        udesProperty->setName(charArrayToString(udes.name));
        udesProperty->setDescription(charArrayToString(udes.description));
        udesProperty->setTags(charArrayToString(udes.tags));

        PropertyInformation info;
        info.isTransformative   = false;
        propertyId              = PropertyId(mMetaBox.addProperty(static_pointer_cast<Box>(udesProperty), {}, false));
        mProperties[propertyId] = info;

        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::addProperty(UserDescription): exception escaped firewall")
    }

    ErrorCode WriterImpl::addProperty(const AccessibilityText& altt, PropertyId& propertyId)
    {
        HEIF_ENTRY_TRY
        {
        if (mState != State::WRITING)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Writer_State,
                             "WriterImpl::addProperty(AccessibilityText): called before initialize()");
        }

        auto alttProperty = makeCustomShared<AccessibilityTextProperty>();

        alttProperty->setLang(charArrayToString(altt.lang));
        alttProperty->setText(charArrayToString(altt.text));

        PropertyInformation info;
        info.isTransformative   = false;
        propertyId              = PropertyId(mMetaBox.addProperty(static_pointer_cast<Box>(alttProperty), {}, false));
        mProperties[propertyId] = info;

        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::addProperty(AccessibilityText): exception escaped firewall")
    }

    ErrorCode WriterImpl::addProperty(const SpatialExtents& ispe, PropertyId& propertyId)
    {
        HEIF_ENTRY_TRY
        {
        if (mState != State::WRITING){
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Writer_State,
                             "WriterImpl::addProperty(SpatialExtents): called before initialize()");
        }
        std::shared_ptr<ImageSpatialExtentsProperty> ispeProperty = makeCustomShared<ImageSpatialExtentsProperty>();

        ispeProperty->setDisplayHeight(ispe.mImageHeight);
        ispeProperty->setDisplayWidth(ispe.mImageWidth);

        PropertyInformation info;
        info.isTransformative   = false;
        propertyId              = PropertyId(mMetaBox.addProperty(static_pointer_cast<Box>(ispeProperty), {}, false));
        mProperties[propertyId] = info;

        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::addProperty(SpatialExtents): exception escaped firewall")
    }

    ErrorCode WriterImpl::addProperty(const ContentLightLevel& clli, PropertyId& propertyId)
    {
        HEIF_ENTRY_TRY
        {
        if(mState != State::WRITING){
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Writer_State,
                             "WriterImpl::addProperty(ContentLightLevel): called before initialize()");
        }
        std::shared_ptr<ContentLightLevelBox> clliProperty = makeCustomShared<ContentLightLevelBox>();
        clliProperty->setMaxContentLightLevel(clli.maxContentLightLevel);
        clliProperty->setMaxPicAverageLightLevel(clli.maxPicAverageLightLevel);

        PropertyInformation info;
        info.isTransformative   = false;
        propertyId              = PropertyId(mMetaBox.addProperty(static_pointer_cast<Box>(clliProperty), {}, false));
        mProperties[propertyId] = info;

        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::addProperty(ContentLightLevel): exception escaped firewall")
    }

    ErrorCode WriterImpl::addProperty(const MasteringDisplayColorVolume& mdcv, PropertyId& propertyId)
    {
        HEIF_ENTRY_TRY
        {
        if(mState != State::WRITING){
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Writer_State,
                             "WriterImpl::addProperty(MasteringDisplayColorVolume): called before initialize()");
        }
        std::shared_ptr<MasteringDisplayColourVolumeBox> mdcvProperty = makeCustomShared<MasteringDisplayColourVolumeBox>();

        Vector<uint16_t> primariesXVec; 
        for(const auto& px : mdcv.displayPrimariesX){ primariesXVec.push_back(px); }
        mdcvProperty->setDisplayPrimariesX(primariesXVec);

        Vector<uint16_t> primariesYVec; 
        for(const auto& py : mdcv.displayPrimariesY){ primariesYVec.push_back(py); }
        mdcvProperty->setDisplayPrimariesY(primariesYVec);

        mdcvProperty->setWhitePointX(mdcv.whitePointX);
        mdcvProperty->setWhitePointY(mdcv.whitePointY);

        mdcvProperty->setMaxDisplayMasteringLuminance(mdcv.maxDisplayMasteringLuminance);
        mdcvProperty->setMinDisplayMasteringLuminance(mdcv.minDisplayMasteringLuminance);

        PropertyInformation info;
        info.isTransformative   = false;
        propertyId              = PropertyId(mMetaBox.addProperty(static_pointer_cast<Box>(mdcvProperty), {}, false));
        mProperties[propertyId] = info;
        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::addProperty(MasteringDisplayColorVolume): exception escaped firewall")
    }

    ErrorCode WriterImpl::addProperty(const ContentColorVolume& cclv, PropertyId& propertyId)
    {
        HEIF_ENTRY_TRY
        {
        if(mState != State::WRITING){
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Writer_State,
                             "WriterImpl::addProperty(ContentColorVolume): called before initialize()");
        }
        std::shared_ptr<ContentColourVolumeBox> cclvProperty = makeCustomShared<ContentColourVolumeBox>();

        cclvProperty->setCcvPrimariesPresent(cclv.ccvPrimariesPresent);
        cclvProperty->setCcvMinLuminanceValuePresent(cclv.ccvMinLuminanceValuePresent);
        cclvProperty->setCcvMaxLuminanceValuePresent(cclv.ccvMaxLuminanceValuePresent);
        cclvProperty->setCcvAvgLuminanceValuePresent(cclv.ccvAvgLuminanceValuePresent);

        if(cclv.ccvPrimariesPresent){
            Vector<uint32_t> primariesXVec; 
            for(const auto& px : cclv.ccvPrimariesX){ primariesXVec.push_back(px); }
            cclvProperty->setCcvPrimariesX(primariesXVec);

            Vector<uint32_t> primariesYVec; 
            for(const auto& py : cclv.ccvPrimariesY){ primariesYVec.push_back(py); }
            cclvProperty->setCcvPrimariesY(primariesYVec);
        }

        if(cclv.ccvMinLuminanceValuePresent){cclvProperty->setCcvMinLuminance(cclv.ccvMinLuminanceValue);}
        if(cclv.ccvMaxLuminanceValuePresent){cclvProperty->setCcvMaxLuminance(cclv.ccvMaxLuminanceValue);}
        if(cclv.ccvAvgLuminanceValuePresent){cclvProperty->setCcvAvgLuminance(cclv.ccvAvgLuminanceValue);}

        PropertyInformation info;
        info.isTransformative   = false;
        propertyId              = PropertyId(mMetaBox.addProperty(static_pointer_cast<Box>(cclvProperty), {}, false));
        mProperties[propertyId] = info;
        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::addProperty(ContentColorVolume): exception escaped firewall")
    }

    ErrorCode WriterImpl::addProperty(const AmbientViewingEnvironment& amve, PropertyId& propertyId)
    {
        HEIF_ENTRY_TRY
        {
        if(mState != State::WRITING){
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Writer_State,
                             "WriterImpl::addProperty(AmbientViewingEnvironment): called before initialize()");
        }
        std::shared_ptr<AmbientViewingEnvironmentBox> amveProperty = makeCustomShared<AmbientViewingEnvironmentBox>();
        amveProperty->setAmbientIlluminance(amve.ambientIlluminance);
        amveProperty->setAmbientLightX(amve.ambientLightX);
        amveProperty->setAmbientLightY(amve.ambientLightY);

        PropertyInformation info;
        info.isTransformative   = false;
        propertyId              = PropertyId(mMetaBox.addProperty(static_pointer_cast<Box>(amveProperty), {}, false));
        mProperties[propertyId] = info;
        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::addProperty(AmbientViewingEnvironment): exception escaped firewall")
    }

    ErrorCode WriterImpl::addProperty(const ReferenceViewingEnvironment& reve, PropertyId& propertyId)
    {
        HEIF_ENTRY_TRY
        {
        if(mState != State::WRITING){
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Writer_State,
                             "WriterImpl::addProperty(ReferenceViewingEnvironment): called before initialize()");
        }
        std::shared_ptr<ReferenceViewingEnvironmentBox> reveProperty = makeCustomShared<ReferenceViewingEnvironmentBox>();
        reveProperty->setSurroundLuminance(reve.surroundLuminance);
        reveProperty->setSurroundLightX(reve.surroundLightX);
        reveProperty->setSurroundLightY(reve.surroundLightY);
        reveProperty->setPeripheryLuminance(reve.peripheryLuminance);
        reveProperty->setPeripheryLightX(reve.peripheryLightX);
        reveProperty->setPeripheryLightY(reve.peripheryLightY);

        PropertyInformation info;
        info.isTransformative   = false;
        propertyId              = PropertyId(mMetaBox.addProperty(static_pointer_cast<Box>(reveProperty), {}, false));
        mProperties[propertyId] = info;
        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::addProperty(ReferenceViewingEnvironment): exception escaped firewall")
    }

    ErrorCode WriterImpl::addProperty(const NominalDiffuseWhite& ndwt, PropertyId& propertyId)
    {
        HEIF_TRY
        {
        if (mState != State::WRITING){
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Writer_State,
                             "WriterImpl::addProperty(NominalDiffuseWhite): called before initialize()");
        }
        std::shared_ptr<NominalDiffuseWhiteBox> ndwtProperty = makeCustomShared<NominalDiffuseWhiteBox>();
        ndwtProperty->setDiffuseWhiteLuminance(ndwt.diffuseWhiteLuminance);

        PropertyInformation info;
        info.isTransformative   = false;
        propertyId              = PropertyId(mMetaBox.addProperty(static_pointer_cast<Box>(ndwtProperty), {}, false));
        mProperties[propertyId] = info;
        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::addProperty(NominalDiffuseWhite): exception escaped firewall")
    }

    ErrorCode WriterImpl::addProperty(const CreationTimeInformation& crtt, PropertyId& propertyId)
    {
        HEIF_TRY
        {
        if (mState != State::WRITING)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Writer_State,
                             "WriterImpl::addProperty(CreationTimeInformation): called before initialize()");
        }

        auto crttProperty = makeCustomShared<CreationTimeProperty>();

        crttProperty->setCreationTime(crtt.time);

        PropertyInformation info;
        info.isTransformative   = false;
        propertyId              = PropertyId(mMetaBox.addProperty(static_pointer_cast<Box>(crttProperty), {}, false));
        mProperties[propertyId] = info;

        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::addProperty(CreationTimeInformation): exception escaped firewall")
    }

    ErrorCode WriterImpl::addProperty(const ModificationTimeInformation& mdft, PropertyId& propertyId)
    {
        HEIF_ENTRY_TRY
        {
        if (mState != State::WRITING)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Writer_State,
                             "WriterImpl::addProperty(ModificationTimeInformation): called before initialize()");
        }

        auto mdftProperty = makeCustomShared<ModificationTimeProperty>();

        mdftProperty->setModificationTime(mdft.time);

        PropertyInformation info;
        info.isTransformative   = false;
        propertyId              = PropertyId(mMetaBox.addProperty(static_pointer_cast<Box>(mdftProperty), {}, false));
        mProperties[propertyId] = info;

        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::addProperty(ModificationTimeInformation): exception escaped firewall")
    }

    ErrorCode WriterImpl::addProperty(const RawProperty& property, const bool isTransformative, PropertyId& propertyId)
    {
        HEIF_ENTRY_TRY
        {
        if (mState != State::WRITING)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Writer_State,
                             "WriterImpl::addProperty(RawProperty): called before initialize()");
        }

        //TBD scenario with RAW property with 4CC type specified? Currently 4CC type value is not used for anything?

        //if the property has no 4cc type it should be handled as raw
        if(property.type == FourCC()){
            auto customBox = makeCustomShared<RawPropertyBox>();

            Vector<uint8_t> data(property.data.begin(), property.data.end());
            customBox->setData(data);

            PropertyInformation info;
            info.isTransformative   = isTransformative;
            propertyId              = PropertyId(mMetaBox.addProperty(static_pointer_cast<Box>(customBox), {}, false));
            mProperties[propertyId] = info;
        }

        //workarounds if writer->feedDecoderConfig() doesn't work, if common occurence fix cfg side
        else if(property.type == "vvcC"){
            logWarning() << "vvcC config passed as RawProperty. Possible error in recognizing cfg." << '\n';

            std::shared_ptr<VvcConfigurationBox> vvcc = makeCustomShared<VvcConfigurationBox>();
            ISOBMFF::BitStream confBitstream;
            Vector<uint8_t> confData(property.data.begin(), property.data.end());
            confBitstream.write8BitsArray(confData, confData.size());
            vvcc->parseBox(confBitstream);

            PropertyInformation info;
            info.isTransformative   = isTransformative;
            propertyId              = PropertyId(mMetaBox.addProperty(static_pointer_cast<Box>(vvcc), {}, true));
            mProperties[propertyId] = info;
        }
        else if(property.type =="hvcC")
        {
            logWarning() << "hvcC config passed as RawProperty. Possible error in recognizing cfg." << '\n';

            std::shared_ptr<HevcConfigurationBox> hvcc = makeCustomShared<HevcConfigurationBox>();
            ISOBMFF::BitStream confBitstream;
            Vector<uint8_t> confData(property.data.begin(), property.data.end());
            confBitstream.write8BitsArray(confData, confData.size());
            hvcc->parseBox(confBitstream);

            PropertyInformation info;
            info.isTransformative   = isTransformative;
            propertyId              = PropertyId(mMetaBox.addProperty(static_pointer_cast<Box>(hvcc), {}, true));
            mProperties[propertyId] = info;            
        }
        else
        {
            //Handle unknown 4cc type RAW property as raw
            
            auto customBox = makeCustomShared<RawPropertyBox>();

            Vector<uint8_t> data(property.data.begin(), property.data.end());
            customBox->setData(data);

            PropertyInformation info;
            info.isTransformative   = isTransformative;
            propertyId              = PropertyId(mMetaBox.addProperty(static_pointer_cast<Box>(customBox), {}, false));
            mProperties[propertyId] = info;
        }

        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::addProperty(RawProperty): exception escaped firewall")
    }

    ErrorCode WriterImpl::addDerivedImage(const ImageId& imageId, ImageId& derivedImageId)
    {
        HEIF_ENTRY_TRY
        {
        if (mState != State::WRITING)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Writer_State,
                             "WriterImpl::addDerivedImage: called before initialize()");
        }

        if (!checkImageIds({imageId}))
        {
            return HEIF_FAIL(ErrorCode::INVALID_ITEM_ID,
                             ErrorOrigin::Writer_BoxBuild,
                             "WriterImpl::addDerivedImage: item id not found");
        }
        derivedImageId = Context::getValue();
        ImageCollection::Image newImage;
        newImage.isHidden                       = false;
        newImage.imageId                        = derivedImageId;
        mImageCollection.images[derivedImageId] = newImage;


        mMetaBox.addItem(derivedImageId.get(), "iden", "");
        mMetaBox.addItemReference("dimg", derivedImageId.get(), imageId.get());

        const auto ispeIndex =
            mMetaBox.getItemPropertiesBox().findPropertyIndex(ItemPropertiesBox::PropertyType::ISPE, imageId.get());
        if (ispeIndex == 0)
        {
            throw std::runtime_error("addIdentityDerivation: no ISPE property found for source image");
        }
        mMetaBox.addProperty(static_cast<uint16_t>(ispeIndex), {derivedImageId.get()}, false);

        if (mWriteItemCreationTimes)
        {
            addCreationTimeInformation(derivedImageId);
        }

        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::addDerivedImage: exception escaped firewall")
    }

    ErrorCode WriterImpl::associateProperty(const GroupId& groupId,
                                            const PropertyId& propertyId,
                                            const bool isEssential)
    {
        HEIF_ENTRY_TRY
        {
        if (mState != State::WRITING)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Writer_State,
                             "WriterImpl::associateProperty(bool): called before initialize()");
        }

        if (mEntityGroups.count(groupId) == 0)
        {
            return HEIF_FAIL(ErrorCode::INVALID_GROUP_ID,
                             ErrorOrigin::Writer_BoxBuild,
                             "WriterImpl::associateProperty(bool): group id not found");
        }

        if (mProperties.count(propertyId) == 0)
        {
            return HEIF_FAIL(ErrorCode::INVALID_PROPERTY_INDEX,
                             ErrorOrigin::Writer_BoxBuild,
                             "WriterImpl::associateProperty(bool): property index out of range");
        }

        PropertyAssociation association;
        association.propertyId = propertyId;
        association.essential  = isEssential;

        if (mProperties.at(propertyId).isTransformative)
        {
            mEntityGroups.at(groupId).transformativeProperties.push_back(association);
        }
        else
        {
            mEntityGroups.at(groupId).descriptiveProperties.push_back(association);
        }

        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::associateProperty(bool): exception escaped firewall")
    }

    ErrorCode WriterImpl::associateProperty(const ImageId& imageId,
                                            const PropertyId& propertyId,
                                            const bool isEssential)
    {
        HEIF_ENTRY_TRY
        {
        if (mState != State::WRITING)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Writer_State,
                             "WriterImpl::associateProperty(bool): called before initialize()");
        }

        if (!checkImageIds({imageId}))
        {
            return HEIF_FAIL(ErrorCode::INVALID_ITEM_ID,
                             ErrorOrigin::Writer_BoxBuild,
                             "WriterImpl::associateProperty(bool): item id not found");
        }

        if (mProperties.count(propertyId) == 0)
        {
            return HEIF_FAIL(ErrorCode::INVALID_PROPERTY_INDEX,
                             ErrorOrigin::Writer_BoxBuild,
                             "WriterImpl::associateProperty(bool): property index out of range");
        }

        PropertyAssociation association;
        association.propertyId = propertyId;
        association.essential  = isEssential;

        if (mProperties.at(propertyId).isTransformative)
        {
            mImageCollection.images.at(imageId).transformativeProperties.push_back(association);
        }
        else
        {
            mImageCollection.images.at(imageId).descriptiveProperties.push_back(association);
        }

        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::associateProperty(bool): exception escaped firewall")
    }

    ErrorCode WriterImpl::addDerivedImageItem(const Grid& grid, ImageId& gridId)
    {
        HEIF_ENTRY_TRY
        {
        if (mState != State::WRITING)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Writer_State,
                             "WriterImpl::addDerivedImageItem: called before initialize()");
        }

        if (!checkImageIds(grid.imageIds))
        {
            return HEIF_FAIL(ErrorCode::INVALID_ITEM_ID,
                             ErrorOrigin::Writer_BoxBuild,
                             "WriterImpl::addDerivedImageItem: item id not found");
        }

        if ((static_cast<size_t>(grid.rows) * grid.columns) != grid.imageIds.size)
        {
            return HEIF_FAIL(ErrorCode::INVALID_REFERENCE_COUNT,
                             ErrorOrigin::Writer_BoxBuild,
                             "WriterImpl::addDerivedImageItem: reference count mismatch");
        }

        if (grid.outputHeight == 0 || grid.outputWidth == 0)
        {
            return HEIF_FAIL(ErrorCode::INVALID_FUNCTION_PARAMETER,
                             ErrorOrigin::Writer_Validation,
                             "WriterImpl::addDerivedImageItem: invalid argument");
        }

        gridId = Context::getValue();
        ImageCollection::Image newImage;
        newImage.imageId                = gridId;
        mImageCollection.images[gridId] = newImage;

        ImageGrid gridItem;
        gridItem.columnsMinusOne = static_cast<uint8_t>(grid.columns - 1);
        gridItem.rowsMinusOne    = static_cast<uint8_t>(grid.rows - 1);
        gridItem.outputHeight    = grid.outputHeight;
        gridItem.outputWidth     = grid.outputWidth;

        BitStream bitstream;
        writeImageGrid(gridItem, bitstream);
        mMetaBox.addIdatItem(gridId.get(), "grid", "Derived image", bitstream.getStorage());
        for (const auto toImageId : grid.imageIds)
        {
            mMetaBox.addItemReference("dimg", gridId.get(), toImageId.get());
        }
        const auto ispeIndex = getIspeIndex(gridItem.outputWidth, gridItem.outputHeight);
        mMetaBox.addProperty(static_cast<uint16_t>(ispeIndex), {gridId.get()}, false);

        if (mWriteItemCreationTimes)
        {
            addCreationTimeInformation(gridId);
        }

        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::addDerivedImageItem: exception escaped firewall")
    }

    ErrorCode WriterImpl::addDerivedImageItem(const Overlay& iovl, ImageId& overlayId)
    {
        HEIF_ENTRY_TRY
        {
        if (mState != State::WRITING)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Writer_State,
                             "WriterImpl::addDerivedImageItem: called before initialize()");
        }

        if (!checkImageIds(iovl.imageIds))
        {
            return HEIF_FAIL(ErrorCode::INVALID_ITEM_ID,
                             ErrorOrigin::Writer_BoxBuild,
                             "WriterImpl::addDerivedImageItem: item id not found");
        }

        if (iovl.offsets.size != iovl.imageIds.size)
        {
            return HEIF_FAIL(ErrorCode::INVALID_REFERENCE_COUNT,
                             ErrorOrigin::Writer_BoxBuild,
                             "WriterImpl::addDerivedImageItem: reference count mismatch");
        }

        if (iovl.outputHeight == 0 || iovl.outputWidth == 0)
        {
            return HEIF_FAIL(ErrorCode::INVALID_FUNCTION_PARAMETER,
                             ErrorOrigin::Writer_Validation,
                             "WriterImpl::addDerivedImageItem: invalid argument");
        }

        overlayId = Context::getValue();
        ImageCollection::Image newImage;
        newImage.imageId                   = overlayId;
        mImageCollection.images[overlayId] = newImage;

        ImageOverlay iovlItem;
        iovlItem.outputHeight = iovl.outputHeight;
        iovlItem.outputWidth  = iovl.outputWidth;

        Vector<ImageOverlay::Offset> offsets;
        for (const auto& offsetConfig : iovl.offsets)
        {
            ImageOverlay::Offset offset;
            offset.horizontalOffset = offsetConfig.horizontal;
            offset.verticalOffset   = offsetConfig.vertical;
            offsets.push_back(offset);
        }
        iovlItem.offsets          = offsets;
        iovlItem.canvasFillValueR = iovl.r;
        iovlItem.canvasFillValueG = iovl.g;
        iovlItem.canvasFillValueB = iovl.b;
        iovlItem.canvasFillValueA = iovl.a;

        BitStream bitstream;
        writeImageOverlay(iovlItem, bitstream);
        mMetaBox.addIdatItem(overlayId.get(), "iovl", "Derived image", bitstream.getStorage());
        for (const auto toImageId : iovl.imageIds)
        {
            mMetaBox.addItemReference("dimg", overlayId.get(), toImageId.get());
        }
        const auto ispeIndex = getIspeIndex(iovlItem.outputWidth, iovlItem.outputHeight);
        mMetaBox.addProperty(static_cast<uint16_t>(ispeIndex), {overlayId.get()}, false);

        if (mWriteItemCreationTimes)
        {
            addCreationTimeInformation(overlayId);
        }

        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::addDerivedImageItem: exception escaped firewall")
    }


    ErrorCode WriterImpl::addMetadataItemReference(const MetadataItemId& metadataItemId, const ImageId& toImageId)
    {
        HEIF_ENTRY_TRY
        {
        if (mState != State::WRITING)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Writer_State,
                             "WriterImpl::addMetadataItemReference(ImageId): called before initialize()");
        }

        if (!checkImageIds({toImageId}))
        {
            return HEIF_FAIL(ErrorCode::INVALID_ITEM_ID,
                             ErrorOrigin::Writer_BoxBuild,
                             "WriterImpl::addMetadataItemReference(ImageId): item id not found");
        }

        bool found = false;
        for (auto finder : mMetadataItems)
        {
            if (finder.second == metadataItemId)
            {
                found = true;
                break;
            }
        }

        if (!found)
        {
            return HEIF_FAIL(ErrorCode::INVALID_METADATAITEM_ID,
                             ErrorOrigin::Writer_BoxBuild,
                             "WriterImpl::addMetadataItemReference(ImageId): metadata item id not found");
        }

        mMetaBox.addItemReference("cdsc", metadataItemId.get(), toImageId.get());
        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::addMetadataItemReference(ImageId): exception escaped firewall")
    }

    ErrorCode WriterImpl::addTbasItemReference(const ImageId& fromImageId, const ImageId& toImageId)
    {
        HEIF_ENTRY_TRY
        {
        if (mState != State::WRITING)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Writer_State,
                             "WriterImpl::addTbasItemReference: called before initialize()");
        }

        if (!checkImageIds({fromImageId}) || !checkImageIds({toImageId}))
        {
            return HEIF_FAIL(ErrorCode::INVALID_ITEM_ID,
                             ErrorOrigin::Writer_BoxBuild,
                             "WriterImpl::addTbasItemReference: item id not found");
        }

        mMetaBox.addItemReference("tbas", fromImageId.get(), toImageId.get());

        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::addTbasItemReference: exception escaped firewall")
    }

    ErrorCode WriterImpl::addBaseItemReference(const ImageId& fromImageId, const Array<ImageId>& toImageIds)
    {
        HEIF_ENTRY_TRY
        {
        if (mState != State::WRITING)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Writer_State,
                             "WriterImpl::addBaseItemReference: called before initialize()");
        }

        if (!checkImageIds({fromImageId}) || !checkImageIds(toImageIds))
        {
            return HEIF_FAIL(ErrorCode::INVALID_ITEM_ID,
                             ErrorOrigin::Writer_BoxBuild,
                             "WriterImpl::addBaseItemReference: item id not found");
        }

        for (const auto toImageId : toImageIds)
        {
            mMetaBox.addItemReference("base", fromImageId.get(), toImageId.get());
        }

        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::addBaseItemReference: exception escaped firewall")
    }

    ErrorCode WriterImpl::addMetadata(const MediaDataId& mediaDataId, MetadataItemId& metadataItemId)
    {
        HEIF_ENTRY_TRY
        {
        if (mState != State::WRITING)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Writer_State,
                             "WriterImpl::addMetadata: called before initialize()");
        }

        if (mMediaData.count(mediaDataId) == 0)
        {
            return HEIF_FAIL(ErrorCode::INVALID_MEDIADATA_ID,
                             ErrorOrigin::Writer_BoxBuild,
                             "WriterImpl::addMetadata: media-data id not found");
        }

        if (!mMetadataItems.count(mediaDataId))
        {
            return createMetadataItem(mediaDataId, metadataItemId);
        }
        else
        {
            metadataItemId = mMetadataItems.at(mediaDataId);
            return ErrorCode::OK;
        }
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::addMetadata: exception escaped firewall")
    }

    /* *************************************************************** */
    /* ******************* Internal helper methods ******************* */
    /* *************************************************************** */

    ErrorCode WriterImpl::createMetadataItem(const MediaDataId& mediaDataId, MetadataItemId& metadataItemId)
    {
        HEIF_TRY
        {
        if (mMediaData.count(mediaDataId) == 0)
        {
            return HEIF_FAIL(ErrorCode::INVALID_MEDIADATA_ID,
                             ErrorOrigin::Writer_BoxBuild,
                             "WriterImpl::createMetadataItem: media-data id not found");
        }

        if (!mMetadataItems.count(mediaDataId))
        {
            const MediaData& mediaData = mMediaData.at(mediaDataId);

            struct FormatNames
            {
                FourCCInt type;
                String name;
                String contentType;
            };
            const map<MediaFormat, FormatNames> formatMapping = {
                {MediaFormat::EXIF, {FourCCInt("Exif"), "", ""}},
                {MediaFormat::MPEG7, {FourCCInt("mime"), "", "text/xml"}},
                {MediaFormat::XMP, {FourCCInt("mime"), "", "application/rdf+xml"}}};
            const FormatNames& format = formatMapping.at(mediaData.mediaFormat);

            mMetadataItems[mediaDataId] = Context::getValue();

            ItemInfoEntry infe;
            infe.setVersion(2);
            infe.setItemType(format.type);
            infe.setItemID(mMetadataItems.at(mediaDataId).get());
            infe.setItemName(format.name);
            infe.setContentType(format.contentType);

            mMetaBox.addItem(infe);
            mMetaBox.addIloc(mMetadataItems.at(mediaDataId).get(), mediaData.offset, mediaData.size, 0);
        }
        metadataItemId = mMetadataItems.at(mediaDataId);
        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::createMetadataItem: exception escaped firewall")
    }

    bool WriterImpl::checkMetadataIds(const MetadataItemId& metadataItemId) const
    {
        bool found = false;
        for (auto finder : mMetadataItems)
        {
            if (finder.second == metadataItemId)
            {
                found = true;
                break;
            }
        }
        return found;
    }

    bool WriterImpl::checkImageIds(const Array<ImageId>& imageIds) const
    {
        if (imageIds.size == 0)
        {
            return false;
        }

        for (auto imageId : imageIds)
        {
            if (mImageCollection.images.count(imageId) == 0)
            {
                return false;
            }
        }

        return true;
    }

    uint16_t WriterImpl::getIspeIndex(const std::uint32_t width, const std::uint32_t height)
    {
        ImageSize size = {width, height};

        if (mIspeIndexes.count(size) == 0)
        {
            mIspeIndexes[size] = mMetaBox.addProperty(
                static_pointer_cast<Box>(makeCustomShared<ImageSpatialExtentsProperty>(width, height)), {}, false);
        }

        return mIspeIndexes[size];
    }

// C6262: MSVC /analyze reports ~60 KB stack.  This function dispatches
// to makeVVC/HEVC/AVC/JPEG SampleEntryBox helpers, each of which
// constructs a decoder config record with VVC spec-mandated fixed-size
// arrays.  The analyser sums the worst-case across all branches even
// though only one executes.  60 KB is well within the 1 MB default stack.
#if defined(_MSC_VER)
#pragma warning(suppress : 6262)
#endif
    ErrorCode WriterImpl::getConfigIndex(const DecoderConfigId decoderConfigId, uint16_t& propertyIndex)
    {
        HEIF_TRY
        {
        // Create a new decoder configuration property, if a matching one was not present already.
        if (mDecoderConfigs.count(decoderConfigId) == 0)
        {
            const Array<DecoderSpecificInfo>& configNalUnits = mAllDecoderConfigs.at(decoderConfigId);

            if (configNalUnits.size)
            {
                if (configNalUnits.begin()->decSpecInfoType == DecoderSpecInfoType::AVC_SPS ||
                    configNalUnits.begin()->decSpecInfoType == DecoderSpecInfoType::AVC_PPS)
                {
                    auto configBox = makeCustomShared<AvcConfigurationBox>();
                    AvcDecoderConfigurationRecord configRecord;
                    ErrorCode error = createAvcDecoderConfigurationRecord(configNalUnits, configRecord);
                    if (error != ErrorCode::OK)
                    {
                        return error;
                    }

                    configBox->setConfiguration(configRecord);
                    const auto index = mMetaBox.addProperty(static_pointer_cast<Box>(configBox), {}, true);
                    mDecoderConfigs[decoderConfigId] = index;
                    mDecoderConfigIndexToSize[index] = {configRecord.getPicWidth(), configRecord.getPicHeight()};
                }
                else if (configNalUnits.begin()->decSpecInfoType == DecoderSpecInfoType::HEVC_SPS ||
                         configNalUnits.begin()->decSpecInfoType == DecoderSpecInfoType::HEVC_PPS ||
                         configNalUnits.begin()->decSpecInfoType == DecoderSpecInfoType::HEVC_VPS)
                {
                    auto configBox = makeCustomShared<HevcConfigurationBox>();
                    HevcDecoderConfigurationRecord configRecord;
                    ErrorCode error = createHevcDecoderConfigurationRecord(configNalUnits, configRecord);
                    if (error != ErrorCode::OK)
                    {
                        return error;
                    }
                    configBox->setConfiguration(configRecord);
                    const auto index = mMetaBox.addProperty(static_pointer_cast<Box>(configBox), {}, true);
                    mDecoderConfigs[decoderConfigId] = index;
                    mDecoderConfigIndexToSize[index] = {configRecord.getPicWidth(), configRecord.getPicHeight()};
                }
                else if (configNalUnits.begin()->decSpecInfoType == DecoderSpecInfoType::JPEG)
                {
                    auto configBox = makeCustomShared<JpegConfigurationBox>();
                    configBox->setPrefix({configNalUnits.begin()->decSpecInfoData.begin(),
                                          configNalUnits.begin()->decSpecInfoData.end()});
                    const auto index = mMetaBox.addProperty(static_pointer_cast<Box>(configBox), {}, true);
                    mDecoderConfigs[decoderConfigId] = index;
                    // JPEG dimensions come from the bitstream (mJpegDimensions),
                    // not from the decoder config -- no mDecoderConfigIndexToSize entry needed.
                }
                else if (configNalUnits.begin()->decSpecInfoType == DecoderSpecInfoType::VVC_SPS ||
                         configNalUnits.begin()->decSpecInfoType == DecoderSpecInfoType::VVC_PPS ||
                         configNalUnits.begin()->decSpecInfoType == DecoderSpecInfoType::VVC_VPS ||
                         configNalUnits.begin()->decSpecInfoType == DecoderSpecInfoType::VVC_APS_PREFIX ||
                         configNalUnits.begin()->decSpecInfoType == DecoderSpecInfoType::VVC_APS_SUFFIX){
                    auto confbox = makeCustomShared<VvcConfigurationBox>();
                    VvcDecoderConfigurationRecord vvcc;
                    ErrorCode error = createVVCDecoderConfigurationRecord(configNalUnits, vvcc);
                    if (error != ErrorCode::OK){
                        return error;
                    }
                    confbox->setConfiguration(vvcc);
                    const auto index = mMetaBox.addProperty(static_pointer_cast<Box>(confbox), {}, true);
                    mDecoderConfigs[decoderConfigId] = index;

                    //create data for 'ispe'
                    ///@note at least in compact vvc conf maxpicwidth is set to image width
                    mDecoderConfigIndexToSize[index] = {vvcc.getMaxPictureWidth(), vvcc.getMaxPictureHeight()};
                }
                else
                {
                    return HEIF_FAIL(ErrorCode::UNSUPPORTED_CODE_TYPE,
                                     ErrorOrigin::Writer_DecoderConfig,
                                     "WriterImpl::getConfigIndex: codec type not supported");
                }
            }
        }

        propertyIndex = mDecoderConfigs.at(decoderConfigId);
        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::getConfigIndex: exception escaped firewall")
    }

    void WriterImpl::addCreationTimeInformation(const ImageId& imageId)
    {
        auto crttProperty = makeCustomShared<CreationTimeProperty>();

        const unsigned int SECONDS_TO_MICROSECONDS = 1000 * 1000;
        crttProperty->setCreationTime(TimeUtility::getSecondsSince1904() * SECONDS_TO_MICROSECONDS);

        PropertyInformation info;
        info.isTransformative = false;
        const auto propertyId =
            PropertyId(mMetaBox.addProperty(static_pointer_cast<Box>(crttProperty), {imageId.get()}, false));
        mProperties[propertyId] = info;
    }


}  // namespace HEIF
