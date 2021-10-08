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
#include "DescriptiveProperty.h"
#include "Helpers.h"

#define CLASS_NAME AuxiliaryProperty

extern "C"
{
    JNI_METHOD_ARG(jlong, createContextNative, jobject javaHEIF)
    {
        NATIVE_HEIF(nativeHeif, javaHEIF);
        HEIFPP::AuxProperty *nativeObject = new HEIFPP::AuxProperty(nativeHeif);
        nativeObject->setContext(static_cast<void*>(env->NewGlobalRef(self)));
        return reinterpret_cast<jlong>(nativeObject);
    }


    JNI_METHOD_ARG(void, setTypeNative, jstring javaString)
    {
        NATIVE_AUXILIARY_PROPERTY(nativeHandle, self);
        const char *nativeString = env->GetStringUTFChars(javaString, 0);
        nativeHandle->auxType(nativeString);
        env->ReleaseStringUTFChars(javaString, nativeString);
    }

    JNI_METHOD(jstring, getTypeNative)
    {
        NATIVE_AUXILIARY_PROPERTY(nativeHandle, self);
        return env->NewStringUTF(nativeHandle->auxType().data());
    }

    JNI_METHOD_ARG(void, setSubTypeNative, jbyteArray subType)
    {
        NATIVE_AUXILIARY_PROPERTY(nativeHandle, self);
        jbyte *nativeData = env->GetByteArrayElements(subType, 0);
        uint32_t dataSize = static_cast<uint32_t>(env->GetArrayLength(subType));
        std::vector<uint8_t> dataAsVector;
        dataAsVector.reserve(dataSize);
        dataAsVector.assign(nativeData, nativeData + dataSize);
        nativeHandle->subType(dataAsVector);
        env->ReleaseByteArrayElements(subType, nativeData, 0);
    }

    JNI_METHOD(jobject, getSubTypeNative)
    {
        NATIVE_AUXILIARY_PROPERTY(nativeHandle, self);
        return env->NewDirectByteBuffer(const_cast<uint8_t*>(nativeHandle->subType().data()),
                                        static_cast<jlong>(nativeHandle->subType().size()));
    }
}