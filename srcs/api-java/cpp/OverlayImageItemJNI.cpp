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
#include "OverlayImageItem.h"
#define JNI_METHOD(return_type, method_name) \
    JNIEXPORT return_type JNICALL Java_com_nokia_heif_OverlayImageItem_##method_name
extern "C"
{
    JNI_METHOD(jlong, createContextNative)(JNIEnv *env, jobject obj, jobject javaHEIF)
    {
        NATIVE_HEIF(nativeHeif, javaHEIF);
        HEIFPP::Overlay *nativeObject = new HEIFPP::Overlay(nativeHeif);
        nativeObject->setContext((void *) env->NewGlobalRef(obj));
        return (jlong) nativeObject;
    }

    JNI_METHOD(jint, imageCountNative)(JNIEnv *env, jobject obj)
    {
        NATIVE_OVERLAY_IMAGE_ITEM(nativeHandle, obj);
        return nativeHandle->imageCount();
    }

    JNI_METHOD(jobject, getImageNative)(JNIEnv *env, jobject obj, int index)
    {
        NATIVE_OVERLAY_IMAGE_ITEM(nativeHandle, obj);


        const jclass javaClass = env->GetObjectClass(obj);

        const jmethodID createMethodId =
            env->GetMethodID(javaClass, "createOverlayedImage",
                             "(Lcom/nokia/heif/ImageItem;II)Lcom/nokia/heif/OverlayImageItem$OverlayedImage;");
        HEIF::Overlay::Offset overlayOffset;
        HEIFPP::ImageItem *image = nativeHandle->getImage(index, overlayOffset);
        env->DeleteLocalRef(javaClass);
        return env->CallObjectMethod(obj, createMethodId, getJavaItem(env, getJavaHEIF(env, obj), image),
                                     overlayOffset.horizontal, overlayOffset.vertical);
    }

    JNI_METHOD(jobject, getBackgroundColourNative)(JNIEnv *env, jobject obj)
    {
        NATIVE_OVERLAY_IMAGE_ITEM(nativeHandle, obj);
        const jclass javaClass = env->GetObjectClass(obj);

        const jmethodID createMethodId = env->GetMethodID(javaClass, "createBackgroundColour",
                                                          "(IIII)Lcom/nokia/heif/OverlayImageItem$BackgroundColour;");

        env->DeleteLocalRef(javaClass);
        return env->CallObjectMethod(obj, createMethodId, nativeHandle->r(), nativeHandle->g(), nativeHandle->b(),
                                     nativeHandle->a());
    }

    JNI_METHOD(void, setBackgroundColourNative)(JNIEnv *env, jobject obj, jint r, jint g, jint b, jint a)
    {
        NATIVE_OVERLAY_IMAGE_ITEM(nativeHandle, obj);
        nativeHandle->setR(r);
        nativeHandle->setG(g);
        nativeHandle->setB(b);
        nativeHandle->setA(a);
    }

    JNI_METHOD(void, addImageNative)(JNIEnv *env, jobject obj, jobject image, int horizontalOffset, int verticalOffset)
    {
        NATIVE_OVERLAY_IMAGE_ITEM(nativeHandle, obj);
        NATIVE_IMAGE_ITEM(nativeImage, image);
        HEIF::Overlay::Offset offset;
        offset.vertical   = verticalOffset;
        offset.horizontal = horizontalOffset;
        nativeHandle->addImage(nativeImage, offset);
    }

    JNI_METHOD(void, setImageNative)
    (JNIEnv *env, jobject obj, int index, jobject image, int horizontalOffset, int verticalOffset)
    {
        NATIVE_OVERLAY_IMAGE_ITEM(nativeHandle, obj);
        NATIVE_IMAGE_ITEM(nativeImage, image);
        HEIF::Overlay::Offset offset;
        offset.vertical   = verticalOffset;
        offset.horizontal = horizontalOffset;
        CHECK_ERROR(nativeHandle->setImage(index, nativeImage, offset), "setImage failed");
    }

    JNI_METHOD(void, removeImageByIndexNative)(JNIEnv *env, jobject obj, int index)
    {
        NATIVE_OVERLAY_IMAGE_ITEM(nativeHandle, obj);
        CHECK_ERROR(nativeHandle->removeImage(index), "removeImage failed");
    }

    JNI_METHOD(void, removeImageNative)(JNIEnv *env, jobject obj, jobject image)
    {
        NATIVE_OVERLAY_IMAGE_ITEM(nativeHandle, obj);
        NATIVE_IMAGE_ITEM(nativeImage, image);
        nativeHandle->removeImage(nativeImage);
    }
}