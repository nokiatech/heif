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
#include "TransformativeProperty.h"
#include "heifcommondatatypes.h"
#include "jni_md.h"

#define CLASS_NAME MirrorProperty

extern "C"
{
    JNI_METHOD_ARG(jlong, createContextNative, jobject javaHEIF)
    {
        JNI_TRY
        {
        UNUSED(self);
        NATIVE_HEIF(nativeHeif, javaHEIF);
        auto* nativeObject = new HEIFPP::MirrorProperty(nativeHeif);
        return reinterpret_cast<jlong>(nativeObject);
            }
        JNI_CATCH_RETURN(0, "MirrorProperty.createContextNative")
    }

    JNI_METHOD(jboolean, getMirrorNative)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        return static_cast<jboolean>(nativeSelf->mMirror.horizontalAxis);
            }
        JNI_CATCH_RETURN(JNI_FALSE, "MirrorProperty.getMirrorNative")
    }

    JNI_METHOD_ARG(void, setMirrorNative, jboolean mirror)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        nativeSelf->mMirror.horizontalAxis = mirror != 0;
            }
        JNI_CATCH_RETURN_VOID("MirrorProperty.setMirrorNative")
    }
}
