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
#define CLASS_NAME ImageItem

extern "C"
{
    JNI_METHOD(jint, getWidthNative)
    {
        NATIVE_IMAGE_ITEM(nativeHandle, self);
        return static_cast<jint>(nativeHandle->width());
    }

    JNI_METHOD(jint, getHeightNative)
    {
        NATIVE_IMAGE_ITEM(nativeHandle, self);
        return static_cast<jint>(nativeHandle->height());
    }

    JNI_METHOD_ARG(void, setSizeNative, jint width, jint height)
    {
        NATIVE_IMAGE_ITEM(nativeHandle, self);
        nativeHandle->setSize(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
    }

    JNI_METHOD(jboolean, getHiddenNative)
    {
        NATIVE_IMAGE_ITEM(nativeHandle, self);
        return static_cast<jboolean>(nativeHandle->isHidden());
    }

    JNI_METHOD_ARG(void, setHiddenNative, jboolean hidden)
    {
        NATIVE_IMAGE_ITEM(nativeHandle, self);
        nativeHandle->setHidden(hidden);
    }

    JNI_METHOD(jint, getThumbnailCountNative)
    {
        NATIVE_IMAGE_ITEM(nativeHandle, self);
        return static_cast<jint>(nativeHandle->getThumbnailCount());
    }

    JNI_METHOD_ARG(jobject, getThumbnailNative, jint index)
    {
        NATIVE_IMAGE_ITEM(nativeHandle, self);
        return getJavaItem(env, getJavaHEIF(env, self),
                           nativeHandle->getThumbnail(static_cast<uint32_t>(index)));
    }

    JNI_METHOD_ARG(void, addThumbnailNative, jobject thumbnail)
    {
        NATIVE_IMAGE_ITEM(nativeHandle, self);
        NATIVE_IMAGE_ITEM(nativeThumbnail, thumbnail);
        nativeHandle->addThumbnail(nativeThumbnail);
    }

    JNI_METHOD_ARG(void, removeThumbnailNative, jobject thumbnail)
    {
        NATIVE_IMAGE_ITEM(nativeHandle, self);
        NATIVE_IMAGE_ITEM(nativeThumbnail, thumbnail);
        nativeHandle->removeThumbnail(nativeThumbnail);
    }

    JNI_METHOD(jint, getAuxiliaryCountNative)
    {
        NATIVE_IMAGE_ITEM(nativeHandle, self);
        return static_cast<jint>(nativeHandle->getAuxCount());
    }

    JNI_METHOD_ARG(jobject, getAuxiliaryNative, jint index)
    {
        NATIVE_IMAGE_ITEM(nativeHandle, self);
        return getJavaItem(env, getJavaHEIF(env, self),
                           nativeHandle->getAux(static_cast<uint32_t>(index)));
    }

    JNI_METHOD_ARG(void, addAuxiliaryImageNative, jobject auxiliary)
    {
        NATIVE_IMAGE_ITEM(nativeHandle, self);
        NATIVE_IMAGE_ITEM(nativeAuxiliary, auxiliary);
        nativeHandle->addAuxImage(nativeAuxiliary);
    }

    JNI_METHOD_ARG(void, removeAuxiliaryImageNative, jobject auxiliary)
    {
        NATIVE_IMAGE_ITEM(nativeHandle, self);
        NATIVE_IMAGE_ITEM(nativeAuxiliary, auxiliary);
        nativeHandle->removeAuxImage(nativeAuxiliary);
    }

    JNI_METHOD(jint, transformativePropertyCountNative)
    {
        NATIVE_IMAGE_ITEM(nativeHandle, self);
        return static_cast<jint>(nativeHandle->transformativePropertyCount());
    }

    JNI_METHOD_ARG(jobject, getTransformativePropertyNative, jint index)
    {
        NATIVE_IMAGE_ITEM(nativeHandle, self);
        return getJavaItemProperty(env, getJavaHEIF(env, self),
                                   nativeHandle->getTransformativeProperty(static_cast<uint32_t>(index)));
    }

    JNI_METHOD(jobject, getPixelAspectRatioPropertyNative)
    {
        NATIVE_IMAGE_ITEM(nativeHandle, self);
        return getJavaItemProperty(env, getJavaHEIF(env, self), nativeHandle->pixelAspectRatio());
    }

    JNI_METHOD(jobject, getColourInformationPropertyNative)
    {
        NATIVE_IMAGE_ITEM(nativeHandle, self);
        return getJavaItemProperty(env, getJavaHEIF(env, self), nativeHandle->colourInformation());
    }

    JNI_METHOD(jobject, getPixelInformationPropertyNative)
    {
        NATIVE_IMAGE_ITEM(nativeHandle, self);
        return getJavaItemProperty(env, getJavaHEIF(env, self), nativeHandle->pixelInformation());
    }

    JNI_METHOD(jobject, getRelativeLocationPropertyNative)
    {
        NATIVE_IMAGE_ITEM(nativeHandle, self);
        return getJavaItemProperty(env, getJavaHEIF(env, self), nativeHandle->relativeLocation());
    }

    JNI_METHOD(jobject, getAuxiliaryPropertyNative)
    {
        NATIVE_IMAGE_ITEM(nativeHandle, self);
        return getJavaItemProperty(env, getJavaHEIF(env, self), nativeHandle->aux());
    }

    JNI_METHOD_ARG(void, addPropertyNative, jobject property, jboolean essential)
    {
        NATIVE_IMAGE_ITEM(nativeHandle, self);
        NATIVE_ITEM_PROPERTY(propertyNative, property);
        nativeHandle->addProperty(propertyNative, essential);
    }

    JNI_METHOD_ARG(void, removePropertyNative, jobject property)
    {
        NATIVE_IMAGE_ITEM(nativeHandle, self);
        NATIVE_ITEM_PROPERTY(propertyNative, property);
        nativeHandle->removeProperty(propertyNative);
    }

    JNI_METHOD(jint, getMetadataCountNative)
    {
        NATIVE_IMAGE_ITEM(nativeHandle, self);
        return static_cast<jint>(nativeHandle->getMetadataCount());
    }

    JNI_METHOD_ARG(jobject, getMetadataNative, jint index)
    {
        NATIVE_IMAGE_ITEM(nativeHandle, self);
        return getJavaItem(env, getJavaHEIF(env, self),
                           nativeHandle->getMetadata(static_cast<uint32_t>(index)));
    }

    JNI_METHOD_ARG(void, addMetadataNative, jobject item)
    {
        NATIVE_IMAGE_ITEM(nativeHandle, self);
        NATIVE_META_ITEM(nativeItem, item);
        nativeHandle->addMetadata(nativeItem);
    }

    JNI_METHOD_ARG(void, removeMetadataNative, jobject item)
    {
        NATIVE_IMAGE_ITEM(nativeHandle, self);
        NATIVE_META_ITEM(nativeItem, item);
        nativeHandle->removeMetadata(nativeItem);
    }
}
