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

#include "EqivGroup.h"
#include "Helpers.h"
#include "jni_md.h"
#include <stdint.h>

#define CLASS_NAME EquivalenceGroup

extern "C"
{
    JNI_METHOD_ARG(jlong, createContextNative, jobject javaHEIF)
    {
        JNI_TRY
        {
        UNUSED(self);
        NATIVE_HEIF(nativeHeif, javaHEIF);
        auto* nativeObject = new HEIFPP::EquivalenceGroup(nativeHeif);
        return reinterpret_cast<jlong>(nativeObject);
            }
        JNI_CATCH_RETURN(0, "EquivalenceGroup.createContextNative")
    }

    JNI_METHOD(void, destroyContextNative)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        setNativeHandle(env, self, 0);
        delete nativeSelf;
            }
        JNI_CATCH_RETURN_VOID("EquivalenceGroup.destroyContextNative")
    }

    JNI_METHOD_ARG(void, addSampleNative, jobject sample, jint offset, jint multiplier)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        NATIVE_SAMPLE(nativeSample, sample);

        nativeSelf->addSample(nativeSample, static_cast<int16_t>(offset), static_cast<uint16_t>(multiplier));
            }
        JNI_CATCH_RETURN_VOID("EquivalenceGroup.addSampleNative")
    }

    JNI_METHOD_ARG(jint, getOffsetNative, jobject sample)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        NATIVE_SAMPLE(nativeSample, sample);

        return static_cast<jint>(nativeSelf->getOffset(nativeSample));
            }
        JNI_CATCH_RETURN(0, "EquivalenceGroup.getOffsetNative")
    }

    JNI_METHOD_ARG(jint, getMultiplierNative, jobject sample)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        NATIVE_SAMPLE(nativeSample, sample);

        return static_cast<jint>(nativeSelf->getMultiplier(nativeSample));
            }
        JNI_CATCH_RETURN(0, "EquivalenceGroup.getMultiplierNative")
    }
}
