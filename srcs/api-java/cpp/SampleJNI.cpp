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

#include "Helpers.h"
#include "Sample.h"
#include "heifcommondatatypes.h"
#include "jni_md.h"
#include <stdint.h>

#define CLASS_NAME Sample

extern "C"
{
    JNI_METHOD(void, destroyContextNative)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        setNativeHandle(env, self, 0);
        delete nativeSelf;
            }
        JNI_CATCH_RETURN_VOID("Sample.destroyContextNative")
    }

    JNI_METHOD(jobject, getDecoderConfigNative)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        return getDecoderConfig(env, getJavaHEIF(env, self), nativeSelf->getDecoderConfiguration());
            }
        JNI_CATCH_RETURN(nullptr, "Sample.getDecoderConfigNative")
    }

    JNI_METHOD_ARG(void, setDecoderConfigNative, jobject config)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        NATIVE_DECODER_CONFIG(nativeConfig, config);
        nativeSelf->setDecoderConfiguration(nativeConfig);
            }
        JNI_CATCH_RETURN_VOID("Sample.setDecoderConfigNative")
    }

    JNI_METHOD(jint, getSampleDataSizeNative)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        return static_cast<jint>(nativeSelf->getSampleDataSize());
            }
        JNI_CATCH_RETURN(0, "Sample.getSampleDataSizeNative")
    }

    JNI_METHOD(jobject, getSampleDataNative)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        return env->NewDirectByteBuffer(const_cast<uint8_t*>(nativeSelf->getSampleData()),
                                        static_cast<jlong>(nativeSelf->getSampleDataSize()));
            }
        JNI_CATCH_RETURN(nullptr, "Sample.getSampleDataNative")
    }

    JNI_METHOD_ARG(void, setSampleDataNative, jbyteArray data)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        jbyte* nativeData = env->GetByteArrayElements(data, nullptr);
        nativeSelf->setItemData((uint8_t*) nativeData, static_cast<uint64_t>(env->GetArrayLength(data)));
        env->ReleaseByteArrayElements(data, nativeData, 0);
            }
        JNI_CATCH_RETURN_VOID("Sample.setSampleDataNative")
    }

    JNI_METHOD(jlong, getDurationNative)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        return static_cast<jlong>(nativeSelf->getDuration());
            }
        JNI_CATCH_RETURN(0, "Sample.getDurationNative")
    }

    JNI_METHOD_ARG(void, setDurationNative, jlong duration)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        nativeSelf->setDuration(static_cast<uint64_t>(duration));
            }
        JNI_CATCH_RETURN_VOID("Sample.setDurationNative")
    }

    JNI_METHOD(jint, getTimestampCountNative)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        return static_cast<jint>(nativeSelf->getTimeStampCount());
            }
        JNI_CATCH_RETURN(0, "Sample.getTimestampCountNative")
    }

    JNI_METHOD_ARG(jlong, getTimestampNative, jint index)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        return static_cast<jlong>(nativeSelf->getTimeStamp(static_cast<uint32_t>(index)));
            }
        JNI_CATCH_RETURN(0, "Sample.getTimestampNative")
    }

    JNI_METHOD_ARG(void, setCompositionOffsetNative, jlong offset)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        nativeSelf->setCompositionOffset(offset);
            }
        JNI_CATCH_RETURN_VOID("Sample.setCompositionOffsetNative")
    }

    JNI_METHOD(jlong, getCompositionOffsetNative)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        return nativeSelf->getCompositionOffset();
            }
        JNI_CATCH_RETURN(0, "Sample.getCompositionOffsetNative")
    }

    JNI_METHOD(jint, getDecodeDependencyCountNative)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        return static_cast<jint>(nativeSelf->getDecodeDependencyCount());
            }
        JNI_CATCH_RETURN(0, "Sample.getDecodeDependencyCountNative")
    }

    JNI_METHOD_ARG(jobject, getDecodeDependencyNative, jint index)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        return getJavaSample(env, getJavaHEIF(env, self),
                             nativeSelf->getDecodeDependency(static_cast<uint32_t>(index)));
            }
        JNI_CATCH_RETURN(nullptr, "Sample.getDecodeDependencyNative")
    }

    JNI_METHOD_ARG(void, addDecodeDependencyNative, jobject dependency)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        NATIVE_SAMPLE(nativeDependency, dependency);
        nativeSelf->addDecodeDependency(nativeDependency);
            }
        JNI_CATCH_RETURN_VOID("Sample.addDecodeDependencyNative")
    }

    JNI_METHOD_ARG(void, removeDecodeDependencyNative, jobject dependency)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        NATIVE_SAMPLE(nativeDependency, dependency);
        nativeSelf->removeDecodeDependency(nativeDependency);
            }
        JNI_CATCH_RETURN_VOID("Sample.removeDecodeDependencyNative")
    }

    JNI_METHOD(jint, getMetadataCountNative)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        return static_cast<jint>(nativeSelf->getMetadataCount());
            }
        JNI_CATCH_RETURN(0, "Sample.getMetadataCountNative")
    }

    JNI_METHOD_ARG(jobject, getMetadataNative, jint index)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        return getJavaItem(env, getJavaHEIF(env, self), nativeSelf->getMetadata(static_cast<uint32_t>(index)));
            }
        JNI_CATCH_RETURN(nullptr, "Sample.getMetadataNative")
    }

    JNI_METHOD_ARG(void, addMetadataNative, jobject item)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        NATIVE_META_ITEM(nativeItem, item);
        nativeSelf->addMetadata(nativeItem);
            }
        JNI_CATCH_RETURN_VOID("Sample.addMetadataNative")
    }

    JNI_METHOD_ARG(void, removeMetadataNative, jobject item)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        NATIVE_META_ITEM(nativeItem, item);
        nativeSelf->removeMetadata(nativeItem);
            }
        JNI_CATCH_RETURN_VOID("Sample.removeMetadataNative")
    }

    JNI_METHOD(jint, getGroupCountNative)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        return static_cast<jint>(nativeSelf->getGroupCount());
            }
        JNI_CATCH_RETURN(0, "Sample.getGroupCountNative")
    }

    JNI_METHOD_ARG(jobject, getGroupNative, jint index)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        return getJavaEntityGroup(env, getJavaHEIF(env, self), nativeSelf->getGroup(static_cast<uint32_t>(index)));
            }
        JNI_CATCH_RETURN(nullptr, "Sample.getGroupNative")
    }

    JNI_METHOD(jobject, getTrackNative)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        return getJavaTrack(env, getJavaHEIF(env, self), nativeSelf->getTrack());
            }
        JNI_CATCH_RETURN(nullptr, "Sample.getTrackNative")
    }

    JNI_METHOD(jint, getSampleTypeNative)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        return static_cast<jint>(nativeSelf->getSampleType());
            }
        JNI_CATCH_RETURN(0, "Sample.getSampleTypeNative")
    }

    JNI_METHOD(jstring, getTypeNative)
    {
        JNI_TRY
        {
        auto* instance = (HEIFPP::Sample*) getNativeHandle(env, self);
        return env->NewStringUTF(instance->getType().value);
            }
        JNI_CATCH_RETURN(nullptr, "Sample.getTypeNative")
    }
}
