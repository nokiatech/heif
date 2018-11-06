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

#include <AlternativeTrackGroup.h>
#include <Track.h>
#include <jni.h>

#include "Helpers.h"

#define CLASS_NAME AlternativeTrackGroup

extern "C"
{
    JNI_METHOD_ARG(jlong, createContextNative, jobject javaHEIF)
    {
        UNUSED(self);
        NATIVE_HEIF(nativeHeif, javaHEIF);
        HEIFPP::AlternativeTrackGroup* nativeObject = new HEIFPP::AlternativeTrackGroup(nativeHeif);
        return reinterpret_cast<jlong>(nativeObject);
    }

    JNI_METHOD(void, destroyContextNative)
    {
        NATIVE_SELF;
        setNativeHandle(env, self, 0);
        delete nativeSelf;
    }

    JNI_METHOD(jint, getTrackCountNative)
    {
        NATIVE_SELF;
        return static_cast<jint>(nativeSelf->getTrackCount());
    }

    JNI_METHOD_ARG(jobject, getTrackNative, jint index)
    {
        NATIVE_SELF;
        return getJavaTrack(env, getJavaHEIF(env, self), nativeSelf->getTrack(static_cast<uint32_t>(index)));
    }

    JNI_METHOD_ARG(void, addTrackNative, jobject track)
    {
        NATIVE_SELF;
        NATIVE_TRACK(nativeTrack, track);
        CHECK_ERROR(nativeSelf->addTrack(nativeTrack), "Adding to group failed");
    }

    JNI_METHOD_ARG(void, removeTrackNative, jobject track)
    {
        NATIVE_SELF;
        NATIVE_TRACK(nativeTrack, track);
        nativeSelf->removeTrack(nativeTrack);
    }
}
