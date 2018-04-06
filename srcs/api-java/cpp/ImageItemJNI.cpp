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
#include "Helpers.h"
#include "ImageItem.h"
#define JNI_METHOD(return_type, method_name) JNIEXPORT return_type JNICALL Java_com_nokia_heif_ImageItem_##method_name

extern "C"
{
    JNI_METHOD(jint, getWidthNative)(JNIEnv *env, jobject obj)
    {
        NATIVE_IMAGE_ITEM(nativeHandle, obj);
        return nativeHandle->width();
    }

    JNI_METHOD(jint, getHeightNative)(JNIEnv *env, jobject obj)
    {
        NATIVE_IMAGE_ITEM(nativeHandle, obj);
        return nativeHandle->height();
    }

    JNI_METHOD(void, setSizeNative)(JNIEnv *env, jobject obj, jint width, jint height)
    {
        NATIVE_IMAGE_ITEM(nativeHandle, obj);
        nativeHandle->setSize(width, height);
    }

    JNI_METHOD(jboolean, getHiddenNative)(JNIEnv *env, jobject obj)
    {
        NATIVE_IMAGE_ITEM(nativeHandle, obj);
        return nativeHandle->isHidden();
    }

    JNI_METHOD(void, setHiddenNative)(JNIEnv *env, jobject obj, jboolean hidden)
    {
        NATIVE_IMAGE_ITEM(nativeHandle, obj);
        nativeHandle->setHidden(hidden);
    }

    JNI_METHOD(jint, getThumbnailCountNative)(JNIEnv *env, jobject obj)
    {
        NATIVE_IMAGE_ITEM(nativeHandle, obj);
        return nativeHandle->getThumbnailCount();
    }

    JNI_METHOD(jobject, getThumbnailNative)(JNIEnv *env, jobject obj, jint index)
    {
        NATIVE_IMAGE_ITEM(nativeHandle, obj);
        return getJavaItem(env, getJavaHEIF(env, obj), nativeHandle->getThumbnail(index));
    }

    JNI_METHOD(void, addThumbnailNative)(JNIEnv *env, jobject obj, jobject thumbnail)
    {
        NATIVE_IMAGE_ITEM(nativeHandle, obj);
        NATIVE_IMAGE_ITEM(nativeThumbnail, thumbnail);
        nativeHandle->addThumbnail(nativeThumbnail);
    }

    JNI_METHOD(void, removeThumbnailNative)(JNIEnv *env, jobject obj, jobject thumbnail)
    {
        NATIVE_IMAGE_ITEM(nativeHandle, obj);
        NATIVE_IMAGE_ITEM(nativeThumbnail, thumbnail);
        nativeHandle->removeThumbnail(nativeThumbnail);
    }

    JNI_METHOD(jint, getAuxiliaryCountNative)(JNIEnv *env, jobject obj)
    {
        NATIVE_IMAGE_ITEM(nativeHandle, obj);
        return nativeHandle->getAuxCount();
    }

    JNI_METHOD(jobject, getAuxiliaryNative)(JNIEnv *env, jobject obj, jint index)
    {
        NATIVE_IMAGE_ITEM(nativeHandle, obj);
        return getJavaItem(env, getJavaHEIF(env, obj), nativeHandle->getAux(index));
    }

    JNI_METHOD(void, addAuxiliaryImageNative)(JNIEnv *env, jobject obj, jobject auxiliary)
    {
        NATIVE_IMAGE_ITEM(nativeHandle, obj);
        NATIVE_IMAGE_ITEM(nativeAuxiliary, auxiliary);
        nativeHandle->addAuxImage(nativeAuxiliary);
    }

    JNI_METHOD(void, removeAuxiliaryImageNative)(JNIEnv *env, jobject obj, jobject auxiliary)
    {
        NATIVE_IMAGE_ITEM(nativeHandle, obj);
        NATIVE_IMAGE_ITEM(nativeAuxiliary, auxiliary);
        nativeHandle->removeAuxImage(nativeAuxiliary);
    }

    JNI_METHOD(jint, transformativePropertyCountNative)(JNIEnv *env, jobject obj)
    {
        NATIVE_IMAGE_ITEM(nativeHandle, obj);
        return nativeHandle->transformativePropertyCount();
    }

    JNI_METHOD(jobject, getTransformativePropertyNative)(JNIEnv *env, jobject obj, jint index)
    {
        NATIVE_IMAGE_ITEM(nativeHandle, obj);
        return getJavaItemProperty(env, getJavaHEIF(env, obj), nativeHandle->getTransformativeProperty(index));
    }

    JNI_METHOD(jobject, getPixelAspectRatioPropertyNative)(JNIEnv *env, jobject obj)
    {
        NATIVE_IMAGE_ITEM(nativeHandle, obj);
        return getJavaItemProperty(env, getJavaHEIF(env, obj), nativeHandle->pixelAspectRatio());
    }

    JNI_METHOD(jobject, getColourInformationPropertyNative)(JNIEnv *env, jobject obj)
    {
        NATIVE_IMAGE_ITEM(nativeHandle, obj);
        return getJavaItemProperty(env, getJavaHEIF(env, obj), nativeHandle->colourInformation());
    }

    JNI_METHOD(jobject, getPixelInformationPropertyNative)(JNIEnv *env, jobject obj)
    {
        NATIVE_IMAGE_ITEM(nativeHandle, obj);
        return getJavaItemProperty(env, getJavaHEIF(env, obj), nativeHandle->pixelInformation());
    }

    JNI_METHOD(jobject, getRelativeLocationPropertyNative)(JNIEnv *env, jobject obj)
    {
        NATIVE_IMAGE_ITEM(nativeHandle, obj);
        return getJavaItemProperty(env, getJavaHEIF(env, obj), nativeHandle->relativeLocation());
    }

    JNI_METHOD(jobject, getAuxiliaryPropertyNative)(JNIEnv *env, jobject obj)
    {
        NATIVE_IMAGE_ITEM(nativeHandle, obj);
        return getJavaItemProperty(env, getJavaHEIF(env, obj), nativeHandle->aux());
    }

    JNI_METHOD(void, addPropertyNative)(JNIEnv *env, jobject obj, jobject property, jboolean essential)
    {
        NATIVE_IMAGE_ITEM(nativeHandle, obj);
        NATIVE_ITEM_PROPERTY(propertyNative, property);
        nativeHandle->addProperty(propertyNative, essential);
    }

    JNI_METHOD(void, removePropertyNative)(JNIEnv *env, jobject obj, jobject property)
    {
        NATIVE_IMAGE_ITEM(nativeHandle, obj);
        NATIVE_ITEM_PROPERTY(propertyNative, property);
        nativeHandle->removeProperty(propertyNative);
    }

    JNI_METHOD(jint, getMetadataCountNative)(JNIEnv *env, jobject obj)
    {
        NATIVE_IMAGE_ITEM(nativeHandle, obj);
        return nativeHandle->getMetadataCount();
    }

    JNI_METHOD(jobject, getMetadataNative)(JNIEnv *env, jobject obj, jint index)
    {
        NATIVE_IMAGE_ITEM(nativeHandle, obj);
        return getJavaItem(env, getJavaHEIF(env, obj), nativeHandle->getMetadata(index));
    }

    JNI_METHOD(void, addMetadataNative)(JNIEnv *env, jobject obj, jobject item)
    {
        NATIVE_IMAGE_ITEM(nativeHandle, obj);
        NATIVE_META_ITEM(nativeItem, item);
        nativeHandle->addMetadata(nativeItem);
    }

    JNI_METHOD(void, removeMetadataNative)(JNIEnv *env, jobject obj, jobject item)
    {
        NATIVE_IMAGE_ITEM(nativeHandle, obj);
        NATIVE_META_ITEM(nativeItem, item);
        nativeHandle->removeMetadata(nativeItem);
    }
}
