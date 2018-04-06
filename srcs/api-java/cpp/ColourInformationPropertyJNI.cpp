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
    JNIEXPORT return_type JNICALL Java_com_nokia_heif_ColourInformationProperty_##method_name
extern "C"
{
    JNI_METHOD(jlong, createContextNative)(JNIEnv *env, jobject obj, jobject javaHEIF)
    {
        NATIVE_HEIF(nativeHeif, javaHEIF);
        HEIFPP::ColourInformationProperty *nativeObject = new HEIFPP::ColourInformationProperty(nativeHeif);
        nativeObject->setContext((void *) env->NewGlobalRef(obj));
        return (jlong) nativeObject;
    }

    JNI_METHOD(jstring, getColourTypeNative)(JNIEnv *env, jobject obj)
    {
        NATIVE_COLOUR_INFORMATION_PROPERTY(nativeHandle, obj);
        return env->NewStringUTF(nativeHandle->mColourInformation.colourType.value);
    }

    JNI_METHOD(void, setColourTypeNative)(JNIEnv *env, jobject obj, jstring javaString)
    {
        NATIVE_COLOUR_INFORMATION_PROPERTY(nativeHandle, obj);
        const char *nativeString                    = env->GetStringUTFChars(javaString, 0);
        nativeHandle->mColourInformation.colourType = HEIF::FourCC(nativeString);
        env->ReleaseStringUTFChars(javaString, nativeString);
    }

    JNI_METHOD(jint, getColourPrimariesNative)(JNIEnv *env, jobject obj)
    {
        NATIVE_COLOUR_INFORMATION_PROPERTY(nativeHandle, obj);
        return nativeHandle->mColourInformation.colourPrimaries;
    }

    JNI_METHOD(void, setColourPrimariesNative)(JNIEnv *env, jobject obj, jint primaries)
    {
        NATIVE_COLOUR_INFORMATION_PROPERTY(nativeHandle, obj);
        nativeHandle->mColourInformation.colourPrimaries = primaries;
    }

    JNI_METHOD(jint, getTransferCharacteristicsNative)(JNIEnv *env, jobject obj)
    {
        NATIVE_COLOUR_INFORMATION_PROPERTY(nativeHandle, obj);
        return nativeHandle->mColourInformation.transferCharacteristics;
    }

    JNI_METHOD(void, setTransferCharacteristicsNative)(JNIEnv *env, jobject obj, jint characteristics)
    {
        NATIVE_COLOUR_INFORMATION_PROPERTY(nativeHandle, obj);
        nativeHandle->mColourInformation.transferCharacteristics = characteristics;
    }

    JNI_METHOD(jint, getMatrixCoefficientsNative)(JNIEnv *env, jobject obj)
    {
        NATIVE_COLOUR_INFORMATION_PROPERTY(nativeHandle, obj);
        return nativeHandle->mColourInformation.matrixCoefficients;
    }

    JNI_METHOD(void, setMatrixCoefficientsNative)(JNIEnv *env, jobject obj, jint matrixCoefficients)
    {
        NATIVE_COLOUR_INFORMATION_PROPERTY(nativeHandle, obj);
        nativeHandle->mColourInformation.matrixCoefficients = matrixCoefficients;
    }

    JNI_METHOD(jboolean, getFullRangeFlagNative)(JNIEnv *env, jobject obj)
    {
        NATIVE_COLOUR_INFORMATION_PROPERTY(nativeHandle, obj);
        return nativeHandle->mColourInformation.fullRangeFlag;
    }

    JNI_METHOD(void, setFullRangeFlagNative)(JNIEnv *env, jobject obj, jboolean flag)
    {
        NATIVE_COLOUR_INFORMATION_PROPERTY(nativeHandle, obj);
        nativeHandle->mColourInformation.fullRangeFlag = flag;
    }

    JNI_METHOD(jobject, getICCProfileNative)(JNIEnv *env, jobject obj)
    {
        NATIVE_COLOUR_INFORMATION_PROPERTY(nativeHandle, obj);
        return env->NewDirectByteBuffer(nativeHandle->mColourInformation.iccProfile.elements,
                                        nativeHandle->mColourInformation.iccProfile.size);
    }

    JNI_METHOD(void, setICCProfileNative)(JNIEnv *env, jobject obj, jbyteArray data)
    {
        NATIVE_COLOUR_INFORMATION_PROPERTY(nativeHandle, obj);
        jbyte *nativeData                           = env->GetByteArrayElements(data, 0);
        uint32_t dataSize                           = env->GetArrayLength(data);
        nativeHandle->mColourInformation.iccProfile = HEIF::Array<uint8_t>(dataSize);
        memcpy(nativeHandle->mColourInformation.iccProfile.elements, nativeData, dataSize);
        env->ReleaseByteArrayElements(data, nativeData, 0);
    }
}
