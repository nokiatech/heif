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
#include "CodedImageItem.h"
#include "DerivedImageItem.h"
#include "DescriptiveProperty.h"
#include "GridImageItem.h"
#include "IdentityImageItem.h"
#include "ImageItem.h"
#include "Item.h"
#include "ItemProperty.h"
#include "OverlayImageItem.h"
#include "TransformativeProperty.h"

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
createBaseObject(JNIEnv* env, jobject parentJavaHEIF, const char* className, HEIFPP::Grid* nativeHandle);
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

jobject createItem(JNIEnv* env, jobject parentJavaHEIF, void* item)
{
    HEIFPP::Item* heifItem = (HEIFPP::Item*) item;
    HEIF::FourCC itemType  = heifItem->getType();
    const char* className  = "invalid_class_name";

    if (itemType == HEIF::FourCC('hvc1'))
    {
        className = HEVC_IMAGE_ITEM_CLASS_NAME;
    }
    else if (itemType == HEIF::FourCC('avc1'))
    {
        className = AVC_IMAGE_ITEM_CLASS_NAME;
    }
    else if (itemType == HEIF::FourCC('jpeg'))
    {
        className = CODED_IMAGE_ITEM_CLASS_NAME;
    }
    if (itemType == HEIF::FourCC('iden'))
    {
        className = IDENTITY_IMAGE_ITEM_CLASS_NAME;
    }
    if (itemType == HEIF::FourCC('iovl'))
    {
        className = OVERLAY_IMAGE_ITEM_CLASS_NAME;
    }
    if (itemType == HEIF::FourCC('grid'))
    {
        className = GRID_IMAGE_ITEM_CLASS_NAME;
    }
    if (itemType == HEIF::FourCC('Exif'))
    {
        className = EXIF_ITEM_CLASS_NAME;
    }
    if (itemType == HEIF::FourCC('mime'))
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

jobject createItemProperty(JNIEnv* env, jobject parentJavaHEIF, void* item)
{
    HEIFPP::ItemProperty* itemProperty = (HEIFPP::ItemProperty*) item;
    const HEIF::ItemPropertyType& type = itemProperty->getType();
    const char* className              = "blaa";
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

jobject createDecoderConfig(JNIEnv* env, jobject parentJavaHEIF, HEIFPP::DecoderConfiguration* nativeConfig)
{
    const char* className = "blaa";
    if (nativeConfig->getMediaFormat() == HEIF::MediaFormat::HEVC)
    {
        className = HEVC_DECODER_CONFIG_CLASS_NAME;
    }
    else if (nativeConfig->getMediaFormat() == HEIF::MediaFormat::AVC)
    {
        className = AVC_DECODER_CONFIG_CLASS_NAME;
    }
    return createBaseObject(env, parentJavaHEIF, className, nativeConfig);
}

jobject getJavaItem(JNIEnv* env, jobject parentJavaHEIF, void* nativeObject)
{
    if (nativeObject != NULL)
    {
        HEIFPP::Item* nativeItem = (HEIFPP::Item*) nativeObject;
        if (nativeItem->getContext() == NULL)
        {
            jobject javaItem = createItem(env, parentJavaHEIF, nativeItem);
        }
        return (jobject) nativeItem->getContext();
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
            jobject javaItem = createDecoderConfig(env, parentJavaHEIF, nativeItem);
        }
        return (jobject) nativeItem->getContext();
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
            jobject javaItem = createItemProperty(env, parentJavaHEIF, nativeItem);
        }
        return (jobject) nativeItem->getContext();
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
            jobject javaItem = createBaseObject(env, parentJavaHEIF, className, nativeHandle);
        }
        return (jobject) nativeHandle->getContext();
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
            env->GetStaticMethodID(errorHandlerClass, "checkError", "(ILjava/lang/String;)V");
        jstring errorMessage = env->NewStringUTF(message);
        env->CallStaticVoidMethod(errorHandlerClass, checkErrorID, errorCode, errorMessage);
        env->DeleteLocalRef(errorHandlerClass);
    }
}