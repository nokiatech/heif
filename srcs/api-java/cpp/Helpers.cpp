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

#include <MimeItem.h>
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

static const char* ALTERNATIVE_TRACK_GROUP_CLASS_NAME = "com/nokia/heif/AlternativeTrackGroup";

static const char* CREATE_ITEM_METHOD = "createItem";
static const char* CREATE_ITEM_PROPERTY_METHOD = "createItemProperty";
static const char* CREATE_DECODER_CONFIG_METHOD = "createDecoderConfig";
static const char* CREATE_SAMPLE_METHOD = "createSample";
static const char* CREATE_ENTITY_GROUP_METHOD = "createEntityGroup";
static const char* CREATE_TRACK_METHOD = "createTrack";

template <class type>
jobject createJavaBaseObject(JNIEnv* env, jobject parentHeif, type nativeHandle, const char* methodName, const char* fourCC)
{
    jclass classType              = env->GetObjectClass(parentHeif);
    const jmethodID createItemMethod = env->GetMethodID(classType, methodName, "(Ljava/lang/String;J)Lcom/nokia/heif/Base;");
    env->DeleteLocalRef(classType);
    jstring fourCCString = env->NewStringUTF(fourCC);
    jobject javaItem = env->CallObjectMethod(parentHeif, createItemMethod, fourCCString, (jlong) nativeHandle);
    return javaItem;
}

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
    // Delete local refs
    env->DeleteLocalRef(itemClass);

    return newObject;
}

jobject createItem(JNIEnv* env, jobject parentJavaHEIF, void* item);
jobject createItem(JNIEnv* env, jobject parentJavaHEIF, void* item)
{
    HEIFPP::Item* heifItem = (HEIFPP::Item*) item;
    HEIF::FourCC itemType  = heifItem->getType();
    const char* mimeForCreation = "invl";

    if (itemType == HEIF::FourCC("hvc1")
            || itemType == HEIF::FourCC("avc1")
            || itemType == HEIF::FourCC("jpeg")
            || itemType == HEIF::FourCC("iden")
            || itemType == HEIF::FourCC("iovl")
            || itemType == HEIF::FourCC("grid")
            || itemType == HEIF::FourCC("Exif"))
    {
        mimeForCreation = itemType.value;
    }

    else if (itemType == HEIF::FourCC("mime"))
    {
        if (heifItem->isMPEG7Item())
        {
            mimeForCreation = "mpg7";
        }
        else if (heifItem->isXMPItem())
        {
            mimeForCreation = "xmp1";
        }
    }
    return createJavaBaseObject(env, parentJavaHEIF, heifItem, CREATE_ITEM_METHOD, mimeForCreation);
}

jobject createItemProperty(JNIEnv* env, jobject parentJavaHEIF, void* item);
jobject createItemProperty(JNIEnv* env, jobject parentJavaHEIF, void* item)
{
    HEIFPP::ItemProperty* itemProperty = (HEIFPP::ItemProperty*) item;
    const HEIF::ItemPropertyType& type = itemProperty->getType();

    const char* mimeForCreation = "invl";

    if (type == HEIF::ItemPropertyType::COLR)
    {
        HEIFPP::ColourInformationProperty* colourInformationProperty =
                (HEIFPP::ColourInformationProperty*) itemProperty;
        if (colourInformationProperty->mColourInformation.colourType == HEIF::FourCC("nclx"))
        {
            mimeForCreation = "nclx";
        }
        else if (colourInformationProperty->mColourInformation.colourType == HEIF::FourCC("rICC") ||
                 colourInformationProperty->mColourInformation.colourType == HEIF::FourCC("prof"))
        {
            mimeForCreation = "rICC";
        }
    }
    else
    {
        mimeForCreation = itemProperty->rawType().value;
    }

    return createJavaBaseObject(env, parentJavaHEIF, itemProperty, CREATE_ITEM_PROPERTY_METHOD, mimeForCreation);
}

jobject createDecoderConfig(JNIEnv* env, jobject parentJavaHEIF, HEIFPP::DecoderConfig* nativeConfig);
jobject createDecoderConfig(JNIEnv* env, jobject parentJavaHEIF, HEIFPP::DecoderConfig* nativeConfig)
{
    return createJavaBaseObject(env, parentJavaHEIF, nativeConfig, CREATE_DECODER_CONFIG_METHOD, nativeConfig->getMediaType().value);
}

jobject createTrack(JNIEnv* env, jobject parentJavaHEIF, HEIFPP::Track* nativeTrack);
jobject createTrack(JNIEnv* env, jobject parentJavaHEIF, HEIFPP::Track* nativeTrack)
{
    const char* mimeForCreation = "invl";
    if (nativeTrack->isMasterImageSequence() || nativeTrack->isAuxiliaryImageSequence() ||
        nativeTrack->isThumbnailImageSequence())
    {
        mimeForCreation = "pict";
    }
    else if (nativeTrack->isVideoTrack())
    {
        mimeForCreation = "vide";
    }
    else if (nativeTrack->isAudioTrack())
    {
        mimeForCreation = "soun";
    }
    return createJavaBaseObject(env, parentJavaHEIF, nativeTrack, CREATE_TRACK_METHOD, mimeForCreation);
}

jobject createSample(JNIEnv* env, jobject parentJavaHEIF, HEIFPP::Sample* nativeSample);
jobject createSample(JNIEnv* env, jobject parentJavaHEIF, HEIFPP::Sample* nativeSample)
{
    return createJavaBaseObject(env, parentJavaHEIF, nativeSample, CREATE_SAMPLE_METHOD, nativeSample->getType().value);
}

jobject createEntityGroup(JNIEnv* env, jobject parentJavaHEIF, HEIFPP::EntityGroup* nativeEntityGroup);
jobject createEntityGroup(JNIEnv* env, jobject parentJavaHEIF, HEIFPP::EntityGroup* nativeEntityGroup)
{
    return createJavaBaseObject(env, parentJavaHEIF, nativeEntityGroup, CREATE_ENTITY_GROUP_METHOD, nativeEntityGroup->getType().value);
}

jobject getJavaItem(JNIEnv* env, jobject parentJavaHEIF, void* nativeObject)
{
    if (nativeObject != NULL)
    {
        HEIFPP::Item* nativeItem = (HEIFPP::Item*) nativeObject;
        return createItem(env, parentJavaHEIF, nativeItem);
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
        return createBaseObject(env, parentJavaHEIF, ALTERNATIVE_TRACK_GROUP_CLASS_NAME, nativeGroup);
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
        return createEntityGroup(env, parentJavaHEIF, nativeGroup);
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
        return createTrack(env, parentJavaHEIF, nativeItem);
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
        return createSample(env, parentJavaHEIF, nativeItem);
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
        HEIFPP::DecoderConfig* nativeItem = (HEIFPP::DecoderConfig*) nativeConfig;
        return createDecoderConfig(env, parentJavaHEIF, nativeItem);
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
        return createItemProperty(env, parentJavaHEIF, nativeItem);
    }
    else
    {
        return NULL;
    }
}

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
