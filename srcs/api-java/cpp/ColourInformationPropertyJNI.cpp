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
#include <cstring>
#include "DescriptiveProperty.h"
#include "Helpers.h"

#define CLASS_NAME ColourInformationProperty
extern "C"
{
    JNI_METHOD_ARG(jlong, createContextNative, jobject javaHEIF)
    {
        UNUSED(self);
        NATIVE_HEIF(nativeHeif, javaHEIF);
        HEIFPP::ColourInformationProperty *nativeObject = new HEIFPP::ColourInformationProperty(nativeHeif);
        return reinterpret_cast<jlong>(nativeObject);
    }

    JNI_METHOD(jstring, getColourTypeNative)
    {
        NATIVE_SELF;
        return env->NewStringUTF(nativeSelf->mColourInformation.colourType.value);
    }

    JNI_METHOD_ARG(void, setColourTypeNative, jstring javaString)
    {
        NATIVE_SELF;
        const char *nativeString                  = env->GetStringUTFChars(javaString, 0);
        nativeSelf->mColourInformation.colourType = HEIF::FourCC(nativeString);
        env->ReleaseStringUTFChars(javaString, nativeString);
    }

    JNI_METHOD(jint, getColourPrimariesNative)
    {
        NATIVE_SELF;
        return static_cast<jint>(nativeSelf->mColourInformation.colourPrimaries);
    }

    JNI_METHOD_ARG(void, setColourPrimariesNative, jint primaries)
    {
        NATIVE_SELF;
        nativeSelf->mColourInformation.colourPrimaries = static_cast<uint16_t>(primaries);
    }

    JNI_METHOD(jint, getTransferCharacteristicsNative)
    {
        NATIVE_SELF;
        return static_cast<jint>(nativeSelf->mColourInformation.transferCharacteristics);
    }

    JNI_METHOD_ARG(void, setTransferCharacteristicsNative, jint characteristics)
    {
        NATIVE_SELF;
        nativeSelf->mColourInformation.transferCharacteristics = static_cast<uint16_t>(characteristics);
    }

    JNI_METHOD(jint, getMatrixCoefficientsNative)
    {
        NATIVE_SELF;
        return static_cast<jint>(nativeSelf->mColourInformation.matrixCoefficients);
    }

    JNI_METHOD_ARG(void, setMatrixCoefficientsNative, jint matrixCoefficients)
    {
        NATIVE_SELF;
        nativeSelf->mColourInformation.matrixCoefficients = static_cast<uint16_t>(matrixCoefficients);
    }

    JNI_METHOD(jboolean, getFullRangeFlagNative)
    {
        NATIVE_SELF;
        return static_cast<jboolean>(nativeSelf->mColourInformation.fullRangeFlag);
    }

    JNI_METHOD_ARG(void, setFullRangeFlagNative, jboolean flag)
    {
        NATIVE_SELF;
        nativeSelf->mColourInformation.fullRangeFlag = (flag != 0);
    }

    JNI_METHOD(jobject, getICCProfileNative)
    {
        NATIVE_SELF;
        return env->NewDirectByteBuffer(nativeSelf->mColourInformation.iccProfile.elements,
                                        static_cast<jlong>(nativeSelf->mColourInformation.iccProfile.size));
    }

    JNI_METHOD_ARG(void, setICCProfileNative, jbyteArray data)
    {
        NATIVE_SELF;
        jbyte *nativeData                         = env->GetByteArrayElements(data, 0);
        uint32_t dataSize                         = static_cast<uint32_t>(env->GetArrayLength(data));
        nativeSelf->mColourInformation.iccProfile = HEIF::Array<uint8_t>(dataSize);
        std::memcpy(nativeSelf->mColourInformation.iccProfile.elements, nativeData, dataSize);
        env->ReleaseByteArrayElements(data, nativeData, 0);
    }
}
