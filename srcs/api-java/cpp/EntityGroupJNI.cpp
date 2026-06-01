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

#include "EntityGroup.h"
#include "Helpers.h"
#include "heifcommondatatypes.h"
#include "jni_md.h"
#include <stdint.h>

#define CLASS_NAME EntityGroup

extern "C"
{
    JNI_METHOD_ARG(jlong, createContextNative, jobject javaHEIF, jstring javaFourCC)
    {
        JNI_TRY
        {
        UNUSED(self);
        NATIVE_HEIF(nativeHeif, javaHEIF);
        const char* nativeString = env->GetStringUTFChars(javaFourCC, nullptr);
        auto* nativeObject       = new HEIFPP::EntityGroup(nativeHeif, HEIF::FourCC(nativeString));
        env->ReleaseStringUTFChars(javaFourCC, nativeString);
        return (jlong) nativeObject;
            }
        JNI_CATCH_RETURN(0, "EntityGroup.createContextNative")
    }

    JNI_METHOD(void, destroyContextNative)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        setNativeHandle(env, self, 0);
        delete nativeSelf;
            }
        JNI_CATCH_RETURN_VOID("EntityGroup.destroyContextNative")
    }

    JNI_METHOD(jstring, getTypeNative)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        return env->NewStringUTF(nativeSelf->getType().value);
            }
        JNI_CATCH_RETURN(nullptr, "EntityGroup.getTypeNative")
    }

    JNI_METHOD(jint, getEntityCountNative)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        return static_cast<jint>(nativeSelf->getEntityCount());
            }
        JNI_CATCH_RETURN(0, "EntityGroup.getEntityCountNative")
    }

    JNI_METHOD_ARG(jboolean, isItemNative, jint index)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        return static_cast<jboolean>(nativeSelf->isItem(static_cast<uint32_t>(index)));
            }
        JNI_CATCH_RETURN(JNI_FALSE, "EntityGroup.isItemNative")
    }

    JNI_METHOD_ARG(jobject, getItemNative, jint index)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        return getJavaItem(env, getJavaHEIF(env, self), nativeSelf->getItem(static_cast<uint32_t>(index)));
            }
        JNI_CATCH_RETURN(nullptr, "EntityGroup.getItemNative")
    }

    JNI_METHOD_ARG(void, addItemNative, jobject item)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        NATIVE_ITEM(nativeItem, item);
        nativeSelf->addItem(nativeItem);
            }
        JNI_CATCH_RETURN_VOID("EntityGroup.addItemNative")
    }

    JNI_METHOD_ARG(void, removeItemNative, jobject item)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        NATIVE_ITEM(nativeItem, item);
        nativeSelf->removeItem(nativeItem);
            }
        JNI_CATCH_RETURN_VOID("EntityGroup.removeItemNative")
    }

    JNI_METHOD_ARG(jboolean, isTrackNative, jint index)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        return static_cast<jboolean>(nativeSelf->isTrack(static_cast<uint32_t>(index)));
            }
        JNI_CATCH_RETURN(JNI_FALSE, "EntityGroup.isTrackNative")
    }

    JNI_METHOD_ARG(jobject, getTrackNative, jint index)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        return getJavaTrack(env, getJavaHEIF(env, self), nativeSelf->getTrack(static_cast<uint32_t>(index)));
            }
        JNI_CATCH_RETURN(nullptr, "EntityGroup.getTrackNative")
    }

    JNI_METHOD_ARG(void, addTrackNative, jobject track)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        NATIVE_TRACK(nativeTrack, track);
        nativeSelf->addTrack(nativeTrack);
            }
        JNI_CATCH_RETURN_VOID("EntityGroup.addTrackNative")
    }

    JNI_METHOD_ARG(void, removeTrackNative, jobject track)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        NATIVE_TRACK(nativeTrack, track);
        nativeSelf->removeTrack(nativeTrack);
            }
        JNI_CATCH_RETURN_VOID("EntityGroup.removeTrackNative")
    }

    JNI_METHOD_ARG(jboolean, isSampleNative, jint index)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        return static_cast<jboolean>(nativeSelf->isSample(static_cast<uint32_t>(index)));
            }
        JNI_CATCH_RETURN(JNI_FALSE, "EntityGroup.isSampleNative")
    }

    JNI_METHOD_ARG(jobject, getSampleNative, jint index)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        return getJavaSample(env, getJavaHEIF(env, self), nativeSelf->getSample(static_cast<uint32_t>(index)));
            }
        JNI_CATCH_RETURN(nullptr, "EntityGroup.getSampleNative")
    }

    JNI_METHOD_ARG(void, addSampleNative, jobject sample)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        NATIVE_SAMPLE(nativeSample, sample);
        nativeSelf->addSample(nativeSample);
            }
        JNI_CATCH_RETURN_VOID("EntityGroup.addSampleNative")
    }

    JNI_METHOD_ARG(void, removeSampleNative, jobject sample)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        NATIVE_SAMPLE(nativeSample, sample);
        nativeSelf->removeSample(nativeSample);
            }
        JNI_CATCH_RETURN_VOID("EntityGroup.removeSampleNative")
    }
}
