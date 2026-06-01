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

#define CLASS_NAME TargetOlsProperty

extern "C"
{
    JNI_METHOD_ARG(jlong, createContextNative, jobject javaHEIF)
    {
        JNI_TRY
        {
        UNUSED(self);
        NATIVE_HEIF(nativeHeif, javaHEIF);
        auto* nativeObject = new HEIFPP::TargetOlsProperty(nativeHeif);
        return reinterpret_cast<jlong>(nativeObject);
            }
        JNI_CATCH_RETURN(0, "TargetOlsProperty.createContextNative")
    }

    JNI_METHOD(jint, getTargetOlsIndexNative)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        return static_cast<jint>(nativeSelf->mTargetOls.targetOlsIndex);
            }
        JNI_CATCH_RETURN(0, "TargetOlsProperty.getTargetOlsIndexNative")
    }

    JNI_METHOD_ARG(void, setTargetOlsIndexNative, jint targetOlsIndex)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        nativeSelf->mTargetOls.targetOlsIndex = static_cast<uint16_t>(targetOlsIndex);
            }
        JNI_CATCH_RETURN_VOID("TargetOlsProperty.setTargetOlsIndexNative")
    }
}
