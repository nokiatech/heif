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
#include "VideoTrack.h"
#include "jni_md.h"


#define CLASS_NAME VideoTrack

extern "C"
{
    JNI_METHOD_ARG(jlong, createContextNative, jobject javaHEIF)
    {
        JNI_TRY
        {
        UNUSED(self);
        NATIVE_HEIF(nativeHeif, javaHEIF);
        auto* nativeObject = new HEIFPP::VideoTrack(nativeHeif);
        return reinterpret_cast<jlong>(nativeObject);
            }
        JNI_CATCH_RETURN(0, "VideoTrack.createContextNative")
    }

    JNI_METHOD_ARG(void, addSampleNative, jobject sample)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        NATIVE_VIDEO_SAMPLE(nativeSample, sample);
        nativeSelf->addSample(nativeSample);
            }
        JNI_CATCH_RETURN_VOID("VideoTrack.addSampleNative")
    }

    JNI_METHOD(jint, getDisplayWidthNative)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        return static_cast<jint>(nativeSelf->getDisplayWidth());
            }
        JNI_CATCH_RETURN(0, "VideoTrack.getDisplayWidthNative")
    }

    JNI_METHOD(jint, getDisplayHeightNative)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        return static_cast<jint>(nativeSelf->getDisplayHeight());
            }
        JNI_CATCH_RETURN(0, "VideoTrack.getDisplayHeightNative")
    }
}
