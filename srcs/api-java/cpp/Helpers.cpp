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
 *
 *
 */

#include "Helpers.h"
#include "AlternativeTrackGroup.h"
#include "AudioTrack.h"
#include "CodedImageItem.h"
#include "DecoderConfiguration.h"
#include "DerivedImageItem.h"
#include "DescriptiveProperty.h"
#include "EntityGroup.h"
#include "GridImageItem.h"
#include "IdentityImageItem.h"
#include "ImageItem.h"
#include "Item.h"
#include "ItemProperty.h"
#include "OverlayImageItem.h"
#include "Sample.h"
#include "Track.h"
#include "TransformativeProperty.h"
#include "VideoTrack.h"

static const char* HEVC_DECODER_CONFIG_CLASS_NAME = "com/nokia/heif/HEVCDecoderConfig";
static const char* AVC_DECODER_CONFIG_CLASS_NAME  = "com/nokia/heif/AVCDecoderConfig";
static const char* AAC_DECODER_CONFIG_CLASS_NAME  = "com/nokia/heif/AACDecoderConfig";

static const char* HEVC_IMAGE_ITEM_CLASS_NAME     = "com/nokia/heif/HEVCImageItem";
static const char* AVC_IMAGE_ITEM_CLASS_NAME      = "com/nokia/heif/AVCImageItem";
static const char* CODED_IMAGE_ITEM_CLASS_NAME    = "com/nokia/heif/CodedImageItem";
static const char* GRID_IMAGE_ITEM_CLASS_NAME     = "com/nokia/heif/GridImageItem";
static const char* IDENTITY_IMAGE_ITEM_CLASS_NAME = "com/nokia/heif/IdentityImageItem";
static const char* OVERLAY_IMAGE_ITEM_CLASS_NAME  = "com/nokia/heif/OverlayImageItem";
static const char* EXIF_ITEM_CLASS_NAME           = "com/nokia/heif/ExifItem";
static const char* MPEG7_ITEM_CLASS_NAME          = "com/nokia/heif/MPEG7Item";
static const char* XMP_ITEM_CLASS_NAME            = "com/nokia/heif/XMPItem";


static const char* AUXILIARY_PROPERTY_CLASS_NAME          = "com/nokia/heif/AuxiliaryProperty";
static const char* CLEAN_APERTURE_PROPERTY_CLASS_NAME     = "com/nokia/heif/CleanApertureProperty";
static const char* ICC_COLOUR_PROPERTY_CLASS_NAME         = "com/nokia/heif/ICCColourProperty";
static const char* NCLX_COLOUR_PROPERTY_CLASS_NAME        = "com/nokia/heif/NCLXColourProperty";
static const char* MIRROR_PROPERTY_CLASS_NAME             = "com/nokia/heif/MirrorProperty";
static const char* ROTATE_PROPERTY_CLASS_NAME             = "com/nokia/heif/RotateProperty";
static const char* PIXEL_ASPECT_RATIO_PROPERTY_CLASS_NAME = "com/nokia/heif/PixelAspectRatioProperty";
static const char* PIXEL_INFORMATION_PROPERTY_CLASS_NAME  = "com/nokia/heif/PixelInformationProperty";
static const char* RELATIVE_LOCATION_PROPERTY_CLASS_NAME  = "com/nokia/heif/RelativeLocationProperty";

static const char* VIDEO_TRACK_CLASS_NAME             = "com/nokia/heif/VideoTrack";
static const char* AUDIO_TRACK_CLASS_NAME             = "com/nokia/heif/AudioTrack";
static const char* IMAGE_SEQUENCE_CLASS_NAME          = "com/nokia/heif/ImageSequence";
static const char* ALTERNATIVE_TRACK_GROUP_CLASS_NAME = "com/nokia/heif/AlternativeTrackGroup";
static const char* ENTITY_GROUP_CLASS_NAME            = "com/nokia/heif/EntityGroup";
static const char* EQUIVALENCE_GROUP_CLASS_NAME       = "com/nokia/heif/EquivalenceGroup";
static const char* STEREOPAIR_GROUP_CLASS_NAME        = "com/nokia/heif/StereoPairGroup";
static const char* ALTERNATE_GROUP_CLASS_NAME         = "com/nokia/heif/AlternateGroup";

static const char* HEVC_SAMPLE_CLASS_NAME = "com/nokia/heif/HEVCSample";
static const char* AVC_SAMPLE_CLASS_NAME  = "com/nokia/heif/AVCSample";
static const char* AAC_SAMPLE_CLASS_NAME  = "com/nokia/heif/AACSample";


template <class type>
jobject createBaseObject(JNIEnv* env, jobject parentJavaHEIF, const char* className, type nativeHandle)
{
    const jclass itemClass      = env->FindClass(className);
    const jmethodID constructor = env->GetMethodID(itemClass, "<init>", "(Lcom/nokia/heif/HEIF;J)V");
    if (constructor == NULL)
    {
        return NULL;
    }

    jobject newObject = env->NewObject(itemClass, constructor, parentJavaHEIF, (jlong) nativeHandle);

    // Create global ref
    jobject javaObject = env->NewGlobalRef(newObject);

    // Delete local ref
    env->DeleteLocalRef(newObject);

    // Delete local refs
    env->DeleteLocalRef(itemClass);

    nativeHandle->setContext(javaObject);
    return javaObject;
}
template jobject
createBaseObject(JNIEnv* env, jobject parentJavaHEIF, const char* className, HEIFPP::Item* nativeHandle);
template jobject
createBaseObject(JNIEnv* env, jobject parentJavaHEIF, const char* className, HEIFPP::CodedImageItem* nativeHandle);
template jobject
createBaseObject(JNIEnv* env, jobject parentJavaHEIF, const char* className, HEIFPP::GridImageItem* nativeHandle);
template jobject
createBaseObject(JNIEnv* env, jobject parentJavaHEIF, const char* className, HEIFPP::Overlay* nativeHandle);
template jobject
createBaseObject(JNIEnv* env, jobject parentJavaHEIF, const char* className, HEIFPP::Identity* nativeHandle);
template jobject
createBaseObject(JNIEnv* env, jobject parentJavaHEIF, const char* className, HEIFPP::ItemProperty* nativeHandle);
template jobject createBaseObject(JNIEnv* env,
                                  jobject parentJavaHEIF,
                                  const char* className,
                                  HEIFPP::DecoderConfiguration* nativeHandle);
template jobject
createBaseObject(JNIEnv* env, jobject parentJavaHEIF, const char* className, HEIFPP::AuxProperty* nativeHandle);
template jobject
createBaseObject(JNIEnv* env, jobject parentJavaHEIF, const char* className, HEIFPP::MirrorProperty* nativeHandle);
template jobject
createBaseObject(JNIEnv* env, jobject parentJavaHEIF, const char* className, HEIFPP::RotateProperty* nativeHandle);
template jobject createBaseObject(JNIEnv* env,
                                  jobject parentJavaHEIF,
                                  const char* className,
                                  HEIFPP::CleanApertureProperty* nativeHandle);
template jobject createBaseObject(JNIEnv* env,
                                  jobject parentJavaHEIF,
                                  const char* className,
                                  HEIFPP::PixelInformationProperty* nativeHandle);
template jobject createBaseObject(JNIEnv* env,
                                  jobject parentJavaHEIF,
                                  const char* className,
                                  HEIFPP::PixelAspectRatioProperty* nativeHandle);
template jobject createBaseObject(JNIEnv* env,
                                  jobject parentJavaHEIF,
                                  const char* className,
                                  HEIFPP::ColourInformationProperty* nativeHandle);

template jobject
createBaseObject(JNIEnv* env, jobject parentJavaHEIF, const char* className, HEIFPP::AudioTrack* nativeHandle);

template jobject
createBaseObject(JNIEnv* env, jobject parentJavaHEIF, const char* className, HEIFPP::VideoTrack* nativeHandle);

jobject createItem(JNIEnv* env, jobject parentJavaHEIF, void* item);
jobject createItem(JNIEnv* env, jobject parentJavaHEIF, void* item)
{
    HEIFPP::Item* heifItem = (HEIFPP::Item*) item;
    HEIF::FourCC itemType  = heifItem->getType();
    const char* className  = "invalid_class_name";

    if (itemType == HEIF::FourCC("hvc1"))
    {
        className = HEVC_IMAGE_ITEM_CLASS_NAME;
    }
    else if (itemType == HEIF::FourCC("avc1"))
    {
        className = AVC_IMAGE_ITEM_CLASS_NAME;
    }
    else if (itemType == HEIF::FourCC("jpeg"))
    {
        className = CODED_IMAGE_ITEM_CLASS_NAME;
    }
    if (itemType == HEIF::FourCC("iden"))
    {
        className = IDENTITY_IMAGE_ITEM_CLASS_NAME;
    }
    if (itemType == HEIF::FourCC("iovl"))
    {
        className = OVERLAY_IMAGE_ITEM_CLASS_NAME;
    }
    if (itemType == HEIF::FourCC("grid"))
    {
        className = GRID_IMAGE_ITEM_CLASS_NAME;
    }
    if (itemType == HEIF::FourCC("Exif"))
    {
        className = EXIF_ITEM_CLASS_NAME;
    }
    if (itemType == HEIF::FourCC("mime"))
    {
        if (heifItem->isMPEG7Item())
        {
            className = MPEG7_ITEM_CLASS_NAME;
        }
        else if (heifItem->isXMPItem())
        {
            className = XMP_ITEM_CLASS_NAME;
        }
    }
    return createBaseObject(env, parentJavaHEIF, className, heifItem);
}

jobject createItemProperty(JNIEnv* env, jobject parentJavaHEIF, void* item);
jobject createItemProperty(JNIEnv* env, jobject parentJavaHEIF, void* item)
{
    HEIFPP::ItemProperty* itemProperty = (HEIFPP::ItemProperty*) item;
    const HEIF::ItemPropertyType& type = itemProperty->getType();
    const char* className              = "invalid_class_name";
    if (type == HEIF::ItemPropertyType::AUXC)
    {
        className = AUXILIARY_PROPERTY_CLASS_NAME;
    }
    else if (type == HEIF::ItemPropertyType::AVCC)
    {
        // TODO
    }
    else if (type == HEIF::ItemPropertyType::CLAP)
    {
        className = CLEAN_APERTURE_PROPERTY_CLASS_NAME;
    }
    else if (type == HEIF::ItemPropertyType::COLR)
    {
        HEIFPP::ColourInformationProperty* colourInformationProperty =
            (HEIFPP::ColourInformationProperty*) itemProperty;
        if (colourInformationProperty->mColourInformation.colourType == HEIF::FourCC("nclx"))
        {
            className = NCLX_COLOUR_PROPERTY_CLASS_NAME;
        }
        else if (colourInformationProperty->mColourInformation.colourType == HEIF::FourCC("rICC") ||
                 colourInformationProperty->mColourInformation.colourType == HEIF::FourCC("prof"))
        {
            className = ICC_COLOUR_PROPERTY_CLASS_NAME;
        }
    }
    else if (type == HEIF::ItemPropertyType::HVCC)
    {
        // TODO
    }
    else if (type == HEIF::ItemPropertyType::IMIR)
    {
        className = MIRROR_PROPERTY_CLASS_NAME;
    }
    else if (type == HEIF::ItemPropertyType::IROT)
    {
        className = ROTATE_PROPERTY_CLASS_NAME;
    }
    else if (type == HEIF::ItemPropertyType::ISPE)
    {
        // TODO
    }
    else if (type == HEIF::ItemPropertyType::JPGC)
    {
        // TODO
    }
    else if (type == HEIF::ItemPropertyType::PASP)
    {
        className = PIXEL_ASPECT_RATIO_PROPERTY_CLASS_NAME;
    }
    else if (type == HEIF::ItemPropertyType::PIXI)
    {
        className = PIXEL_INFORMATION_PROPERTY_CLASS_NAME;
    }
    else if (type == HEIF::ItemPropertyType::RLOC)
    {
        className = RELATIVE_LOCATION_PROPERTY_CLASS_NAME;
    }
    return createBaseObject(env, parentJavaHEIF, className, itemProperty);
}

jobject createDecoderConfig(JNIEnv* env, jobject parentJavaHEIF, HEIFPP::DecoderConfiguration* nativeConfig);
jobject createDecoderConfig(JNIEnv* env, jobject parentJavaHEIF, HEIFPP::DecoderConfiguration* nativeConfig)
{
    const char* className = "invalid_class_name";
    if (nativeConfig->getMediaFormat() == HEIF::MediaFormat::HEVC)
    {
        className = HEVC_DECODER_CONFIG_CLASS_NAME;
    }
    else if (nativeConfig->getMediaFormat() == HEIF::MediaFormat::AVC)
    {
        className = AVC_DECODER_CONFIG_CLASS_NAME;
    }
    else if (nativeConfig->getMediaFormat() == HEIF::MediaFormat::AAC)
    {
        className = AAC_DECODER_CONFIG_CLASS_NAME;
    }
    return createBaseObject(env, parentJavaHEIF, className, nativeConfig);
}

jobject createTrack(JNIEnv* env, jobject parentJavaHEIF, HEIFPP::Track* nativeTrack);
jobject createTrack(JNIEnv* env, jobject parentJavaHEIF, HEIFPP::Track* nativeTrack)
{
    const char* className = "invalid_class_name";
    if (nativeTrack->isMasterImageSequence() || nativeTrack->isAuxiliaryImageSequence() ||
        nativeTrack->isThumbnailImageSequence())
    {
        className = IMAGE_SEQUENCE_CLASS_NAME;
    }
    else if (nativeTrack->isVideoTrack())
    {
        className = VIDEO_TRACK_CLASS_NAME;
    }
    else if (nativeTrack->isAudioTrack())
    {
        className = AUDIO_TRACK_CLASS_NAME;
    }
    return createBaseObject(env, parentJavaHEIF, className, nativeTrack);
}

jobject createSample(JNIEnv* env, jobject parentJavaHEIF, HEIFPP::Sample* nativeSample);
jobject createSample(JNIEnv* env, jobject parentJavaHEIF, HEIFPP::Sample* nativeSample)
{
    const char* className   = "invalid_class_name";
    const auto& decoderType = nativeSample->getDecoderCodeType();
    if (decoderType == "hvc1")
    {
        className = HEVC_SAMPLE_CLASS_NAME;
    }
    else if (decoderType == "mp4a")
    {
        className = AAC_SAMPLE_CLASS_NAME;
    }
    else if (decoderType == "avc1")
    {
        className = AVC_SAMPLE_CLASS_NAME;
    }
    return createBaseObject(env, parentJavaHEIF, className, nativeSample);
}

jobject createEntityGroup(JNIEnv* env, jobject parentJavaHEIF, HEIFPP::EntityGroup* nativeEntityGroup);
jobject createEntityGroup(JNIEnv* env, jobject parentJavaHEIF, HEIFPP::EntityGroup* nativeEntityGroup)
{
    const char* className = ENTITY_GROUP_CLASS_NAME;
    const auto& type      = nativeEntityGroup->getType();
    if (type == "eqiv")
    {
        className = EQUIVALENCE_GROUP_CLASS_NAME;
    }
    else if (type == "altr")
    {
        className = ALTERNATE_GROUP_CLASS_NAME;
    }
    else if (type == "ster")
    {
        className = STEREOPAIR_GROUP_CLASS_NAME;
    }
    return createBaseObject(env, parentJavaHEIF, className, nativeEntityGroup);
}

jobject getJavaItem(JNIEnv* env, jobject parentJavaHEIF, void* nativeObject)
{
    if (nativeObject != NULL)
    {
        HEIFPP::Item* nativeItem = (HEIFPP::Item*) nativeObject;
        if (nativeItem->getContext() == NULL)
        {
            createItem(env, parentJavaHEIF, nativeItem);
        }
        return GET_JAVA_OBJECT(nativeItem);
    }
    else
    {
        return NULL;
    }
}

jobject getJavaAlternativeTrackGroup(JNIEnv* env, jobject parentJavaHEIF, void* nativeObject)
{
    if (nativeObject != NULL)
    {
        HEIFPP::AlternativeTrackGroup* nativeGroup = (HEIFPP::AlternativeTrackGroup*) nativeObject;
        if (nativeGroup->getContext() == NULL)
        {
            createBaseObject(env, parentJavaHEIF, ALTERNATIVE_TRACK_GROUP_CLASS_NAME, nativeGroup);
        }
        return GET_JAVA_OBJECT(nativeGroup);
    }
    else
    {
        return NULL;
    }
}

jobject getJavaEntityGroup(JNIEnv* env, jobject parentJavaHEIF, void* nativeObject)
{
    if (nativeObject != NULL)
    {
        HEIFPP::EntityGroup* nativeGroup = (HEIFPP::EntityGroup*) nativeObject;
        if (nativeGroup->getContext() == NULL)
        {
            createEntityGroup(env, parentJavaHEIF, nativeGroup);
        }
        return GET_JAVA_OBJECT(nativeGroup);
    }
    else
    {
        return NULL;
    }
}

jobject getJavaTrack(JNIEnv* env, jobject parentJavaHEIF, void* nativeObject)
{
    if (nativeObject != NULL)
    {
        HEIFPP::Track* nativeItem = (HEIFPP::Track*) nativeObject;
        if (nativeItem->getContext() == NULL)
        {
            createTrack(env, parentJavaHEIF, nativeItem);
        }
        return GET_JAVA_OBJECT(nativeItem);
    }
    else
    {
        return NULL;
    }
}

jobject getJavaSample(JNIEnv* env, jobject parentJavaHEIF, void* nativeObject)
{
    if (nativeObject != NULL)
    {
        HEIFPP::Sample* nativeItem = (HEIFPP::Sample*) nativeObject;
        if (nativeItem->getContext() == NULL)
        {
            createSample(env, parentJavaHEIF, nativeItem);
        }
        return GET_JAVA_OBJECT(nativeItem);
    }
    else
    {
        return NULL;
    }
}

jobject getDecoderConfig(JNIEnv* env, jobject parentJavaHEIF, void* nativeConfig)
{
    if (nativeConfig != NULL)
    {
        HEIFPP::DecoderConfiguration* nativeItem = (HEIFPP::DecoderConfiguration*) nativeConfig;
        if (nativeItem->getContext() == NULL)
        {
            createDecoderConfig(env, parentJavaHEIF, nativeItem);
        }
        return GET_JAVA_OBJECT(nativeItem);
    }
    else
    {
        return NULL;
    }
}


jobject getJavaItemProperty(JNIEnv* env, jobject parentJavaHEIF, void* propertyItem)
{
    if (propertyItem != NULL)
    {
        HEIFPP::ItemProperty* nativeItem = (HEIFPP::ItemProperty*) propertyItem;
        if (nativeItem->getContext() == NULL)
        {
            createItemProperty(env, parentJavaHEIF, nativeItem);
        }
        return GET_JAVA_OBJECT(nativeItem);
    }
    else
    {
        return NULL;
    }
}

template <class type>
jobject getJavaObject(JNIEnv* env, jobject parentJavaHEIF, const char* className, type nativeHandle)
{
    if (nativeHandle != NULL)
    {
        if (nativeHandle->getContext() == NULL)
        {
            createBaseObject(env, parentJavaHEIF, className, nativeHandle);
        }
        return GET_JAVA_OBJECT(nativeHandle);
    }
    else
    {
        return NULL;
    }
}
template jobject
getJavaObject(JNIEnv* env, jobject parentJavaHEIF, const char* className, HEIFPP::DecoderConfiguration* nativeHandle);


jobject getJavaHEIF(JNIEnv* env, jobject obj)
{
    jclass classType              = env->GetObjectClass(obj);
    const jmethodID getHeifMethod = env->GetMethodID(classType, "getParentHEIF", "()Lcom/nokia/heif/HEIF;");
    env->DeleteLocalRef(classType);
    return env->CallObjectMethod(obj, getHeifMethod);
}

jlong getNativeHandle(JNIEnv* env, jobject obj)
{
    if (obj == nullptr)
    {
        return 0;
    }
    jclass classType = env->GetObjectClass(obj);
    jfieldID fieldID = env->GetFieldID(classType, "mNativeHandle", "J");
    env->DeleteLocalRef(classType);
    return env->GetLongField(obj, fieldID);
}

void setNativeHandle(JNIEnv* env, jobject obj, jlong handle)
{
    jclass classType = env->GetObjectClass(obj);
    jfieldID fieldID = env->GetFieldID(classType, "mNativeHandle", "J");
    env->DeleteLocalRef(classType);
    env->SetLongField(obj, fieldID, handle);
}

void releaseJavaHandles(JNIEnv* env, jobject obj)
{
    jclass classType               = env->GetObjectClass(obj);
    const jmethodID createMethodId = env->GetMethodID(classType, "releaseHandles", "()V");
    env->DeleteLocalRef(classType);
    env->CallVoidMethod(obj, createMethodId);
}

void checkError(JNIEnv* env, const char* message, int errorCode)
{
    if (errorCode != 0)
    {
        const jclass errorHandlerClass = env->FindClass("com/nokia/heif/ErrorHandler");
        const jmethodID checkErrorID =
            env->GetStaticMethodID(errorHandlerClass, "throwException", "(ILjava/lang/String;)V");
        jstring errorMessage = env->NewStringUTF(message);
        env->CallStaticVoidMethod(errorHandlerClass, checkErrorID, errorCode, errorMessage);
        env->DeleteLocalRef(errorHandlerClass);
    }
}
