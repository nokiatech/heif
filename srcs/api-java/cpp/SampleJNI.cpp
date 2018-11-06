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
#include "Sample.h"

#define CLASS_NAME Sample

extern "C"
{
    JNI_METHOD(void, destroyContextNative)
    {
        NATIVE_SELF;
        setNativeHandle(env, self, 0);
        delete nativeSelf;
    }

    JNI_METHOD(jobject, getDecoderConfigNative)
    {
        NATIVE_SELF;
        return getDecoderConfig(env, getJavaHEIF(env, self), nativeSelf->getDecoderConfiguration());
    }

    JNI_METHOD_ARG(void, setDecoderConfigNative, jobject config)
    {
        NATIVE_SELF;
        NATIVE_DECODER_CONFIG(nativeConfig, config);
        nativeSelf->setDecoderConfiguration(nativeConfig);
    }

    JNI_METHOD(jint, getSampleDataSizeNative)
    {
        NATIVE_SELF;
        return static_cast<jint>(nativeSelf->getSampleDataSize());
    }

    JNI_METHOD(jobject, getSampleDataNative)
    {
        NATIVE_SELF;
        return env->NewDirectByteBuffer(const_cast<uint8_t*>(nativeSelf->getSampleData()),
                                        static_cast<jlong>(nativeSelf->getSampleDataSize()));
    }

    JNI_METHOD_ARG(void, setSampleDataNative, jbyteArray data)
    {
        NATIVE_SELF;
        jbyte *nativeData = env->GetByteArrayElements(data, 0);
        nativeSelf->setItemData((uint8_t *) nativeData,
                                static_cast<uint64_t>(env->GetArrayLength(data)));
        env->ReleaseByteArrayElements(data, nativeData, 0);
    }

    JNI_METHOD(jlong, getDurationNative)
    {
        NATIVE_SELF;
        return static_cast<jlong>(nativeSelf->getDuration());
    }

    JNI_METHOD_ARG(void, setDurationNative, jlong duration)
    {
        NATIVE_SELF;
        nativeSelf->setDuration(static_cast<uint64_t>(duration));
    }

    JNI_METHOD(jint, getTimestampCountNative)
    {
        NATIVE_SELF;
        return static_cast<jint>(nativeSelf->getTimeStampCount());
    }

    JNI_METHOD_ARG(jlong, getTimestampNative, jint index)
    {
        NATIVE_SELF;
        return static_cast<jlong>(nativeSelf->getTimeStamp(static_cast<uint32_t>(index)));
    }

    JNI_METHOD_ARG(void, setCompositionOffsetNative, jlong offset)
    {
        NATIVE_SELF;
        nativeSelf->setCompositionOffset(offset);
    }

    JNI_METHOD(jlong, getCompositionOffsetNative)
    {
        NATIVE_SELF;
        return static_cast<jlong>(nativeSelf->getCompositionOffset());
    }

    JNI_METHOD(jint, getDecodeDependencyCountNative)
    {
        NATIVE_SELF;
        return static_cast<jint>(nativeSelf->getDecodeDependencyCount());
    }

    JNI_METHOD_ARG(jobject, getDecodeDependencyNative, jint index)
    {
        NATIVE_SELF;
        return getJavaSample(env, getJavaHEIF(env, self),
                             nativeSelf->getDecodeDependency(static_cast<uint32_t>(index)));
    }

    JNI_METHOD_ARG(void, addDecodeDependencyNative, jobject dependency)
    {
        NATIVE_SELF;
        NATIVE_SAMPLE(nativeDependency, dependency);
        nativeSelf->addDecodeDependency(nativeDependency);
    }

    JNI_METHOD_ARG(void, removeDecodeDependencyNative, jobject dependency)
    {
        NATIVE_SELF;
        NATIVE_SAMPLE(nativeDependency, dependency);
        nativeSelf->removeDecodeDependency(nativeDependency);
    }

    JNI_METHOD(jint, getMetadataCountNative)
    {
        NATIVE_SELF;
        return static_cast<jint>(nativeSelf->getMetadataCount());
    }

    JNI_METHOD_ARG(jobject, getMetadataNative, jint index)
    {
        NATIVE_SELF;
        return getJavaItem(env, getJavaHEIF(env, self),
                           nativeSelf->getMetadata(static_cast<uint32_t>(index)));
    }

    JNI_METHOD_ARG(void, addMetadataNative, jobject item)
    {
        NATIVE_SELF;
        NATIVE_META_ITEM(nativeItem, item);
        nativeSelf->addMetadata(nativeItem);
    }

    JNI_METHOD_ARG(void, removeMetadataNative, jobject item)
    {
        NATIVE_SELF;
        NATIVE_META_ITEM(nativeItem, item);
        nativeSelf->removeMetadata(nativeItem);
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

    JNI_METHOD(jobject, getTrackNative)
    {
        NATIVE_SELF;
        return getJavaTrack(env, getJavaHEIF(env,self), nativeSelf->getTrack());
    }

    JNI_METHOD(jint, getSampleTypeNative)
    {
        NATIVE_SELF;
        return static_cast<jint>(nativeSelf->getSampleType());
    }
}