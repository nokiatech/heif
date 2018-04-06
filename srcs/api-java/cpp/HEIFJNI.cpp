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

#define JNI_METHOD(return_type, method_name) JNIEXPORT return_type JNICALL Java_com_nokia_heif_HEIF_##method_name
#include "DescriptiveProperty.h"
#include "GridImageItem.h"
#include "HEVCCodedImageItem.h"
#include "Heif.h"
#include "Helpers.h"
#include "IdentityImageItem.h"
#include "ImageItem.h"
#include "Item.h"
#include "ItemProperty.h"
#include "OverlayImageItem.h"
#include "TransformativeProperty.h"
extern "C"
{
    JNI_METHOD(void, createInstanceNative)(JNIEnv *env, jobject obj)
    {
        HEIFPP::Heif *instance = new HEIFPP::Heif();
        setNativeHandle(env, obj, (jlong) instance);
    }

    JNI_METHOD(void, destroyInstanceNative)(JNIEnv *env, jobject obj)
    {
        NATIVE_HEIF(nativeHandle, obj);

        uint32_t count = nativeHandle->getImageCount();
        for (uint32_t index = 0; index < count; index++)
        {
            HEIFPP::ImageItem *image = nativeHandle->getImage(index);
            if (image->getContext() != nullptr)
            {
                jobject javaHandle = (jobject) image->getContext();
                releaseJavaHandles(env, javaHandle);
                env->DeleteGlobalRef(javaHandle);
                image->setContext(nullptr);
            }
        }

        count = nativeHandle->getItemCount();
        for (uint32_t index = 0; index < count; index++)
        {
            HEIFPP::Item *item = nativeHandle->getItem(index);
            if (item->getContext() != nullptr)
            {
                jobject javaHandle = (jobject) item->getContext();
                releaseJavaHandles(env, javaHandle);
                env->DeleteGlobalRef(javaHandle);
                item->setContext(nullptr);
            }
        }

        count = nativeHandle->getPropertyCount();
        for (uint32_t index = 0; index < count; index++)
        {
            HEIFPP::ItemProperty *item = nativeHandle->getProperty(index);
            if (item->getContext() != nullptr)
            {
                jobject javaHandle = (jobject) item->getContext();
                releaseJavaHandles(env, javaHandle);
                env->DeleteGlobalRef(javaHandle);
                item->setContext(nullptr);
            }
        }

        count = nativeHandle->getDecoderConfigCount();
        for (uint32_t index = 0; index < count; index++)
        {
            HEIFPP::DecoderConfiguration *item = nativeHandle->getDecoderConfig(index);
            if (item->getContext() != nullptr)
            {
                jobject javaHandle = (jobject) item->getContext();
                releaseJavaHandles(env, javaHandle);
                env->DeleteGlobalRef(javaHandle);
                item->setContext(nullptr);
            }
        }

        delete nativeHandle;
        setNativeHandle(env, obj, 0);
    }

    JNI_METHOD(void, loadNative)(JNIEnv *env, jobject obj, jstring filename)
    {
        NATIVE_HEIF(nativeHandle, obj);
        const char *nativeFilename = env->GetStringUTFChars(filename, 0);

        HEIFPP::Result error = nativeHandle->load(nativeFilename);
        env->ReleaseStringUTFChars(filename, nativeFilename);
        CHECK_ERROR(error, "Loading failed");
    }

    JNI_METHOD(void, saveNative)(JNIEnv *env, jobject obj, jstring filename)
    {
        NATIVE_HEIF(nativeHandle, obj);
        const char *nativeFilename = env->GetStringUTFChars(filename, 0);

        HEIFPP::Result error = nativeHandle->save(nativeFilename);

        env->ReleaseStringUTFChars(filename, nativeFilename);
        CHECK_ERROR(error, "Saving failed");
    }

    JNI_METHOD(jint, getItemCountNative)(JNIEnv *env, jobject obj)
    {
        NATIVE_HEIF(nativeHandle, obj);
        return nativeHandle->getItemCount();
    }

    JNI_METHOD(jobject, getItemNative)(JNIEnv *env, jobject obj, jint itemIndex)
    {
        NATIVE_HEIF(nativeHandle, obj);
        HEIFPP::Item *item = nativeHandle->getItem(itemIndex);
        return getJavaItem(env, obj, item);
    }

    JNI_METHOD(jint, getImageCountNative)(JNIEnv *env, jobject obj)
    {
        NATIVE_HEIF(nativeHandle, obj);
        return nativeHandle->getImageCount();
    }

    JNI_METHOD(jobject, getImageNative)(JNIEnv *env, jobject obj, jint itemIndex)
    {
        NATIVE_HEIF(nativeHandle, obj);
        HEIFPP::ImageItem *item = nativeHandle->getImage(itemIndex);
        return getJavaItem(env, obj, item);
    }

    JNI_METHOD(jint, getMasterImageCountNative)(JNIEnv *env, jobject obj)
    {
        NATIVE_HEIF(nativeHandle, obj);
        return nativeHandle->getMasterImageCount();
    }

    JNI_METHOD(jobject, getMasterImageNative)(JNIEnv *env, jobject obj, jint itemIndex)
    {
        NATIVE_HEIF(nativeHandle, obj);
        HEIFPP::ImageItem *item = nativeHandle->getMasterImage(itemIndex);
        return getJavaItem(env, obj, item);
    }

    JNI_METHOD(jint, getItemsOfTypeCountNative)(JNIEnv *env, jobject obj, jstring type)
    {
        NATIVE_HEIF(nativeHandle, obj);
        const char *nativeString = env->GetStringUTFChars(type, 0);

        jint itemCount = nativeHandle->getItemsOfTypeCount(HEIF::FourCC(nativeString));

        env->ReleaseStringUTFChars(type, nativeString);
        return itemCount;
    }

    JNI_METHOD(jobject, getItemOfTypeNative)(JNIEnv *env, jobject obj, jstring type, jint index)
    {
        NATIVE_HEIF(nativeHandle, obj);
        const char *nativeString = env->GetStringUTFChars(type, 0);
        jobject javaObject = getJavaItem(env, obj, nativeHandle->getItemOfType(HEIF::FourCC(nativeString), index));
        env->ReleaseStringUTFChars(type, nativeString);
        return javaObject;
    }

    JNI_METHOD(jobject, getPrimaryItemNative)(JNIEnv *env, jobject obj)
    {
        NATIVE_HEIF(nativeHandle, obj);
        return getJavaItem(env, obj, nativeHandle->getPrimaryItem());
    }

    JNI_METHOD(void, setPrimaryItemNative)(JNIEnv *env, jobject obj, jobject item)
    {
        NATIVE_HEIF(nativeHandle, obj);
        NATIVE_IMAGE_ITEM(nativeItem, item);
        nativeHandle->setPrimaryItem(nativeItem);
    }

    JNI_METHOD(jint, getCompatibleBrandsCountNative)(JNIEnv *env, jobject obj)
    {
        NATIVE_HEIF(nativeHandle, obj);
        return nativeHandle->compatibleBrands();
    }

    JNI_METHOD(jstring, getCompatibleBrandNative)(JNIEnv *env, jobject obj, jint index)
    {
        NATIVE_HEIF(nativeHandle, obj);
        HEIF::FourCC brand = nativeHandle->getCompatibleBrand(index);
        return env->NewStringUTF(brand.value);
    }

    JNI_METHOD(void, addCompatibleBrandNative)(JNIEnv *env, jobject obj, jstring brand)
    {
        NATIVE_HEIF(nativeHandle, obj);
        const char *nativeBrand = env->GetStringUTFChars(brand, 0);
        nativeHandle->addCompatibleBrand(HEIF::FourCC(nativeBrand));
        env->ReleaseStringUTFChars(brand, nativeBrand);
    }

    JNI_METHOD(void, removeCompatibleBrandNative)(JNIEnv *env, jobject obj, jstring brand)
    {
        NATIVE_HEIF(nativeHandle, obj);
        const char *nativeBrand = env->GetStringUTFChars(brand, 0);
        nativeHandle->removeCompatibleBrand(HEIF::FourCC(nativeBrand));
        env->ReleaseStringUTFChars(brand, nativeBrand);
    }

    JNI_METHOD(jstring, getMajorBrandNative)(JNIEnv *env, jobject obj)
    {
        NATIVE_HEIF(nativeHandle, obj);
        HEIF::FourCC brand = nativeHandle->getMajorBrand();
        return env->NewStringUTF(brand.value);
    }

    JNI_METHOD(void, setMajorBrandNative)(JNIEnv *env, jobject obj, jstring brand)
    {
        NATIVE_HEIF(nativeHandle, obj);
        const char *nativeBrand = env->GetStringUTFChars(brand, 0);
        nativeHandle->setMajorBrand(HEIF::FourCC(nativeBrand));
        env->ReleaseStringUTFChars(brand, nativeBrand);
    }

    JNI_METHOD(jint, getPropertyCountNative)(JNIEnv *env, jobject obj)
    {
        NATIVE_HEIF(nativeHandle, obj);
        return nativeHandle->getPropertyCount();
    }

    JNI_METHOD(jobject, getPropertyNative)(JNIEnv *env, jobject obj, int index)
    {
        NATIVE_HEIF(nativeHandle, obj);
        return getJavaItemProperty(env, obj, nativeHandle->getProperty(index));
    }
}
