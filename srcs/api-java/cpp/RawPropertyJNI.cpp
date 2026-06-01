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
#include "RawProperty.h"
#include "heifcommondatatypes.h"
#include "jni_md.h"
#include <stdint.h>
#define CLASS_NAME RawProperty
extern "C"
{
    JNI_METHOD(jstring, getRawTypeNative)
    {
        JNI_TRY
        {
        auto* instance = (HEIFPP::RawProperty*) getNativeHandle(env, self);
        return env->NewStringUTF(instance->rawType().value);
            }
        JNI_CATCH_RETURN(nullptr, "RawProperty.getRawTypeNative")
    }

    JNI_METHOD_ARG(jlong, createContextNative, jobject javaHEIF)
    {
        JNI_TRY
        {
        UNUSED(self);
        NATIVE_HEIF(nativeHeif, javaHEIF);
        auto* nativeObject = new HEIFPP::RawProperty(nativeHeif);
        return reinterpret_cast<jlong>(nativeObject);
            }
        JNI_CATCH_RETURN(0, "RawProperty.createContextNative")
    }

    JNI_METHOD(void, destroyContextNative)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        setNativeHandle(env, self, 0);
        delete nativeSelf;
            }
        JNI_CATCH_RETURN_VOID("RawProperty.destroyContextNative")
    }

    JNI_METHOD(jobject, getDataNative)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        const uint8_t* data;
        uint64_t length;
        nativeSelf->getData(data, length);
        return env->NewDirectByteBuffer(const_cast<uint8_t*>(data), static_cast<jlong>(length));
            }
        JNI_CATCH_RETURN(nullptr, "RawProperty.getDataNative")
    }

    JNI_METHOD_ARG(void, setDataNative, jbyteArray data)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        jbyte* nativeData = env->GetByteArrayElements(data, nullptr);
        nativeSelf->setData((uint8_t*) (nativeData), static_cast<uint64_t>(env->GetArrayLength(data)));
        env->ReleaseByteArrayElements(data, nativeData, 0);
            }
        JNI_CATCH_RETURN_VOID("RawProperty.setDataNative")
    }

    JNI_METHOD_ARG(void, setRawTypeNative, jstring type)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        const char* nativeType = env->GetStringUTFChars(type, nullptr);

        HEIFPP::Result error = nativeSelf->setRawType(HEIF::FourCC(nativeType), false);
        env->ReleaseStringUTFChars(type, nativeType);
        CHECK_ERROR(error, "Setting raw type failed");
            }
        JNI_CATCH_RETURN_VOID("RawProperty.setRawTypeNative")
    }
}
