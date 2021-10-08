/*
 * This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2021 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
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
        NATIVE_HEIF(nativeHeif, javaHEIF);
        HEIFPP::EqivGroup* nativeObject = new HEIFPP::EqivGroup(nativeHeif);
        nativeObject->setContext(static_cast<void*>(env->NewGlobalRef(self)));
        return reinterpret_cast<jlong>(nativeObject);
    }

    JNI_METHOD(void, destroyContextNative)
    {
        NATIVE_EQUIVALENCE_GROUP(nativeHandle, self);
        jobject javaHandle = GET_JAVA_OBJECT(nativeHandle);
        env->DeleteGlobalRef(javaHandle);
        setNativeHandle(env, self, 0);
        delete nativeHandle;
    }

    JNI_METHOD_ARG(void, addSampleNative, jobject sample, jint offset, jint multiplier)
    {
        NATIVE_EQUIVALENCE_GROUP(nativeHandle, self);
        NATIVE_SAMPLE(nativeSample, sample);

        nativeHandle->addSample(nativeSample, static_cast<int16_t>(offset), static_cast<uint16_t>(multiplier));
    }

    JNI_METHOD_ARG(jint, getOffsetNative, jobject sample)
    {
        NATIVE_EQUIVALENCE_GROUP(nativeHandle, self);
        NATIVE_SAMPLE(nativeSample, sample);

        return static_cast<jint>(nativeHandle->getOffset(nativeSample));
    }

    JNI_METHOD_ARG(jint, getMultiplierNative, jobject sample)
    {
        NATIVE_EQUIVALENCE_GROUP(nativeHandle, self);
        NATIVE_SAMPLE(nativeSample, sample);

        return static_cast<jint>(nativeHandle->getMultiplier(nativeSample));
    }
}
