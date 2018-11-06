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

#include <jni.h>

#include "AlternativeTrackGroup.h"
#include "DescriptiveProperty.h"
#include "GridImageItem.h"
#include "HEVCCodedImageItem.h"
#include "Heif.h"
#include "Helpers.h"
#include "IdentityImageItem.h"
#include "ImageItem.h"
#include "InputStream.h"
#include "Item.h"
#include "ItemProperty.h"
#include "OverlayImageItem.h"
#include "Sample.h"
#include "Track.h"
#include "TransformativeProperty.h"
#include "OutputStream.h"

#define CLASS_NAME HEIF
extern "C"
{
    JNI_METHOD(void, createInstanceNative)
    {
        HEIFPP::Heif *instance = new HEIFPP::Heif();
        setNativeHandle(env, self, (jlong) instance);
    }

    JNI_METHOD(void, destroyInstanceNative)
    {
        NATIVE_HEIF(nativeHandle, self);
        delete nativeHandle;
        setNativeHandle(env, self, 0);
    }

    JNI_METHOD_ARG(void, loadNative, jstring filename)
    {
        NATIVE_HEIF(nativeHandle, self);
        const char *nativeFilename = env->GetStringUTFChars(filename, 0);

        HEIFPP::Result error = nativeHandle->load(nativeFilename);
        env->ReleaseStringUTFChars(filename, nativeFilename);
        CHECK_ERROR(error, "Loading failed");
    }

    JNI_METHOD_ARG(void, loadStreamNative, jobject stream)
    {
        NATIVE_HEIF(nativeHandle, self);
        InputStream *inputStream = new InputStream(env, stream);
        HEIFPP::Result error     = nativeHandle->load(inputStream);
        delete inputStream;
        CHECK_ERROR(error, "Loading failed");
    }

    JNI_METHOD_ARG(void, saveNative, jstring filename)
    {
        NATIVE_HEIF(nativeHandle, self);
        const char *nativeFilename = env->GetStringUTFChars(filename, 0);

        HEIFPP::Result error = nativeHandle->save(nativeFilename);

        env->ReleaseStringUTFChars(filename, nativeFilename);
        CHECK_ERROR(error, "Saving failed");
    }

    JNI_METHOD_ARG(void, saveStreamNative, jobject stream)
    {
        NATIVE_HEIF(nativeHandle, self);
        OutputStream *outputStream = new OutputStream(env, stream);
        HEIFPP::Result error = nativeHandle->save(outputStream);
        delete outputStream;
        CHECK_ERROR(error, "Saving failed");
    }

    JNI_METHOD(jint, getItemCountNative)
    {
        NATIVE_HEIF(nativeHandle, self);
        return static_cast<jint>(nativeHandle->getItemCount());
    }

    JNI_METHOD_ARG(jobject, getItemNative, jint itemIndex)
    {
        NATIVE_HEIF(nativeHandle, self);
        HEIFPP::Item *item = nativeHandle->getItem(static_cast<uint32_t>(itemIndex));
        return getJavaItem(env, self, item);
    }

    JNI_METHOD(jint, getImageCountNative)
    {
        NATIVE_HEIF(nativeHandle, self);
        return static_cast<jint>(nativeHandle->getImageCount());
    }

    JNI_METHOD_ARG(jobject, getImageNative, jint itemIndex)
    {
        NATIVE_HEIF(nativeHandle, self);
        HEIFPP::ImageItem *item = nativeHandle->getImage(static_cast<uint32_t>(itemIndex));
        return getJavaItem(env, self, item);
    }

    JNI_METHOD(jint, getMasterImageCountNative)
    {
        NATIVE_HEIF(nativeHandle, self);
        return static_cast<jint>(nativeHandle->getMasterImageCount());
    }

    JNI_METHOD_ARG(jobject, getMasterImageNative, jint itemIndex)
    {
        NATIVE_HEIF(nativeHandle, self);
        HEIFPP::ImageItem *item = nativeHandle->getMasterImage(static_cast<uint32_t>(itemIndex));
        return getJavaItem(env, self, item);
    }

    JNI_METHOD_ARG(jint, getItemsOfTypeCountNative, jstring type)
    {
        NATIVE_HEIF(nativeHandle, self);
        const char *nativeString = env->GetStringUTFChars(type, 0);

        jint itemCount = static_cast<jint>(nativeHandle->getItemsOfTypeCount(HEIF::FourCC(nativeString)));

        env->ReleaseStringUTFChars(type, nativeString);
        return static_cast<jint>(itemCount);
    }

    JNI_METHOD_ARG(jobject, getItemOfTypeNative, jstring type, jint index)
    {
        NATIVE_HEIF(nativeHandle, self);
        const char *nativeString = env->GetStringUTFChars(type, 0);
        jobject javaObject = getJavaItem(env, self,
                                         nativeHandle->getItemOfType(HEIF::FourCC(nativeString),
                                                                     static_cast<uint32_t>(index)));
        env->ReleaseStringUTFChars(type, nativeString);
        return javaObject;
    }

    JNI_METHOD(jobject, getPrimaryItemNative)
    {
        NATIVE_HEIF(nativeHandle, self);
        return getJavaItem(env, self, nativeHandle->getPrimaryItem());
    }

    JNI_METHOD_ARG(void, setPrimaryItemNative, jobject item)
    {
        NATIVE_HEIF(nativeHandle, self);
        NATIVE_IMAGE_ITEM(nativeItem, item);
        nativeHandle->setPrimaryItem(nativeItem);
    }

    JNI_METHOD(jint, getCompatibleBrandsCountNative)
    {
        NATIVE_HEIF(nativeHandle, self);
        return static_cast<jint>(nativeHandle->compatibleBrands());
    }

    JNI_METHOD_ARG(jstring, getCompatibleBrandNative, jint index)
    {
        NATIVE_HEIF(nativeHandle, self);
        HEIF::FourCC brand = nativeHandle->getCompatibleBrand(static_cast<uint32_t>(index));
        return env->NewStringUTF(brand.value);
    }

    JNI_METHOD_ARG(void, addCompatibleBrandNative, jstring brand)
    {
        NATIVE_HEIF(nativeHandle, self);
        const char *nativeBrand = env->GetStringUTFChars(brand, 0);
        nativeHandle->addCompatibleBrand(HEIF::FourCC(nativeBrand));
        env->ReleaseStringUTFChars(brand, nativeBrand);
    }

    JNI_METHOD_ARG(void, removeCompatibleBrandNative, jstring brand)
    {
        NATIVE_HEIF(nativeHandle, self);
        const char *nativeBrand = env->GetStringUTFChars(brand, 0);
        nativeHandle->removeCompatibleBrand(HEIF::FourCC(nativeBrand));
        env->ReleaseStringUTFChars(brand, nativeBrand);
    }

    JNI_METHOD(jstring, getMajorBrandNative)
    {
        NATIVE_HEIF(nativeHandle, self);
        HEIF::FourCC brand = nativeHandle->getMajorBrand();
        return env->NewStringUTF(brand.value);
    }

    JNI_METHOD_ARG(void, setMajorBrandNative, jstring brand)
    {
        NATIVE_HEIF(nativeHandle, self);
        const char *nativeBrand = env->GetStringUTFChars(brand, 0);
        nativeHandle->setMajorBrand(HEIF::FourCC(nativeBrand));
        env->ReleaseStringUTFChars(brand, nativeBrand);
    }

    JNI_METHOD(jint, getPropertyCountNative)
    {
        NATIVE_HEIF(nativeHandle, self);
        return static_cast<jint>(nativeHandle->getPropertyCount());
    }

    JNI_METHOD_ARG(jobject, getPropertyNative, jint index)
    {
        NATIVE_HEIF(nativeHandle, self);
        return getJavaItemProperty(env, self,
                                   nativeHandle->getProperty(static_cast<uint32_t>(index)));
    }

    JNI_METHOD(jint, getTrackCountNative)
    {
        NATIVE_HEIF(nativeHandle, self);
        return static_cast<jint>(nativeHandle->getTrackCount());
    }

    JNI_METHOD_ARG(jobject, getTrackNative, jint index)
    {
        NATIVE_HEIF(nativeHandle, self);
        return getJavaTrack(env, self,
                            nativeHandle->getTrack(static_cast<uint32_t>(index)));
    }

    JNI_METHOD(jint, getAlternativeTrackGroupCountNative)
    {
        NATIVE_HEIF(nativeSelf, self);
        return static_cast<jint>(nativeSelf->getAlternativeTrackGroupCount());
    }

    JNI_METHOD_ARG(jobject, getAlternativeTrackGroupNative, jint index)
    {
        NATIVE_HEIF(nativeSelf, self);
        return getJavaAlternativeTrackGroup(env, self,
                                            nativeSelf->getAlternativeTrackGroup(static_cast<uint32_t>(index)));
    }

    JNI_METHOD(jint, getEntityGroupCountNative)
    {
        NATIVE_HEIF(nativeSelf, self);
        return static_cast<jint>(nativeSelf->getGroupCount());
    }

    JNI_METHOD_ARG(jobject, getEntityGroupNative, jint index)
    {
        NATIVE_HEIF(nativeSelf, self);
        return getJavaEntityGroup(env, self, nativeSelf->getGroup(static_cast<uint32_t>(index)));
    }
}
