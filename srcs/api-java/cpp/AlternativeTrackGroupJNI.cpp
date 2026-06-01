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

#include <AlternativeTrackGroup.h>
#include <jni.h>

#include "Helpers.h"
#include "jni_md.h"
#include <stdint.h>

#define CLASS_NAME AlternativeTrackGroup

extern "C"
{
    JNI_METHOD_ARG(jlong, createContextNative, jobject javaHEIF)
    {
        JNI_TRY
        {
        UNUSED(self);
        NATIVE_HEIF(nativeHeif, javaHEIF);
        auto* nativeObject = new HEIFPP::AlternativeTrackGroup(nativeHeif);
        return reinterpret_cast<jlong>(nativeObject);
            }
        JNI_CATCH_RETURN(0, "AlternativeTrackGroup.createContextNative")
    }

    JNI_METHOD(void, destroyContextNative)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        setNativeHandle(env, self, 0);
        delete nativeSelf;
            }
        JNI_CATCH_RETURN_VOID("AlternativeTrackGroup.destroyContextNative")
    }

    JNI_METHOD(jint, getTrackCountNative)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        return static_cast<jint>(nativeSelf->getTrackCount());
            }
        JNI_CATCH_RETURN(0, "AlternativeTrackGroup.getTrackCountNative")
    }

    JNI_METHOD_ARG(jobject, getTrackNative, jint index)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        return getJavaTrack(env, getJavaHEIF(env, self), nativeSelf->getTrack(static_cast<uint32_t>(index)));
            }
        JNI_CATCH_RETURN(nullptr, "AlternativeTrackGroup.getTrackNative")
    }

    JNI_METHOD_ARG(void, addTrackNative, jobject track)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        NATIVE_TRACK(nativeTrack, track);
        CHECK_ERROR(nativeSelf->addTrack(nativeTrack), "Adding to group failed");
            }
        JNI_CATCH_RETURN_VOID("AlternativeTrackGroup.addTrackNative")
    }

    JNI_METHOD_ARG(void, removeTrackNative, jobject track)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        NATIVE_TRACK(nativeTrack, track);
        nativeSelf->removeTrack(nativeTrack);
            }
        JNI_CATCH_RETURN_VOID("AlternativeTrackGroup.removeTrackNative")
    }
}
