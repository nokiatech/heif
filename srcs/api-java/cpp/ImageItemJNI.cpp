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
        NATIVE_SELF;
        return static_cast<jint>(nativeSelf->width());
    }

    JNI_METHOD(jint, getHeightNative)
    {
        NATIVE_SELF;
        return static_cast<jint>(nativeSelf->height());
    }

    JNI_METHOD_ARG(void, setSizeNative, jint width, jint height)
    {
        NATIVE_SELF;
        nativeSelf->setSize(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
    }

    JNI_METHOD(jboolean, getHiddenNative)
    {
        NATIVE_SELF;
        return static_cast<jboolean>(nativeSelf->isHidden());
    }

    JNI_METHOD_ARG(void, setHiddenNative, jboolean hidden)
    {
        NATIVE_SELF;
        nativeSelf->setHidden(hidden != 0);
    }

    JNI_METHOD(jint, getThumbnailCountNative)
    {
        NATIVE_SELF;
        return static_cast<jint>(nativeSelf->getThumbnailCount());
    }

    JNI_METHOD_ARG(jobject, getThumbnailNative, jint index)
    {
        NATIVE_SELF;
        return getJavaItem(env, getJavaHEIF(env, self),
                           nativeSelf->getThumbnail(static_cast<uint32_t>(index)));
    }

    JNI_METHOD_ARG(void, addThumbnailNative, jobject thumbnail)
    {
        NATIVE_SELF;
        NATIVE_IMAGE_ITEM(nativeThumbnail, thumbnail);
        nativeSelf->addThumbnail(nativeThumbnail);
    }

    JNI_METHOD_ARG(void, removeThumbnailNative, jobject thumbnail)
    {
        NATIVE_SELF;
        NATIVE_IMAGE_ITEM(nativeThumbnail, thumbnail);
        nativeSelf->removeThumbnail(nativeThumbnail);
    }

    JNI_METHOD(jint, getAuxiliaryCountNative)
    {
        NATIVE_SELF;
        return static_cast<jint>(nativeSelf->getAuxCount());
    }

    JNI_METHOD_ARG(jobject, getAuxiliaryNative, jint index)
    {
        NATIVE_SELF;
        return getJavaItem(env, getJavaHEIF(env, self),
                           nativeSelf->getAux(static_cast<uint32_t>(index)));
    }

    JNI_METHOD_ARG(void, addAuxiliaryImageNative, jobject auxiliary)
    {
        NATIVE_SELF;
        NATIVE_IMAGE_ITEM(nativeAuxiliary, auxiliary);
        nativeSelf->addAuxImage(nativeAuxiliary);
    }

    JNI_METHOD_ARG(void, removeAuxiliaryImageNative, jobject auxiliary)
    {
        NATIVE_SELF;
        NATIVE_IMAGE_ITEM(nativeAuxiliary, auxiliary);
        nativeSelf->removeAuxImage(nativeAuxiliary);
    }

    JNI_METHOD(jint, transformativePropertyCountNative)
    {
        NATIVE_SELF;
        return static_cast<jint>(nativeSelf->transformativePropertyCount());
    }

    JNI_METHOD_ARG(jobject, getTransformativePropertyNative, jint index)
    {
        NATIVE_SELF;
        return getJavaItemProperty(env, getJavaHEIF(env, self),
                                   nativeSelf->getTransformativeProperty(static_cast<uint32_t>(index)));
    }

    JNI_METHOD(jobject, getPixelAspectRatioPropertyNative)
    {
        NATIVE_SELF;
        return getJavaItemProperty(env, getJavaHEIF(env, self), nativeSelf->pixelAspectRatio());
    }

    JNI_METHOD(jobject, getColourInformationPropertyNative)
    {
        NATIVE_SELF;
        return getJavaItemProperty(env, getJavaHEIF(env, self), nativeSelf->colourInformation());
    }

    JNI_METHOD(jobject, getPixelInformationPropertyNative)
    {
        NATIVE_SELF;
        return getJavaItemProperty(env, getJavaHEIF(env, self), nativeSelf->pixelInformation());
    }

    JNI_METHOD(jobject, getRelativeLocationPropertyNative)
    {
        NATIVE_SELF;
        return getJavaItemProperty(env, getJavaHEIF(env, self), nativeSelf->relativeLocation());
    }

    JNI_METHOD(jobject, getAuxiliaryPropertyNative)
    {
        NATIVE_SELF;
        return getJavaItemProperty(env, getJavaHEIF(env, self), nativeSelf->aux());
    }

    JNI_METHOD_ARG(void, addPropertyNative, jobject property, jboolean essential)
    {
        NATIVE_SELF;
        NATIVE_ITEM_PROPERTY(propertyNative, property);
        nativeSelf->addProperty(propertyNative, essential != 0);
    }

    JNI_METHOD_ARG(void, removePropertyNative, jobject property)
    {
        NATIVE_SELF;
        NATIVE_ITEM_PROPERTY(propertyNative, property);
        nativeSelf->removeProperty(propertyNative);
    }

    JNI_METHOD(jint, getMetadataCountNative)
    {
        NATIVE_SELF;
        return static_cast<jint>(nativeSelf->getMetadataCount());
    }

    JNI_METHOD_ARG(jobject, getMetadataNative, jint index)
    {
        NATIVE_SELF;
        return getJavaItem(env, getJavaHEIF(env, self),
                           nativeSelf->getMetadata(static_cast<uint32_t>(index)));
    }

    JNI_METHOD_ARG(void, addMetadataNative, jobject item)
    {
        NATIVE_SELF;
        NATIVE_META_ITEM(nativeItem, item);
        nativeSelf->addMetadata(nativeItem);
    }

    JNI_METHOD_ARG(void, removeMetadataNative, jobject item)
    {
        NATIVE_SELF;
        NATIVE_META_ITEM(nativeItem, item);
        nativeSelf->removeMetadata(nativeItem);
    }
}
