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
#include "DescriptiveProperty.h"
#include "Helpers.h"

#define CLASS_NAME PixelAspectRatioProperty

extern "C"
{
    JNI_METHOD_ARG(jlong, createContextNative, jobject javaHEIF)
    {
        UNUSED(self);
        NATIVE_HEIF(nativeHeif, javaHEIF);
        HEIFPP::PixelAspectRatioProperty *nativeObject = new HEIFPP::PixelAspectRatioProperty(nativeHeif);
        return reinterpret_cast<jlong>(nativeObject);
    }

    JNI_METHOD(jint, getRelativeWidthNative)
    {
        NATIVE_SELF;
        return static_cast<jint>(nativeSelf->mPixelAspectRatio.relativeWidth);
    }

    JNI_METHOD(jint, getRelativeHeightNative)
    {
        NATIVE_SELF;
        return static_cast<jint>(nativeSelf->mPixelAspectRatio.relativeHeight);
    }

    JNI_METHOD_ARG(void, setRelativeWidthNative, jint relativeWidth)
    {
        NATIVE_SELF;
        nativeSelf->mPixelAspectRatio.relativeWidth = static_cast<uint32_t>(relativeWidth);
    }

    JNI_METHOD_ARG(void, setRelativeHeightNative, jint relativeHeight)
    {
        NATIVE_SELF;
        nativeSelf->mPixelAspectRatio.relativeHeight = static_cast<uint32_t>(relativeHeight);
    }
}
