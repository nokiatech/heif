/*
 * This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2026 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
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
#include "jni_md.h"
#include <stdint.h>
#define CLASS_NAME ImageItem

extern "C"
{
    JNI_METHOD(jint, getWidthNative)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        return static_cast<jint>(nativeSelf->width());
            }
        JNI_CATCH_RETURN(0, "ImageItem.getWidthNative")
    }

    JNI_METHOD(jint, getHeightNative)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        return static_cast<jint>(nativeSelf->height());
            }
        JNI_CATCH_RETURN(0, "ImageItem.getHeightNative")
    }

    JNI_METHOD_ARG(void, setSizeNative, jint width, jint height)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        nativeSelf->setSize(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
            }
        JNI_CATCH_RETURN_VOID("ImageItem.setSizeNative")
    }

    JNI_METHOD(jboolean, getHiddenNative)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        return static_cast<jboolean>(nativeSelf->isHidden());
            }
        JNI_CATCH_RETURN(JNI_FALSE, "ImageItem.getHiddenNative")
    }

    JNI_METHOD_ARG(void, setHiddenNative, jboolean hidden)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        nativeSelf->setHidden(hidden != 0);
            }
        JNI_CATCH_RETURN_VOID("ImageItem.setHiddenNative")
    }

    JNI_METHOD(jint, getThumbnailCountNative)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        return static_cast<jint>(nativeSelf->getThumbnailCount());
            }
        JNI_CATCH_RETURN(0, "ImageItem.getThumbnailCountNative")
    }

    JNI_METHOD_ARG(jobject, getThumbnailNative, jint index)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        return getJavaItem(env, getJavaHEIF(env, self), nativeSelf->getThumbnail(static_cast<uint32_t>(index)));
            }
        JNI_CATCH_RETURN(nullptr, "ImageItem.getThumbnailNative")
    }

    JNI_METHOD_ARG(void, addThumbnailNative, jobject thumbnail)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        NATIVE_IMAGE_ITEM(nativeThumbnail, thumbnail);
        nativeSelf->addThumbnail(nativeThumbnail);
            }
        JNI_CATCH_RETURN_VOID("ImageItem.addThumbnailNative")
    }

    JNI_METHOD_ARG(void, removeThumbnailNative, jobject thumbnail)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        NATIVE_IMAGE_ITEM(nativeThumbnail, thumbnail);
        nativeSelf->removeThumbnail(nativeThumbnail);
            }
        JNI_CATCH_RETURN_VOID("ImageItem.removeThumbnailNative")
    }

    JNI_METHOD(jint, getAuxiliaryCountNative)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        return static_cast<jint>(nativeSelf->getAuxCount());
            }
        JNI_CATCH_RETURN(0, "ImageItem.getAuxiliaryCountNative")
    }

    JNI_METHOD_ARG(jobject, getAuxiliaryNative, jint index)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        return getJavaItem(env, getJavaHEIF(env, self), nativeSelf->getAux(static_cast<uint32_t>(index)));
            }
        JNI_CATCH_RETURN(nullptr, "ImageItem.getAuxiliaryNative")
    }

    JNI_METHOD_ARG(void, addAuxiliaryImageNative, jobject auxiliary)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        NATIVE_IMAGE_ITEM(nativeAuxiliary, auxiliary);
        nativeSelf->addAuxImage(nativeAuxiliary);
            }
        JNI_CATCH_RETURN_VOID("ImageItem.addAuxiliaryImageNative")
    }

    JNI_METHOD_ARG(void, removeAuxiliaryImageNative, jobject auxiliary)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        NATIVE_IMAGE_ITEM(nativeAuxiliary, auxiliary);
        nativeSelf->removeAuxImage(nativeAuxiliary);
            }
        JNI_CATCH_RETURN_VOID("ImageItem.removeAuxiliaryImageNative")
    }

    JNI_METHOD(jint, transformativePropertyCountNative)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        return static_cast<jint>(nativeSelf->transformativePropertyCount());
            }
        JNI_CATCH_RETURN(0, "ImageItem.transformativePropertyCountNative")
    }

    JNI_METHOD_ARG(jobject, getTransformativePropertyNative, jint index)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        return getJavaItemProperty(env, getJavaHEIF(env, self),
                                   nativeSelf->getTransformativeProperty(static_cast<uint32_t>(index)));
            }
        JNI_CATCH_RETURN(nullptr, "ImageItem.getTransformativePropertyNative")
    }

    JNI_METHOD(jobject, getPixelAspectRatioPropertyNative)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        return getJavaItemProperty(env, getJavaHEIF(env, self), nativeSelf->pixelAspectRatio());
            }
        JNI_CATCH_RETURN(nullptr, "ImageItem.getPixelAspectRatioPropertyNative")
    }

    JNI_METHOD(jobject, getColourInformationPropertyNative)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        return getJavaItemProperty(env, getJavaHEIF(env, self), nativeSelf->colourInformation());
            }
        JNI_CATCH_RETURN(nullptr, "ImageItem.getColourInformationPropertyNative")
    }

    JNI_METHOD(jobject, getPixelInformationPropertyNative)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        return getJavaItemProperty(env, getJavaHEIF(env, self), nativeSelf->pixelInformation());
            }
        JNI_CATCH_RETURN(nullptr, "ImageItem.getPixelInformationPropertyNative")
    }

    JNI_METHOD(jobject, getRelativeLocationPropertyNative)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        return getJavaItemProperty(env, getJavaHEIF(env, self), nativeSelf->relativeLocation());
            }
        JNI_CATCH_RETURN(nullptr, "ImageItem.getRelativeLocationPropertyNative")
    }

    JNI_METHOD(jobject, getAuxiliaryPropertyNative)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        return getJavaItemProperty(env, getJavaHEIF(env, self), nativeSelf->aux());
            }
        JNI_CATCH_RETURN(nullptr, "ImageItem.getAuxiliaryPropertyNative")
    }

    JNI_METHOD_ARG(void, addPropertyNative, jobject property, jboolean essential)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        NATIVE_ITEM_PROPERTY(propertyNative, property);
        nativeSelf->addProperty(propertyNative, essential != 0);
            }
        JNI_CATCH_RETURN_VOID("ImageItem.addPropertyNative")
    }

    JNI_METHOD_ARG(void, removePropertyNative, jobject property)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        NATIVE_ITEM_PROPERTY(propertyNative, property);
        nativeSelf->removeProperty(propertyNative);
            }
        JNI_CATCH_RETURN_VOID("ImageItem.removePropertyNative")
    }

    JNI_METHOD(jint, getMetadataCountNative)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        return static_cast<jint>(nativeSelf->getMetadataCount());
            }
        JNI_CATCH_RETURN(0, "ImageItem.getMetadataCountNative")
    }

    JNI_METHOD_ARG(jobject, getMetadataNative, jint index)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        return getJavaItem(env, getJavaHEIF(env, self), nativeSelf->getMetadata(static_cast<uint32_t>(index)));
            }
        JNI_CATCH_RETURN(nullptr, "ImageItem.getMetadataNative")
    }

    JNI_METHOD_ARG(void, addMetadataNative, jobject item)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        NATIVE_META_ITEM(nativeItem, item);
        nativeSelf->addMetadata(nativeItem);
            }
        JNI_CATCH_RETURN_VOID("ImageItem.addMetadataNative")
    }

    JNI_METHOD_ARG(void, removeMetadataNative, jobject item)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        NATIVE_META_ITEM(nativeItem, item);
        nativeSelf->removeMetadata(nativeItem);
            }
        JNI_CATCH_RETURN_VOID("ImageItem.removeMetadataNative")
    }
}
