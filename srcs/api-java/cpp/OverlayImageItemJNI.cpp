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

#include "Heif.h"
#include "Helpers.h"
#include "OverlayImageItem.h"
#include "heifcommondatatypes.h"
#include "jni_md.h"
#include <stdint.h>

#define CLASS_NAME OverlayImageItem

extern "C"
{
    JNI_METHOD_ARG(jlong, createContextNative, jobject javaHEIF)
    {
        JNI_TRY
        {
        UNUSED(self);
        NATIVE_HEIF(nativeHeif, javaHEIF);
        auto* nativeObject = nativeHeif->create<HEIFPP::OverlayImageItem>();
        return reinterpret_cast<jlong>(nativeObject);
            }
        JNI_CATCH_RETURN(0, "OverlayImageItem.createContextNative")
    }

    JNI_METHOD(jint, imageCountNative)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        return static_cast<jint>(nativeSelf->imageCount());
            }
        JNI_CATCH_RETURN(0, "OverlayImageItem.imageCountNative")
    }

    JNI_METHOD_ARG(jobject, getImageNative, jint index)
    {
        JNI_TRY
        {
        NATIVE_SELF;


        const jclass javaClass = env->GetObjectClass(self);

        const jmethodID createMethodId =
            env->GetMethodID(javaClass, "createOverlayedImage",
                             "(Lcom/nokia/heif/ImageItem;II)Lcom/nokia/heif/OverlayImageItem$OverlayedImage;");
        HEIF::Overlay::Offset overlayOffset;
        HEIFPP::ImageItem* image = nativeSelf->getImage(static_cast<uint32_t>(index), overlayOffset);
        env->DeleteLocalRef(javaClass);
        return env->CallObjectMethod(self, createMethodId, getJavaItem(env, getJavaHEIF(env, self), image),
                                     overlayOffset.horizontal, overlayOffset.vertical);
            }
        JNI_CATCH_RETURN(nullptr, "OverlayImageItem.getImageNative")
    }

    JNI_METHOD(jobject, getBackgroundColourNative)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        const jclass javaClass = env->GetObjectClass(self);

        const jmethodID createMethodId = env->GetMethodID(javaClass, "createBackgroundColour",
                                                          "(IIII)Lcom/nokia/heif/OverlayImageItem$BackgroundColour;");

        env->DeleteLocalRef(javaClass);
        return env->CallObjectMethod(self, createMethodId, nativeSelf->r(), nativeSelf->g(), nativeSelf->b(),
                                     nativeSelf->a());
            }
        JNI_CATCH_RETURN(nullptr, "OverlayImageItem.getBackgroundColourNative")
    }

    JNI_METHOD_ARG(void, setBackgroundColourNative, jint r, jint g, jint b, jint a)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        nativeSelf->setR(static_cast<uint16_t>(r));
        nativeSelf->setG(static_cast<uint16_t>(g));
        nativeSelf->setB(static_cast<uint16_t>(b));
        nativeSelf->setA(static_cast<uint16_t>(a));
            }
        JNI_CATCH_RETURN_VOID("OverlayImageItem.setBackgroundColourNative")
    }

    JNI_METHOD_ARG(void, addImageNative, jobject image, jint horizontalOffset, jint verticalOffset)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        NATIVE_IMAGE_ITEM(nativeImage, image);
        HEIF::Overlay::Offset offset;
        offset.vertical   = verticalOffset;
        offset.horizontal = horizontalOffset;
        nativeSelf->addImage(nativeImage, offset);
            }
        JNI_CATCH_RETURN_VOID("OverlayImageItem.addImageNative")
    }

    JNI_METHOD_ARG(void, setImageNative, jint index, jobject image, jint horizontalOffset, jint verticalOffset)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        NATIVE_IMAGE_ITEM(nativeImage, image);
        HEIF::Overlay::Offset offset;
        offset.vertical   = verticalOffset;
        offset.horizontal = horizontalOffset;
        CHECK_ERROR(nativeSelf->setImage(static_cast<uint32_t>(index), nativeImage, offset), "setImage failed");
            }
        JNI_CATCH_RETURN_VOID("OverlayImageItem.setImageNative")
    }

    JNI_METHOD_ARG(void, removeImageByIndexNative, jint index)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        CHECK_ERROR(nativeSelf->removeImage(static_cast<uint32_t>(index)), "removeImage failed");
            }
        JNI_CATCH_RETURN_VOID("OverlayImageItem.removeImageByIndexNative")
    }

    JNI_METHOD_ARG(void, removeImageNative, jobject image)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        NATIVE_IMAGE_ITEM(nativeImage, image);
        nativeSelf->removeImage(nativeImage);
            }
        JNI_CATCH_RETURN_VOID("OverlayImageItem.removeImageNative")
    }
}
