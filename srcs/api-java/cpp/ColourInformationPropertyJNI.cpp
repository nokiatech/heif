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

#include <cstring>

#include "DescriptiveProperty.h"
#include "Helpers.h"
#include "heifcommondatatypes.h"
#include "jni_md.h"
#include <stdint.h>

#define CLASS_NAME ColourInformationProperty
extern "C"
{
    JNI_METHOD_ARG(jlong, createContextNative, jobject javaHEIF)
    {
        JNI_TRY
        {
        UNUSED(self);
        NATIVE_HEIF(nativeHeif, javaHEIF);
        auto* nativeObject = new HEIFPP::ColourInformationProperty(nativeHeif);
        return reinterpret_cast<jlong>(nativeObject);
            }
        JNI_CATCH_RETURN(0, "ColourInformationProperty.createContextNative")
    }

    JNI_METHOD(jstring, getColourTypeNative)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        return env->NewStringUTF(nativeSelf->mColourInformation.colourType.value);
            }
        JNI_CATCH_RETURN(nullptr, "ColourInformationProperty.getColourTypeNative")
    }

    JNI_METHOD_ARG(void, setColourTypeNative, jstring javaString)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        const char* nativeString                  = env->GetStringUTFChars(javaString, nullptr);
        nativeSelf->mColourInformation.colourType = HEIF::FourCC(nativeString);
        env->ReleaseStringUTFChars(javaString, nativeString);
            }
        JNI_CATCH_RETURN_VOID("ColourInformationProperty.setColourTypeNative")
    }

    JNI_METHOD(jint, getColourPrimariesNative)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        return static_cast<jint>(nativeSelf->mColourInformation.colourPrimaries);
            }
        JNI_CATCH_RETURN(0, "ColourInformationProperty.getColourPrimariesNative")
    }

    JNI_METHOD_ARG(void, setColourPrimariesNative, jint primaries)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        nativeSelf->mColourInformation.colourPrimaries = static_cast<uint16_t>(primaries);
            }
        JNI_CATCH_RETURN_VOID("ColourInformationProperty.setColourPrimariesNative")
    }

    JNI_METHOD(jint, getTransferCharacteristicsNative)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        return static_cast<jint>(nativeSelf->mColourInformation.transferCharacteristics);
            }
        JNI_CATCH_RETURN(0, "ColourInformationProperty.getTransferCharacteristicsNative")
    }

    JNI_METHOD_ARG(void, setTransferCharacteristicsNative, jint characteristics)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        nativeSelf->mColourInformation.transferCharacteristics = static_cast<uint16_t>(characteristics);
            }
        JNI_CATCH_RETURN_VOID("ColourInformationProperty.setTransferCharacteristicsNative")
    }

    JNI_METHOD(jint, getMatrixCoefficientsNative)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        return static_cast<jint>(nativeSelf->mColourInformation.matrixCoefficients);
            }
        JNI_CATCH_RETURN(0, "ColourInformationProperty.getMatrixCoefficientsNative")
    }

    JNI_METHOD_ARG(void, setMatrixCoefficientsNative, jint matrixCoefficients)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        nativeSelf->mColourInformation.matrixCoefficients = static_cast<uint16_t>(matrixCoefficients);
            }
        JNI_CATCH_RETURN_VOID("ColourInformationProperty.setMatrixCoefficientsNative")
    }

    JNI_METHOD(jboolean, getFullRangeFlagNative)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        return static_cast<jboolean>(nativeSelf->mColourInformation.fullRangeFlag);
            }
        JNI_CATCH_RETURN(JNI_FALSE, "ColourInformationProperty.getFullRangeFlagNative")
    }

    JNI_METHOD_ARG(void, setFullRangeFlagNative, jboolean flag)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        nativeSelf->mColourInformation.fullRangeFlag = (flag != 0);
            }
        JNI_CATCH_RETURN_VOID("ColourInformationProperty.setFullRangeFlagNative")
    }

    JNI_METHOD(jobject, getICCProfileNative)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        return env->NewDirectByteBuffer(nativeSelf->mColourInformation.iccProfile.elements,
                                        static_cast<jlong>(nativeSelf->mColourInformation.iccProfile.size));
            }
        JNI_CATCH_RETURN(nullptr, "ColourInformationProperty.getICCProfileNative")
    }

    JNI_METHOD_ARG(void, setICCProfileNative, jbyteArray data)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        jbyte* nativeData                         = env->GetByteArrayElements(data, nullptr);
        auto dataSize                             = static_cast<uint32_t>(env->GetArrayLength(data));
        nativeSelf->mColourInformation.iccProfile = HEIF::Array<uint8_t>(dataSize);
        std::memcpy(nativeSelf->mColourInformation.iccProfile.elements, nativeData, dataSize);
        env->ReleaseByteArrayElements(data, nativeData, 0);
            }
        JNI_CATCH_RETURN_VOID("ColourInformationProperty.setICCProfileNative")
    }
}
