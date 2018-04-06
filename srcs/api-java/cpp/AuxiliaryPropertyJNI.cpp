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
#define JNI_METHOD(return_type, method_name) \
    JNIEXPORT return_type JNICALL Java_com_nokia_heif_AuxiliaryProperty_##method_name
extern "C"
{
    JNI_METHOD(jlong, createContextNative)(JNIEnv *env, jobject obj, jobject javaHEIF)
    {
        NATIVE_HEIF(nativeHeif, javaHEIF);
        HEIFPP::AuxProperty *nativeObject = new HEIFPP::AuxProperty(nativeHeif);
        nativeObject->setContext((void *) env->NewGlobalRef(obj));
        return (jlong) nativeObject;
    }


    JNI_METHOD(void, setTypeNative)(JNIEnv *env, jobject obj, jstring javaString)
    {
        NATIVE_AUXILIARY_PROPERTY(nativeHandle, obj);
        const char *nativeString = env->GetStringUTFChars(javaString, 0);
        nativeHandle->auxType(nativeString);
        env->ReleaseStringUTFChars(javaString, nativeString);
    }

    JNI_METHOD(jstring, getTypeNative)(JNIEnv *env, jobject obj)
    {
        NATIVE_AUXILIARY_PROPERTY(nativeHandle, obj);
        return env->NewStringUTF(nativeHandle->auxType().data());
    }

    JNI_METHOD(void, setSubTypeNative)(JNIEnv *env, jobject obj, jbyteArray subType)
    {
        NATIVE_AUXILIARY_PROPERTY(nativeHandle, obj);
        jbyte *nativeData = env->GetByteArrayElements(subType, 0);
        uint32_t dataSize = env->GetArrayLength(subType);
        std::vector<uint8_t> dataAsVector;
        dataAsVector.reserve(dataSize);
        dataAsVector.assign(nativeData, nativeData + dataSize);
        nativeHandle->subType(dataAsVector);
        env->ReleaseByteArrayElements(subType, nativeData, 0);
    }

    JNI_METHOD(jobject, getSubTypeNative)(JNIEnv *env, jobject obj)
    {
        NATIVE_AUXILIARY_PROPERTY(nativeHandle, obj);
        return env->NewDirectByteBuffer((void *) nativeHandle->subType().data(), nativeHandle->subType().size());
    }
}