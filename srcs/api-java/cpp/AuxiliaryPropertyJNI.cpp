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

#include "DescriptiveProperty.h"
#include "Helpers.h"
#include "jni_md.h"
#include <stdint.h>
#include <string>
#include <vector>

#define CLASS_NAME AuxiliaryProperty

extern "C"
{
    JNI_METHOD_ARG(jlong, createContextNative, jobject javaHEIF)
    {
        JNI_TRY
        {
        UNUSED(self);
        NATIVE_HEIF(nativeHeif, javaHEIF);
        auto* nativeObject = new HEIFPP::AuxiliaryProperty(nativeHeif);
        return reinterpret_cast<jlong>(nativeObject);
            }
        JNI_CATCH_RETURN(0, "AuxiliaryProperty.createContextNative")
    }


    JNI_METHOD_ARG(void, setTypeNative, jstring javaString)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        const char* nativeString = env->GetStringUTFChars(javaString, nullptr);
        nativeSelf->auxType(nativeString);
        env->ReleaseStringUTFChars(javaString, nativeString);
            }
        JNI_CATCH_RETURN_VOID("AuxiliaryProperty.setTypeNative")
    }

    JNI_METHOD(jstring, getTypeNative)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        return env->NewStringUTF(nativeSelf->auxType().data());
            }
        JNI_CATCH_RETURN(nullptr, "AuxiliaryProperty.getTypeNative")
    }

    JNI_METHOD_ARG(void, setSubTypeNative, jbyteArray subType)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        jbyte* nativeData = env->GetByteArrayElements(subType, nullptr);
        auto dataSize     = static_cast<uint32_t>(env->GetArrayLength(subType));
        std::vector<uint8_t> dataAsVector;
        dataAsVector.reserve(dataSize);
        dataAsVector.assign(nativeData, nativeData + dataSize);
        nativeSelf->subType(dataAsVector);
        env->ReleaseByteArrayElements(subType, nativeData, 0);
            }
        JNI_CATCH_RETURN_VOID("AuxiliaryProperty.setSubTypeNative")
    }

    JNI_METHOD(jobject, getSubTypeNative)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        return env->NewDirectByteBuffer(const_cast<uint8_t*>(nativeSelf->subType().data()),
                                        static_cast<jlong>(nativeSelf->subType().size()));
            }
        JNI_CATCH_RETURN(nullptr, "AuxiliaryProperty.getSubTypeNative")
    }
}
