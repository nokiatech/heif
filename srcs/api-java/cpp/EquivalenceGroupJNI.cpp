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
#include "EqivGroup.h"
#include "Helpers.h"

#define CLASS_NAME EquivalenceGroup

extern "C"
{
    JNI_METHOD_ARG(jlong, createContextNative, jobject javaHEIF)
    {
        UNUSED(self);
        NATIVE_HEIF(nativeHeif, javaHEIF);
        HEIFPP::EquivalenceGroup* nativeObject = new HEIFPP::EquivalenceGroup(nativeHeif);
        return reinterpret_cast<jlong>(nativeObject);
    }

    JNI_METHOD(void, destroyContextNative)
    {
        NATIVE_SELF;
        setNativeHandle(env, self, 0);
        delete nativeSelf;
    }

    JNI_METHOD_ARG(void, addSampleNative, jobject sample, jint offset, jint multiplier)
    {
        NATIVE_SELF;
        NATIVE_SAMPLE(nativeSample, sample);

        nativeSelf->addSample(nativeSample, static_cast<int16_t>(offset), static_cast<uint16_t>(multiplier));
    }

    JNI_METHOD_ARG(jint, getOffsetNative, jobject sample)
    {
        NATIVE_SELF;
        NATIVE_SAMPLE(nativeSample, sample);

        return static_cast<jint>(nativeSelf->getOffset(nativeSample));
    }

    JNI_METHOD_ARG(jint, getMultiplierNative, jobject sample)
    {
        NATIVE_SELF;
        NATIVE_SAMPLE(nativeSample, sample);

        return static_cast<jint>(nativeSelf->getMultiplier(nativeSample));
    }
}
