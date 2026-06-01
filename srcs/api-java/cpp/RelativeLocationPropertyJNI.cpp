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

#include <DescriptiveProperty.h>
#include <jni.h>

#include "Helpers.h"
#include "heifcommondatatypes.h"
#include "jni_md.h"
#include <stdint.h>

#define CLASS_NAME RelativeLocationProperty

extern "C"
{
    JNI_METHOD_ARG(jlong, createContextNative, jobject javaHEIF)
    {
        JNI_TRY
        {
        UNUSED(self);
        NATIVE_HEIF(nativeHeif, javaHEIF);
        auto* nativeObject = new HEIFPP::RelativeLocationProperty(nativeHeif);
        return reinterpret_cast<jlong>(nativeObject);
            }
        JNI_CATCH_RETURN(0, "RelativeLocationProperty.createContextNative")
    }

    JNI_METHOD_ARG(void, setHorizontalOffsetNative, jint offset)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        nativeSelf->mRelativeLocation.horizontalOffset = static_cast<uint32_t>(offset);
            }
        JNI_CATCH_RETURN_VOID("RelativeLocationProperty.setHorizontalOffsetNative")
    }

    JNI_METHOD_ARG(void, setVerticalOffsetNative, jint offset)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        nativeSelf->mRelativeLocation.verticalOffset = static_cast<uint32_t>(offset);
            }
        JNI_CATCH_RETURN_VOID("RelativeLocationProperty.setVerticalOffsetNative")
    }

    JNI_METHOD(int, getHorizontalOffsetNative)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        return static_cast<jint>(nativeSelf->mRelativeLocation.horizontalOffset);
            }
        JNI_CATCH_RETURN(0, "RelativeLocationProperty.getHorizontalOffsetNative")
    }

    JNI_METHOD(int, getVerticalOffsetNative)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        return static_cast<jint>(nativeSelf->mRelativeLocation.verticalOffset);
            }
        JNI_CATCH_RETURN(0, "RelativeLocationProperty.getVerticalOffsetNative")
    }
}
