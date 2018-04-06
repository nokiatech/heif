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
#define JNI_METHOD(return_type, method_name) \
    JNIEXPORT return_type JNICALL Java_com_nokia_heif_PixelAspectRatioProperty_##method_name
#include "DescriptiveProperty.h"
#include "Helpers.h"
extern "C"
{
    JNI_METHOD(jlong, createContextNative)(JNIEnv *env, jobject obj, jobject javaHEIF)
    {
        NATIVE_HEIF(nativeHeif, javaHEIF);
        HEIFPP::PixelAspectRatioProperty *nativeObject = new HEIFPP::PixelAspectRatioProperty(nativeHeif);
        nativeObject->setContext((void *) env->NewGlobalRef(obj));
        return (jlong) nativeObject;
    }

    JNI_METHOD(jint, getRelativeWidthNative)(JNIEnv *env, jobject obj)
    {
        NATIVE_PIXEL_ASPECT_RATIO_PROPERTY(nativeHandle, obj);
        return nativeHandle->mPixelAspectRatio.relativeWidth;
    }

    JNI_METHOD(jint, getRelativeHeightNative)(JNIEnv *env, jobject obj)
    {
        NATIVE_PIXEL_ASPECT_RATIO_PROPERTY(nativeHandle, obj);
        return nativeHandle->mPixelAspectRatio.relativeHeight;
    }

    JNI_METHOD(void, setRelativeWidthNative)(JNIEnv *env, jobject obj, jint relativeWidth)
    {
        NATIVE_PIXEL_ASPECT_RATIO_PROPERTY(nativeHandle, obj);
        nativeHandle->mPixelAspectRatio.relativeWidth = relativeWidth;
    }

    JNI_METHOD(void, setRelativeHeightNative)(JNIEnv *env, jobject obj, jint relativeHeight)
    {
        NATIVE_PIXEL_ASPECT_RATIO_PROPERTY(nativeHandle, obj);
        nativeHandle->mPixelAspectRatio.relativeHeight = relativeHeight;
    }
}
