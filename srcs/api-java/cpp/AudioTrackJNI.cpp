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
#include "AudioTrack.h"
#include "Helpers.h"


#define CLASS_NAME AudioTrack

extern "C"
{
    JNI_METHOD_ARG(jlong, createContextNative, jobject javaHEIF)
    {
        UNUSED(self);
        NATIVE_HEIF(nativeHeif, javaHEIF);
        HEIFPP::AudioTrack* nativeObject = new HEIFPP::AudioTrack(nativeHeif);
        return reinterpret_cast<jlong>(nativeObject);
    }

    JNI_METHOD_ARG(void, addSampleNative, jobject sample)
    {
        NATIVE_SELF;
        NATIVE_AUDIO_SAMPLE(nativeSample, sample);
        nativeSelf->addSample(nativeSample);
    }
}
