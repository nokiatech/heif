/*
 * This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2021 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
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
#include "OverlayImageItem.h"

#define CLASS_NAME OverlayImageItem

extern "C"
{
    JNI_METHOD_ARG(jlong, createContextNative, jobject javaHEIF)
    {
        NATIVE_HEIF(nativeHeif, javaHEIF);
        HEIFPP::Overlay *nativeObject = new HEIFPP::Overlay(nativeHeif);
        nativeObject->setContext(static_cast<void*>(env->NewGlobalRef(self)));
        return reinterpret_cast<jlong>(nativeObject);
    }

    JNI_METHOD(jint, imageCountNative)
    {
        NATIVE_OVERLAY_IMAGE_ITEM(nativeHandle, self);
        return static_cast<jint>(nativeHandle->imageCount());
    }

    JNI_METHOD_ARG(jobject, getImageNative, jint index)
    {
        NATIVE_OVERLAY_IMAGE_ITEM(nativeHandle, self);


        const jclass javaClass = env->GetObjectClass(self);

        const jmethodID createMethodId =
            env->GetMethodID(javaClass, "createOverlayedImage",
                             "(Lcom/nokia/heif/ImageItem;II)Lcom/nokia/heif/OverlayImageItem$OverlayedImage;");
        HEIF::Overlay::Offset overlayOffset;
        HEIFPP::ImageItem *image = nativeHandle->getImage(static_cast<uint32_t>(index), overlayOffset);
        env->DeleteLocalRef(javaClass);
        return env->CallObjectMethod(self, createMethodId, getJavaItem(env, getJavaHEIF(env, self), image),
                                     overlayOffset.horizontal, overlayOffset.vertical);
    }

    JNI_METHOD(jobject, getBackgroundColourNative)
    {
        NATIVE_OVERLAY_IMAGE_ITEM(nativeHandle, self);
        const jclass javaClass = env->GetObjectClass(self);

        const jmethodID createMethodId = env->GetMethodID(javaClass, "createBackgroundColour",
                                                          "(IIII)Lcom/nokia/heif/OverlayImageItem$BackgroundColour;");

        env->DeleteLocalRef(javaClass);
        return env->CallObjectMethod(self, createMethodId, nativeHandle->r(), nativeHandle->g(), nativeHandle->b(),
                                     nativeHandle->a());
    }

    JNI_METHOD_ARG(void, setBackgroundColourNative, jint r, jint g, jint b, jint a)
    {
        NATIVE_OVERLAY_IMAGE_ITEM(nativeHandle, self);
        nativeHandle->setR(static_cast<uint16_t>(r));
        nativeHandle->setG(static_cast<uint16_t>(g));
        nativeHandle->setB(static_cast<uint16_t>(b));
        nativeHandle->setA(static_cast<uint16_t>(a));
    }

    JNI_METHOD_ARG(void, addImageNative, jobject image, jint horizontalOffset, jint verticalOffset)
    {
        NATIVE_OVERLAY_IMAGE_ITEM(nativeHandle, self);
        NATIVE_IMAGE_ITEM(nativeImage, image);
        HEIF::Overlay::Offset offset;
        offset.vertical   = verticalOffset;
        offset.horizontal = horizontalOffset;
        nativeHandle->addImage(nativeImage, offset);
    }

    JNI_METHOD_ARG(void, setImageNative, jint index, jobject image, jint horizontalOffset, jint verticalOffset)
    {
        NATIVE_OVERLAY_IMAGE_ITEM(nativeHandle, self);
        NATIVE_IMAGE_ITEM(nativeImage, image);
        HEIF::Overlay::Offset offset;
        offset.vertical   = verticalOffset;
        offset.horizontal = horizontalOffset;
        CHECK_ERROR(nativeHandle->setImage(static_cast<uint32_t>(index),
                                           nativeImage, offset), "setImage failed");
    }

    JNI_METHOD_ARG(void, removeImageByIndexNative, jint index)
    {
        NATIVE_OVERLAY_IMAGE_ITEM(nativeHandle, self);
        CHECK_ERROR(nativeHandle->removeImage(static_cast<uint32_t>(index)), "removeImage failed");
    }

    JNI_METHOD_ARG(void, removeImageNative, jobject image)
    {
        NATIVE_OVERLAY_IMAGE_ITEM(nativeHandle, self);
        NATIVE_IMAGE_ITEM(nativeImage, image);
        nativeHandle->removeImage(nativeImage);
    }
}