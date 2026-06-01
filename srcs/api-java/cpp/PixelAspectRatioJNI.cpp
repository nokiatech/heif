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

#include "DescriptiveProperty.h"
#include "Helpers.h"
#include "heifcommondatatypes.h"
#include "jni_md.h"
#include <stdint.h>

#define CLASS_NAME PixelAspectRatioProperty

extern "C"
{
    JNI_METHOD_ARG(jlong, createContextNative, jobject javaHEIF)
    {
        JNI_TRY
        {
        UNUSED(self);
        NATIVE_HEIF(nativeHeif, javaHEIF);
        auto* nativeObject = new HEIFPP::PixelAspectRatioProperty(nativeHeif);
        return reinterpret_cast<jlong>(nativeObject);
            }
        JNI_CATCH_RETURN(0, "PixelAspectRatioProperty.createContextNative")
    }

    JNI_METHOD(jint, getRelativeWidthNative)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        return static_cast<jint>(nativeSelf->mPixelAspectRatio.relativeWidth);
            }
        JNI_CATCH_RETURN(0, "PixelAspectRatioProperty.getRelativeWidthNative")
    }

    JNI_METHOD(jint, getRelativeHeightNative)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        return static_cast<jint>(nativeSelf->mPixelAspectRatio.relativeHeight);
            }
        JNI_CATCH_RETURN(0, "PixelAspectRatioProperty.getRelativeHeightNative")
    }

    JNI_METHOD_ARG(void, setRelativeWidthNative, jint relativeWidth)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        nativeSelf->mPixelAspectRatio.relativeWidth = static_cast<uint32_t>(relativeWidth);
            }
        JNI_CATCH_RETURN_VOID("PixelAspectRatioProperty.setRelativeWidthNative")
    }

    JNI_METHOD_ARG(void, setRelativeHeightNative, jint relativeHeight)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        nativeSelf->mPixelAspectRatio.relativeHeight = static_cast<uint32_t>(relativeHeight);
            }
        JNI_CATCH_RETURN_VOID("PixelAspectRatioProperty.setRelativeHeightNative")
    }
}
