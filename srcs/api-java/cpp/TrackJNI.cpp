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
#include "Helpers.h"
#include "Track.h"

#define CLASS_NAME Track

extern "C"
{
    JNI_METHOD(void, destroyContextNative)
    {
        NATIVE_SELF;
        setNativeHandle(env, self, 0);
        delete nativeSelf;
    }

    JNI_METHOD(jint, getSampleCountNative)
    {
        NATIVE_SELF;
        return static_cast<jint>(nativeSelf->getSampleCount());
    }

    JNI_METHOD_ARG(jobject, getSampleNative, jint index)
    {
        NATIVE_SELF;
        return getJavaSample(env, getJavaHEIF(env, self), nativeSelf->getSample(static_cast<uint32_t>(index)));
    }

    JNI_METHOD_ARG(void, removeSampleNative, jobject sample)
    {
        NATIVE_SELF;
        NATIVE_SAMPLE(nativeSample, sample);
        nativeSelf->removeSample(nativeSample);
    }

    JNI_METHOD(jint, getTimescaleNative)
    {
        NATIVE_SELF;
        return static_cast<jint>(nativeSelf->getTimescale());
    }

    JNI_METHOD_ARG(void, setTimescaleNative, jint timescale)
    {
        NATIVE_SELF;
        nativeSelf->setTimescale(static_cast<uint32_t>(timescale));
    }

    JNI_METHOD(jint, getThumbnailCountNative)
    {
        NATIVE_SELF;
        return static_cast<jint>(nativeSelf->getThumbnailCount());
    }

    JNI_METHOD_ARG(jobject, getThumbnailNative, jint index)
    {
        NATIVE_SELF;
        return getJavaTrack(env, getJavaHEIF(env, self),
                            nativeSelf->getThumbnail(static_cast<uint32_t>(index)));
    }

    JNI_METHOD_ARG(void, addThumbnailNative, jobject thumbnail)
    {
        NATIVE_SELF;
        NATIVE_TRACK(nativeThumbnail, thumbnail);
        nativeSelf->addThumbnail(nativeThumbnail);
    }

    JNI_METHOD_ARG(void, removeThumbnailNative, jobject thumbnail)
    {
        NATIVE_SELF;
        NATIVE_TRACK(nativeThumbnail, thumbnail);
        nativeSelf->removeThumbnail(nativeThumbnail);
    }

    JNI_METHOD(jobject, getAlternativeTrackGroupNative)
    {
        NATIVE_SELF;
        return getJavaAlternativeTrackGroup(env, getJavaHEIF(env, self),
                                            nativeSelf->getAlternativeTrackGroup());
    }

    JNI_METHOD(jint, getGroupCountNative)
    {
        NATIVE_SELF;
        return static_cast<jint>(nativeSelf->getGroupCount());
    }

    JNI_METHOD_ARG(jobject, getGroupNative, jint index)
    {
        NATIVE_SELF;
        return getJavaEntityGroup(env, getJavaHEIF(env, self),
                                  nativeSelf->getGroup(static_cast<uint32_t>(index)));
    }

    JNI_METHOD(jboolean, hasInfiniteLoopPlaybackNative)
    {
        NATIVE_SELF;
        return static_cast<jboolean>(nativeSelf->hasInfiniteLoopPlayback());
    }

    JNI_METHOD(jdouble, getDurationNative)
    {
        NATIVE_SELF;
        return static_cast<jdouble>(nativeSelf->getDuration());
    }
}
