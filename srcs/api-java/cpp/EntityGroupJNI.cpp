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
#include "EntityGroup.h"
#include "Heif.h"
#include "Helpers.h"

#define CLASS_NAME EntityGroup

extern "C"
{
    JNI_METHOD_ARG(jlong, createContextNative, jobject javaHEIF, jstring javaFourCC)
    {
        UNUSED(self);
        NATIVE_HEIF(nativeHeif, javaHEIF);
        const char* nativeString          = env->GetStringUTFChars(javaFourCC, 0);
        HEIFPP::EntityGroup* nativeObject = new HEIFPP::EntityGroup(nativeHeif, HEIF::FourCC(nativeString));
        env->ReleaseStringUTFChars(javaFourCC, nativeString);
        return (jlong) nativeObject;
    }

    JNI_METHOD(void, destroyContextNative)
    {
        NATIVE_SELF;
        setNativeHandle(env, self, 0);
        delete nativeSelf;
    }

    JNI_METHOD(jstring, getTypeNative)
    {
        NATIVE_SELF;
        return env->NewStringUTF(nativeSelf->getType().value);
    }

    JNI_METHOD(jint, getEntityCountNative)
    {
        NATIVE_SELF;
        return static_cast<jint>(nativeSelf->getEntityCount());
    }

    JNI_METHOD_ARG(jboolean, isItemNative, jint index)
    {
        NATIVE_SELF;
        return static_cast<jboolean>(nativeSelf->isItem(static_cast<uint32_t>(index)));
    }

    JNI_METHOD_ARG(jobject, getItemNative, jint index)
    {
        NATIVE_SELF;
        return getJavaItem(env, getJavaHEIF(env, self), nativeSelf->getItem(static_cast<uint32_t>(index)));
    }

    JNI_METHOD_ARG(void, addItemNative, jobject item)
    {
        NATIVE_SELF;
        NATIVE_ITEM(nativeItem, item);
        nativeSelf->addItem(nativeItem);
    }

    JNI_METHOD_ARG(void, removeItemNative, jobject item)
    {
        NATIVE_SELF;
        NATIVE_ITEM(nativeItem, item);
        nativeSelf->removeItem(nativeItem);
    }

    JNI_METHOD_ARG(jboolean, isTrackNative, jint index)
    {
        NATIVE_SELF;
        return static_cast<jboolean>(nativeSelf->isTrack(static_cast<uint32_t>(index)));
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
        nativeSelf->addTrack(nativeTrack);
    }

    JNI_METHOD_ARG(void, removeTrackNative, jobject track)
    {
        NATIVE_SELF;
        NATIVE_TRACK(nativeTrack, track);
        nativeSelf->removeTrack(nativeTrack);
    }

    JNI_METHOD_ARG(jboolean, isSampleNative, jint index)
    {
        NATIVE_SELF;
        return static_cast<jboolean>(nativeSelf->isSample(static_cast<uint32_t>(index)));
    }

    JNI_METHOD_ARG(jobject, getSampleNative, jint index)
    {
        NATIVE_SELF;
        return getJavaSample(env, getJavaHEIF(env, self), nativeSelf->getSample(static_cast<uint32_t>(index)));
    }

    JNI_METHOD_ARG(void, addSampleNative, jobject sample)
    {
        NATIVE_SELF;
        NATIVE_SAMPLE(nativeSample, sample);
        nativeSelf->addSample(nativeSample);
    }

    JNI_METHOD_ARG(void, removeSampleNative, jobject sample)
    {
        NATIVE_SELF;
        NATIVE_SAMPLE(nativeSample, sample);
        nativeSelf->removeSample(nativeSample);
    }
}
