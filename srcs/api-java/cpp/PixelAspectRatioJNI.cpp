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
#include "DescriptiveProperty.h"
#include "Helpers.h"

#define CLASS_NAME PixelAspectRatioProperty

extern "C"
{
    JNI_METHOD_ARG(jlong, createContextNative, jobject javaHEIF)
    {
        NATIVE_HEIF(nativeHeif, javaHEIF);
        HEIFPP::PixelAspectRatioProperty *nativeObject = new HEIFPP::PixelAspectRatioProperty(nativeHeif);
        nativeObject->setContext(static_cast<void*>(env->NewGlobalRef(self)));
        return reinterpret_cast<jlong>(nativeObject);
    }

    JNI_METHOD(jint, getRelativeWidthNative)
    {
        NATIVE_PIXEL_ASPECT_RATIO_PROPERTY(nativeHandle, self);
        return static_cast<jint>(nativeHandle->mPixelAspectRatio.relativeWidth);
    }

    JNI_METHOD(jint, getRelativeHeightNative)
    {
        NATIVE_PIXEL_ASPECT_RATIO_PROPERTY(nativeHandle, self);
        return static_cast<jint>(nativeHandle->mPixelAspectRatio.relativeHeight);
    }

    JNI_METHOD_ARG(void, setRelativeWidthNative, jint relativeWidth)
    {
        NATIVE_PIXEL_ASPECT_RATIO_PROPERTY(nativeHandle, self);
        nativeHandle->mPixelAspectRatio.relativeWidth = static_cast<uint32_t>(relativeWidth);
    }

    JNI_METHOD_ARG(void, setRelativeHeightNative, jint relativeHeight)
    {
        NATIVE_PIXEL_ASPECT_RATIO_PROPERTY(nativeHandle, self);
        nativeHandle->mPixelAspectRatio.relativeHeight = static_cast<uint32_t >(relativeHeight);
    }
}
