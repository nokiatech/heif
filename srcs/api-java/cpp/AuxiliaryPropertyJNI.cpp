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
#include "DescriptiveProperty.h"
#include "Helpers.h"

#define CLASS_NAME AuxiliaryProperty

extern "C"
{
    JNI_METHOD_ARG(jlong, createContextNative, jobject javaHEIF)
    {
        UNUSED(self);
        NATIVE_HEIF(nativeHeif, javaHEIF);
        HEIFPP::AuxiliaryProperty *nativeObject = new HEIFPP::AuxiliaryProperty(nativeHeif);
        return reinterpret_cast<jlong>(nativeObject);
    }


    JNI_METHOD_ARG(void, setTypeNative, jstring javaString)
    {
        NATIVE_SELF;
        const char *nativeString = env->GetStringUTFChars(javaString, 0);
        nativeSelf->auxType(nativeString);
        env->ReleaseStringUTFChars(javaString, nativeString);
    }

    JNI_METHOD(jstring, getTypeNative)
    {
        NATIVE_SELF;
        return env->NewStringUTF(nativeSelf->auxType().data());
    }

    JNI_METHOD_ARG(void, setSubTypeNative, jbyteArray subType)
    {
        NATIVE_SELF;
        jbyte *nativeData = env->GetByteArrayElements(subType, 0);
        uint32_t dataSize = static_cast<uint32_t>(env->GetArrayLength(subType));
        std::vector<uint8_t> dataAsVector;
        dataAsVector.reserve(dataSize);
        dataAsVector.assign(nativeData, nativeData + dataSize);
        nativeSelf->subType(dataAsVector);
        env->ReleaseByteArrayElements(subType, nativeData, 0);
    }

    JNI_METHOD(jobject, getSubTypeNative)
    {
        NATIVE_SELF;
        return env->NewDirectByteBuffer(const_cast<uint8_t *>(nativeSelf->subType().data()),
                                        static_cast<jlong>(nativeSelf->subType().size()));
    }
}
